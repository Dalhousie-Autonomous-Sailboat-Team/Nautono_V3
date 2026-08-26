#include "control_logic.h"

#include <stddef.h>

#define SAIL_DEAD_BAND_DEG 20.0f
#define MOTOR_FULL 12800U
#define MOTOR_OFF 0U
#define XBEE_TIMEOUT_MS 2000U
#define RPI_TIMEOUT_MS 10000U
#define RUDDER_ANGLE_OFFSET_PWM -130.0f

/* Map a sail command in degrees to the AS5600 encoder scale. */
#define SAIL_CENTER_DEG 180.0f

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
    float deg = SAIL_CENTER_DEG + sail_angle;

    while (deg >= 360.0f)
        deg -= 360.0f;
    while (deg < 0.0f)
        deg += 360.0f;

    return deg;
}

void ControlLogic_Evaluate(const ControlLogicInput_t *input, ControlLogicOutput_t *output)
{
    if (output == NULL)
        return;

    ControlLogicOutput_t result = {0};

    if (input == NULL)
    {
        *output = result;
        return;
    }

    result.xbee_valid = input->xbee.ever_received &&
                        (input->now_ms - input->xbee.last_updated_ms) < XBEE_TIMEOUT_MS;
    result.rpi_valid = input->rpi.ever_received &&
                       (input->now_ms - input->rpi.last_updated_ms) < RPI_TIMEOUT_MS;

    if (result.xbee_valid && result.rpi_valid)
    {
        result.target_sail_angle =
            (input->xbee.sail_angle != 0.0f) ? input->xbee.sail_angle : input->rpi.target_sail_angle;
    }
    else if (result.xbee_valid)
    {
        result.target_sail_angle = input->xbee.sail_angle;
    }
    else if (result.rpi_valid)
    {
        result.target_sail_angle = input->rpi.target_sail_angle;
    }
    else
    {
        result.target_sail_angle = 0.0f;
    }

    if (result.xbee_valid && result.rpi_valid)
    {
        result.target_rudder_angle =
            (input->xbee.rud_angle != 0.0f) ? input->xbee.rud_angle : input->rpi.target_rudder_angle;
    }
    else if (result.xbee_valid)
    {
        result.target_rudder_angle = input->xbee.rud_angle;
    }
    else if (result.rpi_valid)
    {
        result.target_rudder_angle = input->rpi.target_rudder_angle;
    }
    else
    {
        result.target_rudder_angle = 0.0f;
    }

    float target_encoder_angle = sail_command_to_encoder_deg(result.target_sail_angle);
    float sail_error = wrap_error(target_encoder_angle - input->encoder.angle);

    if (sail_error > SAIL_DEAD_BAND_DEG)
    {
        result.sail_motor_channel_1_pwm = MOTOR_FULL;
        result.sail_motor_channel_2_pwm = MOTOR_OFF;
    }
    else if (sail_error < -SAIL_DEAD_BAND_DEG)
    {
        result.sail_motor_channel_1_pwm = MOTOR_OFF;
        result.sail_motor_channel_2_pwm = MOTOR_FULL;
    }
    else
    {
        result.sail_motor_channel_1_pwm = MOTOR_OFF;
        result.sail_motor_channel_2_pwm = MOTOR_OFF;
    }

    float rudder_center = 1500.0f + RUDDER_ANGLE_OFFSET_PWM;
    result.rudder_pwm =
        (uint16_t)(rudder_center + (result.target_rudder_angle / 45.0f) * 400.0f);

    *output = result;
}
