/** @file sensor.c
 *
 * @brief Tasks for polling sensors.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

#include "main.h"
#include "cmsis_os2.h"
#include "sensor.h"
#include "FreeRTOS.h"

#include "debug.h"

#define MEASURE_POWER_TASK_DELAY 900
#define ANGLE_SENSOR_TASK_DELAY 100
#define INA_CONVERSION_DELAY 100

#define INA_COUNT 3
#define INA_DATA_REGISTER_COUNT 6
#define MAX_I2C_RETRIES 5

#define I2C_MUX_ADDRESS 0x70
#define AS5600_ADDRESS 0x36
#define AS5600_ANGLE_REG 0x0E
#define ANGLE_SENSOR_COUNT 4

/* INA Configuration 
  - All Channels Enabled
  - 16 Sample Average
  - 204 us Voltage Conversion Time
  - 1.1 ms Shunt Conversion Time
  - Shunt and Bus Single-Shot
  Total Conversion Time = 3 * 16 * (1.1 + .204) = 62.592 ms
  */

/* Write this to register 0x00 on every conversion */
#define INA_CONFIGURATION 0x7463 /*0111 0100 0110 0011 */
/* Angle Sensor Channel Mappings */
#define MAST_ANGLE_CHANNEL (1 << 4)
#define RUDDER_ANGLE_CHANNEL (1 << 5)
#define FLAP1_ANGLE_CHANNEL (1 << 6)
#define FLAP2_ANGLE_CHANNEL (1 << 7)

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern osEventFlagsId_t I2C1_EventHandle;
extern osEventFlagsId_t I2C2_EventHandle;
extern osEventFlagsId_t Power_EventHandle;
extern osMutexId_t PowerConversionDataHandle;
extern osMutexId_t AngleDataHandle;

/* INA Addresses */
uint8_t sensor_addresses[] = {INA1, INA2, INA3};
uint8_t register_addresses[] = {CONFIGURATION, REG_CH1_SHUNT, REG_CH1_VOLTAGE, 
                                  REG_CH2_SHUNT, REG_CH2_VOLTAGE, REG_CH3_SHUNT, 
                                  REG_CH3_VOLTAGE, MASK_ENABLE, MANUFACTURER_ID, DIE_ID};
uint8_t angle_sensor_channels[] = {MAST_ANGLE_CHANNEL, RUDDER_ANGLE_CHANNEL, FLAP1_ANGLE_CHANNEL, FLAP2_ANGLE_CHANNEL};

uint8_t raw_conversion_data[INA_COUNT][2*INA_DATA_REGISTER_COUNT];
uint8_t raw_angle_data[ANGLE_SENSOR_COUNT][2];

/**
 * @brief RTOS Task - Read Motor Angles
 * - Write to I2C MUX to select the correct channel
 * - Read the angle from the encoder
 * - Write the angle to the shared memory
 * @param argument: Not used
 * @retval None
 */
void Measure_Angles(void *argument)
{
  static uint8_t angle_config = 0x00;

  while(true)
  {
    for (int i = 0; i < ANGLE_SENSOR_COUNT; i++)
    {
      /* Ensure the I2C bus is not busy before initiating the transfer */
      while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY) 
      {
        osDelay(1);
      }       

      /* Clear any previous event flags before starting a new transaction */
      osEventFlagsClear(I2C2_EventHandle, TX_FLAG | ERR_FLAG);

      angle_config = angle_sensor_channels[i];
      /* Write to I2C MUX to select the correct channel */
      if (HAL_I2C_Master_Transmit_DMA(&hi2c2, (I2C_MUX_ADDRESS << 1), &angle_config, 1) != HAL_OK) 
      {
        DEBUG_PRINT("Error writing to I2C MUX\n");
        osDelay(1);
      }
      /* Block task until I2C handle finishes TX or an error occurs */
      osEventFlagsWait(I2C2_EventHandle, TX_FLAG | ERR_FLAG, osFlagsWaitAny, osWaitForever);

      /* Ensure the I2C bus is not busy before initiating the transfer */
      while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY) 
      {
        osDelay(1);
      }     

      /* Clear any previous event flags before starting a new transaction */
      osEventFlagsClear(I2C2_EventHandle, TX_FLAG | ERR_FLAG);
      osMutexAcquire(AngleDataHandle, osWaitForever);
      /* Read angle from sensor - 2 Bytes starting at register 0x0E*/
      if (HAL_I2C_Mem_Read_DMA(&hi2c2, (AS5600_ADDRESS << 1), AS5600_ANGLE_REG, I2C_MEMADD_SIZE_8BIT, raw_angle_data[i], 2) != HAL_OK) 
      {
        DEBUG_PRINT("Error reading from AS5600\n");
        osDelay(1);
      }

      /* Block task until I2C handle finishes TX or an error occurs */
      osEventFlagsWait(I2C2_EventHandle, RX_FLAG | ERR_FLAG, osFlagsWaitAny, osWaitForever);
      osMutexRelease(AngleDataHandle);
    }
    DEBUG_PRINT("Mast Angle: %d\n", (raw_angle_data[0][0] << 8) | raw_angle_data[0][1]);
    osDelay(ANGLE_SENSOR_TASK_DELAY);
  }

  UNUSED(argument);
}

/**
* @brief RTOS Task - Read Current and Power Measurements for each INA channel.
* @param argument: Not used
* @retval None
*/
void MeasurePower(void *argument)
{
  /* Account for little endian memory storage in ARM */
  static uint16_t ina_config = (uint16_t)((INA_CONFIGURATION >> 8) | (INA_CONFIGURATION << 8));
  
  uint32_t flags = 0U;
  int retry_count = 0;
  while(true)
  {
    for (int i = 0; i < INA_COUNT; i++)
    {
      retry_count = 0;

      /* Trigger Conversion */
      do 
      {
        /* Ensure the I2C bus is not busy before initiating the transfer */
        while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) 
        {
            osDelay(1);
        }       

        /* Clear any previous event flags before starting a new transaction */
        osEventFlagsClear(I2C1_EventHandle, TX_FLAG | ERR_FLAG);

        /* Write Configuraton Register to trigger conversion */
        if (HAL_I2C_Mem_Write_IT(&hi2c1, sensor_addresses[i] << 1, CONFIGURATION, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&ina_config, 2) != HAL_OK) 
        {
            DEBUG_PRINT("Error writing to INA device, retrying\n");
            osDelay(1);
        }

        /* Block task until I2C handle finishes TX or an error occurs */
        flags = osEventFlagsWait(I2C1_EventHandle, TX_FLAG | ERR_FLAG, osFlagsWaitAny, osWaitForever);

        retry_count++;

      } while ((flags & ERR_FLAG) && (retry_count < MAX_I2C_RETRIES));

      if (flags & ERR_FLAG)
      {
        DEBUG_PRINT("Max retries reached for INA device, skipping.\n");
      }
    }
    /* Wait for Conversions to finish */
    osDelay(INA_CONVERSION_DELAY);

    /* Take Power Conversion Data Mutex */
    osMutexAcquire(PowerConversionDataHandle, osWaitForever);
    /* Clear Data Ready Flag */
    osEventFlagsClear(Power_EventHandle, PWR_RDY_FLAG);

    /* Loop Through Each INA*/
    for (int i = 0; i < INA_COUNT; i++)
    {
      /* Loop through each register */
      for (int j = 0; j < INA_DATA_REGISTER_COUNT; j++)
      {
      retry_count = 0;
      
      /* Read Data */
      do 
      {
      
      /* Ensure the I2C bus is not busy before initiating the transfer */
        while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) 
        {
          osDelay(1);
        }       

        /* Clear any previous event flags before starting a new transaction */
        osEventFlagsClear(I2C1_EventHandle, RX_FLAG | ERR_FLAG);

        /* Read Conversion Registers */
        if (HAL_I2C_Mem_Read_IT(&hi2c1, sensor_addresses[i] << 1, register_addresses[j + 1], I2C_MEMADD_SIZE_8BIT, &raw_conversion_data[i][2*j], 2) != HAL_OK) 
        {
          DEBUG_PRINT("Error reading from INA device, retrying\n");
          osDelay(1);
        }

        /* Block task until I2C handle finishes TX or an error occurs */
        flags = osEventFlagsWait(I2C1_EventHandle, RX_FLAG | ERR_FLAG, osFlagsWaitAny, osWaitForever);

        retry_count++;

      } while ((flags & ERR_FLAG) && (retry_count < MAX_I2C_RETRIES));

      if (flags & ERR_FLAG)
      {
        DEBUG_PRINT("Max retries reached for INA device, skipping register %d.\n", register_addresses[j]);
      }
      }
    }
    /* Release Power Conversion Data Mutex */
    osMutexRelease(PowerConversionDataHandle);
    /* Signal Logging Task */
    osEventFlagsSet(Power_EventHandle, PWR_RDY_FLAG);
    osDelay(MEASURE_POWER_TASK_DELAY);
  }

  UNUSED(argument);
}