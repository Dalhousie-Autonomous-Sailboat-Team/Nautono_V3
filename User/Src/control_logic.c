#include "control_logic.h"

#include <stddef.h>

#define SAIL_DEAD_BAND_DEG 20.0f
#define MOTOR_FULL 12800U
#define MOTOR_OFF 0U

/* Treat commands as stale after twice their nominal transmission period. */
#define XBEE_TIMEOUT_MS 2000U // 2x the send rate
#define RPI_TIMEOUT_MS 10000U // 2x the send rate

/* Compensate for the rudder linkage's mechanical center misalignment. */
#define RUDDER_ANGLE_OFFSET_PWM -130.0f

/*
 * Map sail commands (-45 to +45 degrees sail range) to the AS5600's 0-to-360-degree
 * scale. SAIL_CENTER_DEG is the encoder reading at a zero-degree command;
 * tune it to the physical setup.
 */
#define SAIL_CENTER_DEG 180.0f

static float wrap_error(float error)
{
    /* Use the shortest signed error across the encoder's 0/360 boundary. */
    while (error > 180.0f)
        error -= 360.0f;
    while (error < -180.0f)
        error += 360.0f;
    return error;
}

static float sail_command_to_encoder_deg(float sail_angle)
{
    /* Linear map: -45 -> center - 45, 0 -> center, +45 -> center + 45. */
    float deg = SAIL_CENTER_DEG + sail_angle;

    /* Wrap the target encoder angle into [0, 360). */
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

    /*
     * Sail angle fallback hierarchy: a non-zero XBee command overrides the
     * RPi when both sources are valid; otherwise use the remaining valid
     * source, or command zero when neither source is valid.
     */
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

    /* Apply the same fallback hierarchy independently to the rudder angle. */
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

    if (result.xbee_valid || result.rpi_valid)
    {
        float target_encoder_angle = sail_command_to_encoder_deg(result.target_sail_angle);
        float sail_error = wrap_error(target_encoder_angle - input->encoder.angle);

        /* Bang-bang sail control: drive at full duty outside the dead band. */
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
    }
    else
    {
        /* No fresh command means no sail movement, regardless of encoder position. */
        result.sail_motor_channel_1_pwm = MOTOR_OFF;
        result.sail_motor_channel_2_pwm = MOTOR_OFF;
    }

    /* Direct rudder control maps +/-45 degrees to +/-400 PWM around center. */
    float rudder_center = 1500.0f + RUDDER_ANGLE_OFFSET_PWM;
    result.rudder_pwm =
        (uint16_t)(rudder_center + (result.target_rudder_angle / 45.0f) * 400.0f);

    *output = result;
}
