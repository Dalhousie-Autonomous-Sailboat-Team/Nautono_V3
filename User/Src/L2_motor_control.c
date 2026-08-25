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

static char buf[128];

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