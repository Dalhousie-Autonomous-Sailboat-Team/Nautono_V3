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

#define FLAP_MOTOR_DUTY_CYCLE 0.9f // 90% duty cycle for flap motor

/**
 * @brief Adjust Rudder Servo Position
 *
 * @param angle
 */
void MoveRudder(int angle)
{
  /* Clamp input to allowed range */
  if (angle < -20)
    angle = -20;
  if (angle > 20)
    angle = 20;

  /* Full servo range is ±90°, corresponding to 1.0ms–2.0ms pulse */
  /* Your range is a subset: -20° to +20° maps to 1.4ms–1.6ms */

  const uint32_t min_pulse = 1200; // 1.4 ms
  const uint32_t max_pulse = 1800; // 1.6 ms

  /* Linearly map angle [-20, +20] → pulse [1400, 1600] us */
  uint32_t pulse = min_pulse + ((angle + 20) * (max_pulse - min_pulse)) / 40;

  /* Apply to PWM (assuming 1 tick = 1 us resolution) */
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pulse);
}

/**
 * @brief Adjust Mast Motor Speed
 *
 * @param speed Speed in percentage (-100 to 100)
 */
void MoveMast(int speed)
{
  if (speed > 20)
    speed = 20;
  if (speed < -20)
    speed = -20;

  uint32_t pwm = (abs(speed) * __HAL_TIM_GET_AUTORELOAD(&htim2)) / 20;

  if (speed > 0)
  {
    /* Forward: CH1 = PWM, CH2 = always HIGH */
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm);
    //__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, __HAL_TIM_GET_AUTORELOAD(&htim2));
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
  }
  else if (speed < 0)
  {
    /* Reverse: CH1 = always HIGH, CH2 = PWM */
    //__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, __HAL_TIM_GET_AUTORELOAD(&htim2));
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwm);
  }
  else
  {
    /* Stop: both low (or use brake mode if needed) */
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);
  }
}

/**
 * @brief Set Flap Motor Direction (full speed)
 *
 * @param direction Positive = forward, Negative = reverse, 0 = stop
 */
void MoveFlap(int direction)
{
  if (direction > 0)
  {
    /* Forward: CH1 = 100%, CH2 = 0 */
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, __HAL_TIM_GET_AUTORELOAD(&htim3) * FLAP_MOTOR_DUTY_CYCLE);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  }
  else if (direction < 0)
  {
    /* Reverse: CH1 = 0, CH2 = 100% */
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, __HAL_TIM_GET_AUTORELOAD(&htim3) * FLAP_MOTOR_DUTY_CYCLE);
  }
  else
  {
    /* Stop: both low */
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
  }
}

/**
 * @brief Set Motor PWM Duty Cycle
 *
 * @param argument
 */
void Control_Motors(void *argument)
{
  /* Start PWM on Timer1 Channel 3 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  /* Start PWM on Timer2 Channel 1 and 2 */
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  /* Start PWM on Timer3 Channel 1 and 2 */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  while (true)
  {
    osEventFlagsWait(Radio_EventHandle, CMD_FLAG, osFlagsWaitAny, osWaitForever);

    radio_control_t cmd = latest_radio_command;

    MoveRudder(cmd.rudder);
    MoveMast(cmd.mast);
    MoveFlap(cmd.flap);
  }

  UNUSED(argument);
}