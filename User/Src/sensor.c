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

#define ANGLE_SENSOR_TASK_DELAY 1000U   /* Task period in RTOS ticks */
#define FAILED_SENSOR_RETRY_SECONDS 30U /* Retry failed sensors every 30 seconds */

#define I2C_MUX_ADDRESS 0x70
#define AS5600_ADDRESS 0x36
#define AS5600_ANGLE_REG 0x0E
#define ANGLE_SENSOR_COUNT 4

/* Angle Sensor Channel Mappings */
#define MAST_ANGLE_CHANNEL (1 << 4)
#define RUDDER_ANGLE_CHANNEL (1 << 5)
#define FLAP1_ANGLE_CHANNEL (1 << 6)
#define FLAP2_ANGLE_CHANNEL (1 << 7)

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern osEventFlagsId_t I2C1_EventHandle;
extern osEventFlagsId_t I2C2_EventHandle;
extern osMutexId_t AngleDataHandle;

uint8_t angle_sensor_channels[] = {MAST_ANGLE_CHANNEL, RUDDER_ANGLE_CHANNEL, FLAP1_ANGLE_CHANNEL, FLAP2_ANGLE_CHANNEL};
uint8_t raw_angle_data[ANGLE_SENSOR_COUNT][2];

/**
 * @brief RTOS Task - Read Angle Measurements from AS5600 sensors via I2C MUX
 * - Every loop, read each angle sensor and flag the relevant control loop task
 * - Skip sensors that fail to respond
 * @param argument: Not used
 * @retval None
 */
void Measure_Angles(void *argument)
{
  osDelay(10000);
  static uint8_t angle_config = 0x00;
  uint32_t flags = 0U;
  bool sensor_failed[ANGLE_SENSOR_COUNT] = {false};
  uint32_t lastRetryTime[ANGLE_SENSOR_COUNT] = {0};

  uint32_t lastWakeTime = osKernelGetTickCount();

  for (;;)
  {
    uint32_t currentTime = osKernelGetTickCount();

    for (int i = 0; i < ANGLE_SENSOR_COUNT; i++)
    {
      /* Skip failed sensors until retry window expires (30s) */
      if (sensor_failed[i] && (currentTime - lastRetryTime[i] < osKernelGetTickFreq() * FAILED_SENSOR_RETRY_SECONDS))
      {
        continue;
      }

      /* -------- Select MUX channel (1 try only) -------- */
      angle_config = angle_sensor_channels[i];
      if (HAL_I2C_Master_Transmit_IT(&hi2c2, (I2C_MUX_ADDRESS << 1),
                                     &angle_config, 1) != HAL_OK)
      {
        sensor_failed[i] = true;
        lastRetryTime[i] = currentTime;
        continue; /* Fail fast */
      }

      flags = osEventFlagsWait(I2C2_EventHandle, TX_FLAG | ERR_FLAG,
                               osFlagsWaitAny, osWaitForever);
      if ((flags & ERR_FLAG) || (flags == osFlagsErrorTimeout))
      {
        sensor_failed[i] = true;
        lastRetryTime[i] = currentTime;
        continue;
      }

      /* -------- Read angle data -------- */
      osMutexAcquire(AngleDataHandle, osWaitForever);

      if (HAL_I2C_Mem_Read_IT(&hi2c2, (AS5600_ADDRESS << 1),
                              AS5600_ANGLE_REG,
                              I2C_MEMADD_SIZE_8BIT,
                              raw_angle_data[i], 2) != HAL_OK)
      {
        sensor_failed[i] = true;
        lastRetryTime[i] = currentTime;
        osMutexRelease(AngleDataHandle);
        continue;
      }

      flags = osEventFlagsWait(I2C2_EventHandle, RX_FLAG | ERR_FLAG,
                               osFlagsWaitAny, osWaitForever);
      if ((flags & ERR_FLAG) || (flags == osFlagsErrorTimeout))
      {
        sensor_failed[i] = true;
        lastRetryTime[i] = currentTime;
      }
      else
      {
        sensor_failed[i] = false;
      }

      osMutexRelease(AngleDataHandle);
    }

    /* Keep loop period stable (ANGLE_SENSOR_TASK_DELAY ticks) */
    osDelay(ANGLE_SENSOR_TASK_DELAY);
  }

  UNUSED(argument);
}
