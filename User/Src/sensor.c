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
#define CONVERSION_DELAY 100

#define NUMBER_OF_INA 3
#define NUMBER_OF_DATA_REGISTERS 6
#define MAX_I2C_RETRIES 5

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

extern I2C_HandleTypeDef hi2c1;
extern osEventFlagsId_t I2C1_EventHandle;

/* INA Addresses */
uint8_t sensor_addresses[] = {INA1, INA2, INA3};
uint8_t register_addresses[] = {CONFIGURATION, REG_CH1_SHUNT, REG_CH1_VOLTAGE, 
                                  REG_CH2_SHUNT, REG_CH2_VOLTAGE, REG_CH3_SHUNT, 
                                  REG_CH3_VOLTAGE, MASK_ENABLE, MANUFACTURER_ID, DIE_ID};

/* Should Change to Thread Safe */
uint8_t power_data[NUMBER_OF_INA][2*NUMBER_OF_DATA_REGISTERS];

/**
* @brief Read Current and Power Measurements for each INA channel.
* @param argument: Not used
* @retval None
*/
__weak void MeasurePower(void *argument)
{
  /* Account for little endian memory storage in ARM */
  static uint16_t ina_config = (uint16_t)((INA_CONFIGURATION >> 8) | (INA_CONFIGURATION << 8));
  
  uint32_t flags = 0U;
  int retry_count = 0;
  while(true)
  {
    for (int i = 0; i < NUMBER_OF_INA; i++)
    {
      retry_count = 0;

      /* Trigger Single Shot Conversion */
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
    osDelay(CONVERSION_DELAY);
    /* Loop Through Each INA*/
    for (int i = 0; i < NUMBER_OF_INA; i++)
    {
      /* Loop through each register */
      for (int j = 0; j < NUMBER_OF_DATA_REGISTERS; j++)
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
        if (HAL_I2C_Mem_Read_IT(&hi2c1, sensor_addresses[i] << 1, register_addresses[j + 1], I2C_MEMADD_SIZE_8BIT, &power_data[i][2*j], 2) != HAL_OK) 
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
    osDelay(MEASURE_POWER_TASK_DELAY);
  }

  UNUSED(argument);
}