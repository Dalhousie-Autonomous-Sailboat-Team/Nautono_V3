#include "cmsis_os.h"
#include "tim.h"
#include <stdint.h>
#include <stdio.h>

#include "L1_user_comms.h"
#include "L2_motor_control.h"
#include "L2_pid.h"
#include "L2_encoder.h"
#include "L2_wind.h"
#include "L2_xbee.h"
#include "L2_rpi.h"
#include "L3_boat_mode.h"

/* Module Header */
#include "system.h"


/* System Headers */
#include "main.h"
#include "cmsis_os2.h"


#define SAIL_TASK_PERIOD_MS 50
#define SAIL_DEAD_BAND_DEG 20.0f
#define MOTOR_FULL 12800
#define MOTOR_OFF 0
#define RUDDER_TASK_PERIOD_MS 20
#define XBEE_TIMEOUT_MS 2000 // 2x the send rate
#define RPI_TIMEOUT_MS 10000 // 2x the send rate
#define RUDDER_ANGLE_OFFSET_PWM -130.0f // adjust for mechanical misalignment

// Map XBee sail_angle (-45 to +45) -> AS5600 angle (0 to 360)
// Define your mechanical zero point — adjust SAIL_CENTER_DEG to match
// where the sail sits when sail_angle == 0 on the AS5600 scale
#define SAIL_CENTER_DEG 180.0f // <-- tune this to your physical setup
#define SAIL_RANGE_DEG 45.0f   // max deflection each side

/* =============== PWM ================*/ */
#define SERVO_PWM_FREQUENCY_HZ 50U
#define MOTOR_PWM_FREQUENCY_HZ 20000U

extern TIM_HandleTypeDef htim1; //
extern TIM_HandleTypeDef htim2; //
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
osMessageQueueId_t PWM_Queue;

static char buf[128];

const BoatMode_t boat_mode = MODE_AUTONOMOUS;       // HARD CODES MODE OF OPERATION FOR NOW!

/* ================================== MOTOR CONTROL STUFF ================================*/ */

static float wrap_error(float error)
{
    while (error > 180.0f)
        error -= 360.0f;
    while (error < -180.0f)
        error += 360.0f;
    return error;
}

static float sail_command_to_encoder_deg(float sail_angle)
{
    // Linear map: -45 -> (CENTER - 45), 0 -> CENTER, +45 -> (CENTER + 45)
    float deg = SAIL_CENTER_DEG + sail_angle;

    // Wrap into [0, 360)
    while (deg >= 360.0f)
        deg -= 360.0f;
    while (deg < 0.0f)
        deg += 360.0f;

    return deg;
}

void SailMotorTask(void *argument)
{
    (void)argument;

    WindSample_t wind = {0};
    RPiSample_t rpi = {0};
    XbeeCommand_t xbee = {0};
    EncoderSample_t enc = {0};

    float target_sail_angle = 0.0f;
    float target_rudder_angle = 0.0f;

    static uint32_t print_counter = 0;

    while (true)
    {

        uint32_t now = osKernelGetTickCount();

        Wind_GetLatest(&wind);
        RPi_GetLatest(&rpi);
        Xbee_GetLatest(&xbee);
        Encoder_GetLatest(&enc);

        bool xbee_valid = xbee.ever_received &&
                          (now - xbee.last_updated_ms) < XBEE_TIMEOUT_MS;
        bool rpi_valid = rpi.ever_received &&
                         (now - rpi.last_updated_ms) < RPI_TIMEOUT_MS;

        /* Throttle debug prints */
        bool do_print = (++print_counter >= 10);
        if (do_print)
            print_counter = 0;

        if (do_print){

            if (xbee_valid)
            {
                Debug_Print_String("Valid Xbee found\r\n");
            }
            else
            {
                Debug_Print_String("Valid Xbee not found\r\n");
            }

            if (rpi_valid)
            {
                Debug_Print_String("Valid RPi found\r\n");
            }
            else
            {
                Debug_Print_String("Valid RPi not found\r\n");
            }

        }

        // Sail angle fallback hierarchy
        if (xbee_valid && rpi_valid)
        {
            target_sail_angle = (xbee.sail_angle != 0.0f) ? xbee.sail_angle : rpi.target_sail_angle;
        }
        else if (xbee_valid)
        {
            target_sail_angle = xbee.sail_angle;
        }
        else if (rpi_valid)
        {
            target_sail_angle = rpi.target_sail_angle;
        }
        else
        {
            target_sail_angle = 0.0f;
        }

        // Rudder angle fallback hierarchy
        if (xbee_valid && rpi_valid)
        {
            target_rudder_angle = (xbee.rud_angle != 0.0f) ? xbee.rud_angle : rpi.target_rudder_angle;
        }
        else if (xbee_valid)
        {
            target_rudder_angle = xbee.rud_angle;
        }
        else if (rpi_valid)
        {
            target_rudder_angle = rpi.target_rudder_angle;
        }
        else
        {
            target_rudder_angle = 0.0f;
        }

        if(do_print){

            snprintf(buf, sizeof(buf), "wind=%d, rpi_sail=%d, rpi_rudder=%d, xbee_sail=%d, xbee_rudder=%d, enc=%d\r\n",
                    (int)wind.direction, (int)rpi.target_sail_angle, (int)rpi.target_rudder_angle,
                    (int)xbee.sail_angle, (int)xbee.rud_angle, (int)enc.angle);
            Debug_Print_String(buf);
        }

        // Convert command degrees (-45..+45) to encoder space (0..360)
        float target_enc_deg = sail_command_to_encoder_deg(target_sail_angle);

        // Bang Bang control for sail
        float error = wrap_error(target_enc_deg - enc.angle);

        if (error > SAIL_DEAD_BAND_DEG)
        {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, MOTOR_FULL);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, MOTOR_OFF);
        }
        else if (error < -SAIL_DEAD_BAND_DEG)
        {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, MOTOR_OFF);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, MOTOR_FULL);
        }
        else
        {
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, MOTOR_OFF);
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, MOTOR_OFF);
        }

        // Direct control for rudder
        float rudder_center = 1500.0f + RUDDER_ANGLE_OFFSET_PWM;

        uint16_t pulse = (uint16_t)(rudder_center + (target_rudder_angle / 45.0f) * 400.0f);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, pulse);

        osDelay(SAIL_TASK_PERIOD_MS);
    }
}

/* ================================== PWM STUFF ================================*/ */

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