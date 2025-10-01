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

#define MEASURE_POWER_TASK_DELAY 1000
#define ANGLE_SENSOR_TASK_DELAY 1
#define INA_CONVERSION_DELAY 100

#define INA_COUNT 3
#define INA_DATA_REGISTER_COUNT 6
#define MAX_I2C_RETRIES 5

#define I2C_MUX_ADDRESS 0x70
#define AS5600_ADDRESS 0x36
#define AS5600_ANGLE_REG 0x0E

/* Uncomment to disable debug prints in this file */
// #define DEBUG_PRINT(...)

#define ENABLED_ANGLE_SENSOR_CHANNELS (MAST_ANGLE_CHANNEL) //| FLAP1_ANGLE_CHANNEL) // | RUDDER_ANGLE_CHANNEL | FLAP1_ANGLE_CHANNEL | FLAP2_ANGLE_CHANNEL)

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
extern osEventFlagsId_t Motor_Control_EventHandle;
extern osMutexId_t PowerConversionDataHandle;
extern osMutexId_t AngleDataHandle;

/* INA Addresses */
uint8_t sensor_addresses[] = {INA1, INA2, INA3};
uint8_t register_addresses[] = {CONFIGURATION, REG_CH1_SHUNT, REG_CH1_VOLTAGE,
                                REG_CH2_SHUNT, REG_CH2_VOLTAGE, REG_CH3_SHUNT,
                                REG_CH3_VOLTAGE, MASK_ENABLE, MANUFACTURER_ID, DIE_ID};
uint8_t angle_sensor_channels[] = {MAST_ANGLE_CHANNEL, RUDDER_ANGLE_CHANNEL, FLAP1_ANGLE_CHANNEL, FLAP2_ANGLE_CHANNEL};

uint8_t raw_conversion_data[INA_COUNT][2 * INA_DATA_REGISTER_COUNT];
uint8_t raw_angle_data[ANGLE_SENSOR_COUNT][2];

/**
 * @brief RTOS Task - Read Angle Measurements from AS5600 sensors via I2C MUX
 * - Read angle for Mast (ch0)
 * - Select sensor channel via MUX
 * - Read the angle register from each AS5600 device
 * - Store the raw data and timestamp in shared memory
 * - Run at 1 kHz
 * @param argument: Not used
 * @retval None
 */
void Measure_Angles(void *argument)
{
  static uint8_t angle_config = 0x00;
  uint32_t flags = 0U;

  while (true)
  {
    uint32_t currentTime = osKernelGetTickCount();

    /* Loop through each angle sensor */
    for (int i = 0; i < ANGLE_SENSOR_COUNT; i++)
    {
      /* Skip disabled channels */
      if (!(1 << (i + 4) & ENABLED_ANGLE_SENSOR_CHANNELS))
      {
        continue;
      }

      /* Select sensor via MUX */
      /* Wait for I2C bus to be ready before starting the transaction */
      while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY)
      {
        osThreadYield(); /* Yield to other equal priority tasks while waiting */
      }

      /* Clear previous event flags */
      osEventFlagsClear(I2C2_EventHandle, TX_FLAG | ERR_FLAG);

      /* Write to MUX to select sensor channel */
      angle_config = angle_sensor_channels[i];
      if (HAL_I2C_Master_Transmit_IT(&hi2c2, (I2C_MUX_ADDRESS << 1), &angle_config, 1) != HAL_OK)
      {
        DEBUG_PRINT("Error writing to I2C MUX channel %d\n", i);
      }

      /* Wait for TX complete or error */
      flags = osEventFlagsWait(I2C2_EventHandle, TX_FLAG | ERR_FLAG, osFlagsWaitAny, osWaitForever);

      /* If unsuccessful, skip this sensor and try again next cycle */
      if (flags & ERR_FLAG)
      {
        DEBUG_PRINT("Error received from I2C MUX channel %d\n", i);
        continue;
      }

      /* Acquire Mutex to write to shared memory */
      osMutexAcquire(AngleDataHandle, osWaitForever);

      /* Attempt to read angle data from sensor */
      while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY)
      {
        osThreadYield(); /* Yield to other equal priority tasks while waiting */
      }

      osEventFlagsClear(I2C2_EventHandle, RX_FLAG | ERR_FLAG);

      /* Read 2 bytes from angle register */
      if (HAL_I2C_Mem_Read_IT(&hi2c2, (AS5600_ADDRESS << 1), AS5600_ANGLE_REG, I2C_MEMADD_SIZE_8BIT, raw_angle_data[i], 2) != HAL_OK)
      {
        DEBUG_PRINT("Error initiating read from AS5600 channel %d\n", i);
      }

      /* Wait for RX complete or error */
      flags = osEventFlagsWait(I2C2_EventHandle, RX_FLAG | ERR_FLAG, osFlagsWaitAny, osWaitForever);

      /* If unsuccessful, skip this sensor and try again next cycle */
      if ((flags & ERR_FLAG))
      {
        DEBUG_PRINT("Error received from AS5600 channel %d.\n", i);
      }
      else
      {
        /* Set the appropriate ready flag */
        switch (i)
        {
        case 0:
          osEventFlagsSet(Motor_Control_EventHandle, MAST_ANGLE_READY_FLAG);
          break;
        case 1:
          osEventFlagsSet(Motor_Control_EventHandle, RUDDER_ANGLE_READY_FLAG);
          break;
        case 2:
          osEventFlagsSet(Motor_Control_EventHandle, FLAP1_ANGLE_READY_FLAG);
          break;
        case 3:
          osEventFlagsSet(Motor_Control_EventHandle, FLAP2_ANGLE_READY_FLAG);
          break;
        default:
          break;
        }
      }
      osMutexRelease(AngleDataHandle);
      // DEBUG_PRINT("Angle Sensor %d Raw Data: 0x%02X%02X\n", i, raw_angle_data[i][0], raw_angle_data[i][1]);
    }

    /* Wait before the next measurement cycle */
    osDelayUntil(currentTime + ANGLE_SENSOR_TASK_DELAY);
  }

  UNUSED(argument); /* Prevent unused argument compiler warning */
}

/**
 * @brief RTOS Task - Read Current and Power Measurements for each INA channel.
 * - Write to each INA device to configure it for the next measurement
 * - Retry failed sensors 5 times, then defer retry for 30 seconds
 * - Read the current and voltage data from each INA device
 * - Store the data in shared memory
 * - Notify the main thread when the data is ready
 * - Run once per second (100 ms conversion delay + 900 ms task delay)
 * @param argument: Not used
 * @retval None
 */
void MeasurePower(void *argument)
{
  static uint16_t ina_config = (uint16_t)((INA_CONFIGURATION >> 8) | (INA_CONFIGURATION << 8));

  uint32_t flags = 0U;
  int retry_count = 0;
  bool sensor_failed[INA_COUNT] = {false}; /* Track failed sensors */
  uint32_t lastRetryTime = osKernelGetTickCount();

  /* Main loop for measuring power */
  while (true)
  {
    uint32_t currentTime = osKernelGetTickCount();

    /* Loop through each device */
    for (int i = 0; i < INA_COUNT; i++)
    {
      /* If sensor previously failed, only retry every 30 seconds */
      if (sensor_failed[i] && (currentTime - lastRetryTime < pdMS_TO_TICKS(30000)))
      {
        continue;
      }

      retry_count = 0;
      /* Attempt to write configuration to INA Device to start conversion */
      /* Retry up to MAX_I2C_RETRIES times if the write fails */
      do
      {
        /* Wait for I2C bus to be ready before starting the transaction */
        while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
        {
          osDelay(1); /* Delay 1 ms as timing is not critical */
        }

        /* Clear any previous event flags before starting a new transaction */
        osEventFlagsClear(I2C1_EventHandle, TX_FLAG | ERR_FLAG);

        /* Write Configuration to INA device */
        if (HAL_I2C_Mem_Write_IT(&hi2c1, sensor_addresses[i] << 1, CONFIGURATION, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&ina_config, 2) != HAL_OK)
        {
          DEBUG_PRINT("Error writing to INA device %d, retrying\n", i);
          osDelay(1);
        }

        /* Block task until I2C handle finishes TX or an error occurs */
        flags = osEventFlagsWait(I2C1_EventHandle, TX_FLAG | ERR_FLAG, osFlagsWaitAny, osWaitForever);
        retry_count++;

        /* Continue retrying if an error occurred and the retry count is less than MAX_I2C_RETRIES */
      } while ((flags & ERR_FLAG) && (retry_count < MAX_I2C_RETRIES));

      /* If the maximum number of retries is reached, mark the sensor as failed */
      if (flags & ERR_FLAG)
      {
        DEBUG_PRINT("Max retries reached for INA device %d, deferring retry for 30s.\n", i);
        sensor_failed[i] = true; /* Mark as failed, retry later */
        lastRetryTime = currentTime;
        continue;
      }

      /* If write was successful, clear the failed flag */
      sensor_failed[i] = false; /* Sensor recovered */
    }

    /* Wait for INA_CoNVERSION_DELAY ms to allow ADC conversion to complete */
    osDelay(INA_CONVERSION_DELAY);

    /* Acquire Mutex to allow for writing to shared memory */
    osMutexAcquire(PowerConversionDataHandle, osWaitForever);
    osEventFlagsClear(Power_EventHandle, PWR_RDY_FLAG); /* Clear Power Ready Flag */

    /* Loop through each device again to read the data */
    for (int i = 0; i < INA_COUNT; i++)
    {
      /* Skip Failed Sensors */
      if (sensor_failed[i])
      {
        continue;
      }

      /* Loop through each conversion register */
      for (int j = 0; j < INA_DATA_REGISTER_COUNT; j++)
      {
        retry_count = 0;

        /* Try to read the data from the INA device */
        /* Retry up to MAX_I2C_RETRIES times if the read fails */
        do
        {
          /* Wait for I2C bus to be ready before starting the transaction */
          while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
          {
            osDelay(1);
          }

          /* Clear any previous event flags before starting a new transaction */
          osEventFlagsClear(I2C1_EventHandle, RX_FLAG | ERR_FLAG);

          /* Read the data from the INA device */
          if (HAL_I2C_Mem_Read_IT(&hi2c1, sensor_addresses[i] << 1, register_addresses[j + 1], I2C_MEMADD_SIZE_8BIT, &raw_conversion_data[i][2 * j], 2) != HAL_OK)
          {
            DEBUG_PRINT("Error reading from INA device %d register %d, retrying\n", i, register_addresses[j]);
            osDelay(1);
          }

          /* Block task until I2C handle finishes RX or an error occurs */
          flags = osEventFlagsWait(I2C1_EventHandle, RX_FLAG | ERR_FLAG, osFlagsWaitAny, osWaitForever);
          retry_count++;

          /* Continue retrying if an error occurred and the retry count is less than MAX_I2C_RETRIES */
        } while ((flags & ERR_FLAG) && (retry_count < MAX_I2C_RETRIES));

        /* If the maximum number of retries is reached, mark the sensor as failed */
        if (flags & ERR_FLAG)
        {
          DEBUG_PRINT("Max retries reached for INA device %d, skipping register %d.\n", i, register_addresses[j]);
        }
      }
    }

    /* Release mutex to allow reading from shared memory */
    osMutexRelease(PowerConversionDataHandle);
    /* Notify the main thread that the data is ready */
    osEventFlagsSet(Power_EventHandle, PWR_RDY_FLAG);
    /* Wait for the next measurement cycle */
    osDelay(MEASURE_POWER_TASK_DELAY);
  }

  UNUSED(argument); /* Prevent unused argument compiler warning */
}