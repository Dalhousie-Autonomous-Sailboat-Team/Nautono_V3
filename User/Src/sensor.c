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
__weak void MeasurePower(void *argument)
{
  /* Infinite loop */
  osThreadSuspend(osThreadGetId());
  UNUSED(argument);
}