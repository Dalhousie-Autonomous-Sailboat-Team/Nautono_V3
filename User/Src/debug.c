/** @file debug.c
 *
 * @brief Implement debug functionality.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

#include "debug.h"

#define LED_FLASH_PERIOD 2000
#define LED_FLASH_TIME 50

extern osTimerId_t Debug_Blink_OnHandle;
extern osTimerId_t Debug_Blink_OffHandle;

/* Task Overwrites */

/**
* @brief Function implementing the defaultTask thread.
* @param argument: Not used
* @retval None
*/
void StartDefaultTask(void *argument)
{
    /* Start Debug Timer */
    osTimerStart(Debug_Blink_OnHandle, LED_FLASH_PERIOD);

    /* Infinite loop */
    vTaskSuspend(NULL);
    (void)argument;
}

/* Timer Overwrites */

/**
 * @brief Set the LED on.  Start the off timer.
 * 
 * @param argument 
 */
void Set_LED(void *argument)
{
    HAL_GPIO_WritePin(DEBUG_LED1_GPIO_Port, DEBUG_LED1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIO1_GPIO_Port, GPIO1_Pin, GPIO_PIN_SET);
    osTimerStart(Debug_Blink_OffHandle, LED_FLASH_TIME);
    (void)argument;
}

/**
 * @brief Clear the LED.  This is a one-shot timer.
 * 
 * @param argument 
 */
void Clear_LED(void *argument)
{
    HAL_GPIO_WritePin(DEBUG_LED1_GPIO_Port, DEBUG_LED1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO1_GPIO_Port, GPIO1_Pin, GPIO_PIN_RESET);
    (void)argument;
}