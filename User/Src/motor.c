/** @file motor.c
 *
 * @brief Tasks and functions for controlling motor position.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"

#include "motor.h"
#include "debug.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

/**
 * @brief Set Motor PWM Duty Cycle
 * 
 * @param argument 
 */
void Control_Motors(void *argument)
{
    uint32_t compareValue = 1000;
    int step = 100;

    // Start PWM on Timer 1 Channel 3
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3) != HAL_OK)
    {
        DEBUG_PRINT("Failed to start PWM on Timer 1 Channel 3");
        return;
    }

    while (true)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, compareValue);
        compareValue += step;

        if (compareValue >= 2000 || compareValue <= 1000)
        {
            step = -step; // Reverse direction
        }

        osDelay(100);
    }
    while(true)
    {
        osDelay(1000);
    }
    UNUSED(argument);
}