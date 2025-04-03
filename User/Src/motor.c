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
    // HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    // HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
    // /* Set Motor1 Out2 Low */
    // HAL_GPIO_WritePin(MOTOR2_OUT1_GPIO_Port, MOTOR2_OUT1_Pin, GPIO_PIN_RESET);
    // /* Set Motor1 Out2 Low */
    // HAL_GPIO_WritePin(MOTOR2_OUT2_GPIO_Port, MOTOR2_OUT2_Pin, GPIO_PIN_RESET);
    // __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 170000);
    // __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 170000);
    // while(true)
    // {
    //     HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
    //     HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    //     osDelay(1000);
    //     HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
    //     HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    //     osDelay(1000);
    // }
    while(true)
    {
        osDelay(1000);
    }
    UNUSED(argument);
}