#include "cmsis_os.h"
#include "tim.h"
#include <stdint.h>
#include <stdio.h>

#include "board_io.h"
#include "boat_control.h"
#include "pid.h"
#include "encoder.h"
#include "app_state.h"
#include "control_logic.h"

/* Module Header */
#include "system_tasks.h"


/* System Headers */
#include "main.h"
#include "cmsis_os2.h"


#define SAIL_TASK_PERIOD_MS 50

/* =============== PWM ================*/
#define SERVO_PWM_FREQUENCY_HZ 50U
#define MOTOR_PWM_FREQUENCY_HZ 20000U

extern TIM_HandleTypeDef htim1; //
extern TIM_HandleTypeDef htim2; //
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
osMessageQueueId_t PWM_Queue;

static char buf[128];

extern const BoatMode_t boat_mode;

const BoatMode_t boat_mode = MODE_AUTONOMOUS;       // HARD CODES MODE OF OPERATION FOR NOW!

void BoatControlTask(void *argument)
{
    (void)argument;

    WindSample_t wind = {0};
    RPiSample_t rpi = {0};
    XbeeCommand_t xbee = {0};
    EncoderSample_t enc = {0};

    static uint32_t print_counter = 0;

    while (true)
    {

        uint32_t now = osKernelGetTickCount();

        Wind_GetLatest(&wind);
        RPi_GetLatest(&rpi);
        Xbee_GetLatest(&xbee);
        Encoder_GetLatest(&enc);

        ControlLogicInput_t control_input = {
            .now_ms = now,
            .xbee = xbee,
            .rpi = rpi,
            .encoder = enc,
        };
        ControlLogicOutput_t control_output;
        ControlLogic_Evaluate(&control_input, &control_output);

        /* Throttle debug prints */
        bool do_print = (++print_counter >= 10);
        if (do_print)
            print_counter = 0;

        if (do_print){

            if (control_output.xbee_valid)
            {
                Debug_Print("Valid Xbee found\r\n");
            }
            else
            {
                Debug_Print("Valid Xbee not found\r\n");
            }

            if (control_output.rpi_valid)
            {
                Debug_Print("Valid RPi found\r\n");
            }
            else
            {
                Debug_Print("Valid RPi not found\r\n");
            }

        }

        if(do_print){

            snprintf(buf, sizeof(buf), "wind=%d, rpi_sail=%d, rpi_rudder=%d, xbee_sail=%d, xbee_rudder=%d, enc=%d\r\n",
                    (int)wind.direction, (int)rpi.target_sail_angle, (int)rpi.target_rudder_angle,
                    (int)xbee.sail_angle, (int)xbee.rud_angle, (int)enc.angle);
            Debug_Print(buf);
        }

        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, control_output.sail_motor_channel_1_pwm);
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, control_output.sail_motor_channel_2_pwm);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, control_output.rudder_pwm);

        osDelay(SAIL_TASK_PERIOD_MS);
    }
}

/* ================================== PWM STUFF ================================*/

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
