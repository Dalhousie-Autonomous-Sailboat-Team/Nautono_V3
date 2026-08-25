/** @file system.c
 *
 * @brief System initialization functions.
 */

/* Module Headers */
#include "system.h"

/* System Headers */
#include "main.h"
#include "cmsis_os2.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* User Headers */
#include "actuators.h"

#define HEARTBEAT_PERIOD_MS 2000 /* Total period for one heartbeat cycle in milliseconds */
#define HEARTBEAT_ON_TIME_MS 1   /* Duration for which the LED stays ON in milliseconds */

void InitTask(void *argument)
{
    /* Initialize PWM module */
    PWM_Init();

    /* Delete Init Task */
    osThreadExit();
}

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

/**
 * @brief Pre Sleep Processing
 * - This function is called before the system enters sleep mode.
 *
 * @param ulExpectedIdleTime
 */
void PreSleepProcessing(uint32_t ulExpectedIdleTime)
{
    /* Disable Systick */
    HAL_SuspendTick();
#ifdef LED_SLEEP_INDICATOR
    /* Set GPIO Pin to indicate sleep*/
    HAL_GPIO_WritePin(DEBUG_LED2_GPIO_Port, DEBUG_LED2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIO4_GPIO_Port, GPIO4_Pin, GPIO_PIN_SET);
#endif /* LED_SLEEP_INDICATOR */

    /* Enter Sleep */
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

    (void)ulExpectedIdleTime;
}

/* ======================== SLEEP STUFF ===========================*/ */

/**
 * @brief Post Sleep Processing
 * - This function is called after the system exits sleep mode.
 *
 * @param ulExpectedIdleTime
 */
void PostSleepProcessing(uint32_t ulExpectedIdleTime)
{
    /* Re-enable Systick */
    HAL_ResumeTick();
#ifdef LED_SLEEP_INDICATOR
    /* Clear GPIO Pin to indicate End of Sleep */
    HAL_GPIO_WritePin(DEBUG_LED2_GPIO_Port, DEBUG_LED2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO4_GPIO_Port, GPIO4_Pin, GPIO_PIN_RESET);
#endif /* LED_SLEEP_INDICATOR */

    (void)ulExpectedIdleTime;
}