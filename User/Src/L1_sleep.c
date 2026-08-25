/** @file sleep.c
 *
 * @brief Pre/Post Sleep processing.
 */

#define LED_SLEEP_INDICATOR /* Uncomment to enable sleep indicator LEDs */

/* Module Header */

/* System Headers */
#include "main.h"
#include "cmsis_os2.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* User Includes */
#include "user_main.h"

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