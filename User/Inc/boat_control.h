#ifndef boat_control_H
#define boat_control_H

#include <stdint.h>
#include <stdbool.h>

#define SERVO_DUTY_CYCLE_MIN 1000U
#define SERVO_DUTY_CYCLE_MAX 2000U

#define MOTOR_DUTY_CYCLE_MIN 0U
#define MOTOR_DUTY_CYCLE_MAX 3200U

typedef enum
{
    PWM_CHANNEL_RUDDER_SERVO,
    PWM_CHANNEL_MAST_1_MOTOR,
    PWM_CHANNEL_MAST_2_MOTOR,
    PWM_CHANNEL_FLAP_1_MOTOR,
    PWM_CHANNEL_FLAP_2_MOTOR,
    PWM_CHANNEL_FLAP2_1_MOTOR,
    PWM_CHANNEL_FLAP2_2_MOTOR,
    PWM_CHANNEL_COUNT
} PWM_Channel_t;

typedef struct
{
    PWM_Channel_t channel;
    int16_t drive;
} PWM_Duty_Cycle_t;

typedef enum
{
    MODE_AUTONOMOUS, // Rpi controls sail and rudder. Can be overriden by Xbee commands
    MODE_MANUAL, // Xbee controls sail and rudder, Rpi is ignored
    MODE_WIND_FOLLOWING // Windvane controls sail, Rpi is ignored. Can be overriden by Xbee commands
} BoatMode_t;

void PWM_Init(void);
void PWM_SetDutyCycle(PWM_Channel_t channel, uint16_t duty_cycle);

void BoatControlTask(void *argument);

#endif /* boat_control_H */