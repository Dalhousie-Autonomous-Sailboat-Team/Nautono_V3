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

/**
* @brief Read Current and Power Measurements for each INA channel.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MeasurePower */
__weak void MeasurePower(void *argument)
{
  /* USER CODE BEGIN Measure_Power */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Measure_Power */
}