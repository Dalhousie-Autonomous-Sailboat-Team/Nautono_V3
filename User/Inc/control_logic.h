#ifndef CONTROL_LOGIC_H
#define CONTROL_LOGIC_H

#include <stdbool.h>
#include <stdint.h>

#include "app_state.h"
#include "encoder.h"

typedef struct
{
    uint32_t now_ms;
    XbeeCommand_t xbee;
    RPiSample_t rpi;
    EncoderSample_t encoder;
} ControlLogicInput_t;

typedef struct
{
    bool xbee_valid;
    bool rpi_valid;
    float target_sail_angle;
    float target_rudder_angle;
    uint16_t sail_motor_channel_1_pwm;
    uint16_t sail_motor_channel_2_pwm;
    uint16_t rudder_pwm;
} ControlLogicOutput_t;

void ControlLogic_Evaluate(const ControlLogicInput_t *input, ControlLogicOutput_t *output);

#endif /* CONTROL_LOGIC_H */
