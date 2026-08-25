#ifndef PID_H
#define PID_H

typedef struct
{
    /* Gains */
    float kp;
    float ki;
    float kd;

    /* State */
    float integral;
    float prev_error;

    /* Output clamping */
    float output_min;
    float output_max;

    /* Integral clamping — prevents windup */
    float integral_min;
    float integral_max;

} PID_t;

void  PID_Init(PID_t *pid, float kp, float ki, float kd,
               float output_min, float output_max,
               float integral_min, float integral_max);
void  PID_Reset(PID_t *pid);
float PID_Update(PID_t *pid, float setpoint, float measured, float dt);

#endif // PID_H