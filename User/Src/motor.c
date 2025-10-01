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
#include "task.h"

#include "motor.h"
#include "debug.h"
#include "radio_control.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern osEventFlagsId_t Radio_EventHandle;
extern radio_control_t latest_radio_command;

#define PWM_FREQUENCY 20000 /* 20 kHz */

float mast_speed = 0.0f;

void Set_Mast_Speed(float speed)
{
  mast_speed = speed;
}

void Change_Mast_Speed(float speed)
{
  int direction;
  /* Clamp speed to -100 to 100 */
  if (speed > 100)
  {
    speed = 100;
  }
  else if (speed < -100)
  {
    speed = -100;
  }

  if (speed >= 0)
  {
    direction = 1; /* Forward */
  }
  else
  {
    direction = -1; /* Reverse */
    speed = -speed; /* Make speed positive for PWM calculation */
  }

  /* Apply dead zone: set speed to 0 if within -1 to 1 */
  if (speed < 0.1 && speed > -0.1)
  {
    direction = 0;
    speed = 0;
  }

  /* Calculate the PWM duty cycle */
  uint32_t pulse = (htim1.Init.Period + 1) * speed / 100 - 1;

  if (direction == 1)
  {
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, htim2.Init.Period); /* CH1 high (full on) */
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);             /* CH2 PWM */
  }
  else if (direction == -1)
  {
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);             /* CH1 PWM */
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, htim2.Init.Period); /* CH2 high (full on) */
  }
  else if (direction == 0)
  {
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, htim2.Init.Period); /* CH1 high (full on) */
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, htim2.Init.Period); /* CH2 high (full on) */
  }
}

/**
 * @brief Start Motor PWM Duty Cycle
 *
 * @param argument
 */
void Control_Motors(void *argument)
{
  /* Start PWM on Timer2 Channel 1 and 2 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

  while (1)
  {
    Change_Mast_Speed(mast_speed);
    osDelay(10); /* Update every 50 ms */
  }

  UNUSED(argument);
}