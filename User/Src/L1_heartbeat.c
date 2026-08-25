/**
 * @file heartbeat.c
 *
 * @brief Heartbeat LED control.
 */

/* Module Header */

/* System Includes */
#include "main.h"
#include "cmsis_os2.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* User Includes */
#include "user_main.h"

#define HEARTBEAT_PERIOD_MS 2000 /* Total period for one heartbeat cycle in milliseconds */
#define HEARTBEAT_ON_TIME_MS 1   /* Duration for which the LED stays ON in milliseconds */

void HeartbeatTask(void *argument)
{
    /* Infinite loop */
    while (true)
    {
        /* Toggle Heartbeat LED */
        HAL_GPIO_WritePin(DEBUG_LED1_GPIO_Port, DEBUG_LED1_Pin, GPIO_PIN_SET);

        /* Delay for 100 milliseconds */
        osDelay(HEARTBEAT_ON_TIME_MS);
        HAL_GPIO_WritePin(DEBUG_LED1_GPIO_Port, DEBUG_LED1_Pin, GPIO_PIN_RESET);
        osDelay(HEARTBEAT_PERIOD_MS - HEARTBEAT_ON_TIME_MS);
    }
}