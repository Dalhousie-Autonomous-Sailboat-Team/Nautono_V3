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

/**
 * @brief Set Motor PWM Duty Cycle
 * 
 * @param argument 
 */
void Control_Motors(void *argument)
{
    /* Set Motor1 Out2 Low */
    HAL_GPIO_WritePin(MOTOR1_OUT2_GPIO_Port, MOTOR1_OUT2_Pin, GPIO_PIN_RESET);
    /* Set initial PWM */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    while(true)
    {
        /* Loop through duty cycles up and down */
        for (int i = 0; i < 100; i++)
        {
            /* Set PWM Duty Cycle */
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 34*i);
            osDelay(10);
        }
        for (int i = 100; i > 0; i--)
        {
            /* Set PWM Duty Cycle */
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 34*i);
            osDelay(10);
        }
    }
    UNUSED(argument);
}