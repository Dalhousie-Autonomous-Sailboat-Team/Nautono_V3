/** @file sleep.c
 *
 * @brief Pre/Post Sleep processing.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

#include "main.h"
#include "cmsis_os2.h"

#include "sleep.h"

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
    /* Set GPIO Pin to indicate sleep*/
    HAL_GPIO_WritePin(DEBUG_LED2_GPIO_Port, DEBUG_LED2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIO4_GPIO_Port, GPIO4_Pin, GPIO_PIN_SET);

    /* Enter Sleep */
    __WFI();

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
    /* Clear GPIO Pin to indicate End of Sleep */
    HAL_GPIO_WritePin(DEBUG_LED2_GPIO_Port, DEBUG_LED2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO4_GPIO_Port, GPIO4_Pin, GPIO_PIN_RESET);

    (void)ulExpectedIdleTime;
}