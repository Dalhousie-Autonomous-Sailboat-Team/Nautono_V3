/**
 * @file    pwm.c
 *
 * @brief PWM timer for servo and motor control processing.
 */

/* Module Header */
#include "L1_pwm.h"

/* System Headers */
#include "main.h"
#include "cmsis_os2.h"

/* User Includes */
#include "user_main.h"

#define SERVO_PWM_FREQUENCY_HZ 50U
#define MOTOR_PWM_FREQUENCY_HZ 20000U

extern TIM_HandleTypeDef htim1; //
extern TIM_HandleTypeDef htim2; //
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
osMessageQueueId_t PWM_Queue;

/**
 * @brief Initialize PWM module
 */
void PWM_Init(void)
{
    /* Timers are clocked @ 64 MHz */

    /* Motor PWM @ 20 kHz
     *
     * Maximum Counter period is 65535
     * No Prescaler (Prescaler = 0) => Counter clock = Timer clock = 64 MHz
     * Counter clock = 64 MHz
     * PWM Frequency = Counter clock / (Period + 1) = 64,000,000 / 3200 = 20,000 Hz
     *
     * Configuration set in CubeMX
     */

    /* Servo PWM @ 50 Hz
     *
     * Maximum Counter period is 65535
     * Prescaler = (Timer clock / Desired counter clock) - 1
     * Prescaler = (64,000,000 / 1,000,000) - 1 = 63
     * Counter clock = 1 MHz
     * PWM Frequency = Counter clock / (Period + 1) = 1,000,000 / 20000 = 50 Hz
     *
     * Configuration set in CubeMX
     */

    /* Initialize servo PWM duty cycle to 1500 */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 6400);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

    /* Initialize Motor PWM duty cycles to 0 */

    // J9 CONNECTOR ON PCB
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0);

    // J10 CONNECTOR - ON PCB
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);

    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, 0);

    /* Start Motor PWM generation on tim 2 */
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
// #ifdef USE_FLAP_MOTOR
//     HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
//     HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
// #endif
// #ifdef USE_MOTOR_CHANNEL_4
//     HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_1);
//     HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);
// #endif
}

/**
 * @brief Set PWM duty cycle for a given channel
 * @param channel PWM channel to set
 * @param duty_cycle Duty cycle value
 */
void PWM_SetDutyCycle(PWM_Channel_t channel, uint16_t duty_cycle)
{
    switch (channel)
    {
    case PWM_CHANNEL_RUDDER_SERVO:
        /* Servo PWM duty cycle range: 1000 to 2000 */
        if (duty_cycle < SERVO_DUTY_CYCLE_MIN)
            duty_cycle = SERVO_DUTY_CYCLE_MIN;
        if (duty_cycle > SERVO_DUTY_CYCLE_MAX)
            duty_cycle = SERVO_DUTY_CYCLE_MAX;
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, duty_cycle);
        break;
    case PWM_CHANNEL_MAST_1_MOTOR:
        /* Motor PWM duty cycle range: 0 to 3200 */
        if (duty_cycle < MOTOR_DUTY_CYCLE_MIN)
            duty_cycle = MOTOR_DUTY_CYCLE_MIN;
        if (duty_cycle > MOTOR_DUTY_CYCLE_MAX)
            duty_cycle = MOTOR_DUTY_CYCLE_MAX;
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty_cycle);
        break;
    case PWM_CHANNEL_MAST_2_MOTOR:
        /* Motor PWM duty cycle range: 0 to 3200 */
        if (duty_cycle < MOTOR_DUTY_CYCLE_MIN)
            duty_cycle = MOTOR_DUTY_CYCLE_MIN;
        if (duty_cycle > MOTOR_DUTY_CYCLE_MAX)
            duty_cycle = MOTOR_DUTY_CYCLE_MAX;
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, duty_cycle);
        break;
#ifdef USE_FLAP_MOTOR
    case PWM_CHANNEL_FLAP_1_MOTOR:
        /* Motor PWM duty cycle range: 0 to 3200 */
        if (duty_cycle < MOTOR_DUTY_CYCLE_MIN)
            duty_cycle = MOTOR_DUTY_CYCLE_MIN;
        if (duty_cycle > MOTOR_DUTY_CYCLE_MAX)
            duty_cycle = MOTOR_DUTY_CYCLE_MAX;
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty_cycle);
        break;
    case PWM_CHANNEL_FLAP_2_MOTOR:
        /* Motor PWM duty cycle range: 0 to 3200 */
        if (duty_cycle < MOTOR_DUTY_CYCLE_MIN)
            duty_cycle = MOTOR_DUTY_CYCLE_MIN;
        if (duty_cycle > MOTOR_DUTY_CYCLE_MAX)
            duty_cycle = MOTOR_DUTY_CYCLE_MAX;
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, duty_cycle);
        break;
#endif
#ifdef USE_MOTOR_CHANNEL_4
    case PWM_CHANNEL_FLAP2_1_MOTOR:
        /* Motor PWM duty cycle range: 0 to 3200 */
        if (duty_cycle < MOTOR_DUTY_CYCLE_MIN)
            duty_cycle = MOTOR_DUTY_CYCLE_MIN;
        if (duty_cycle > MOTOR_DUTY_CYCLE_MAX)
            duty_cycle = MOTOR_DUTY_CYCLE_MAX;
        __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_1, duty_cycle);
        break;
    case PWM_CHANNEL_FLAP2_2_MOTOR:
        /* Motor PWM duty cycle range: 0 to 3200 */
        if (duty_cycle < MOTOR_DUTY_CYCLE_MIN)
            duty_cycle = MOTOR_DUTY_CYCLE_MIN;
        if (duty_cycle > MOTOR_DUTY_CYCLE_MAX)
            duty_cycle = MOTOR_DUTY_CYCLE_MAX;
        __HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, duty_cycle);
        break;
#endif
    default:
        /* Invalid channel */
        break;
    }
}