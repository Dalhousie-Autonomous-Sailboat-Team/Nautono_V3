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
#define MEASURE_POWER_RETRY_DELAY 100

#define NUMBER_OF_INA 3
#define NUMBER_OF_DATA_REGISTERS 6
#define MAX_I2C_RETRIES 5

#define INA_CONFIGURATION 0x4127

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
  static uint16_t ina_config = INA_CONFIGURATION;
  for (int i = 0; i < NUMBER_OF_INA; i++)
  {
    uint32_t flags;
    int retry_count = 0;

    /* Set Configuration Register */
    do 
    {
        /* Ensure the I2C bus is not busy before initiating the transfer */
        while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY) 
        {
            osDelay(1);
        }       

        /* Clear any previous event flags before starting a new transaction */
        osEventFlagsClear(I2C1_EventHandle, TX_FLAG | ERR_FLAG);

        if (HAL_I2C_Mem_Write_IT(&hi2c1, sensor_addresses[i] << 1, CONFIGURATION, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&ina_config, 2) != HAL_OK) 
        {
            DEBUG_PRINT("Error writing to INA device %d, retry %d\n", i, retry_count);
            osDelay(MEASURE_POWER_RETRY_DELAY);
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

  while(true)
  {
    osDelay(1);
  }

  UNUSED(argument);
}