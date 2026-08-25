#include "pid.h"

/* Clamp helper */
static float Clamp(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void PID_Init(PID_t *pid, float kp, float ki, float kd,
              float output_min, float output_max,
              float integral_min, float integral_max)
{
    pid->kp           = kp;
    pid->ki           = ki;
    pid->kd           = kd;
    pid->output_min   = output_min;
    pid->output_max   = output_max;
    pid->integral_min = integral_min;
    pid->integral_max = integral_max;

    PID_Reset(pid);
}

void PID_Reset(PID_t *pid)
{
    pid->integral   = 0.0f;
    pid->prev_error = 0.0f;
}

float PID_Update(PID_t *pid, float setpoint, float measured, float dt)
{
    /* ── Proportional ────────────────────────────────────────────────── */
    float error = setpoint - measured;

    /* ── Integral with anti-windup clamp ─────────────────────────────── */
    pid->integral += error * dt;
    pid->integral  = Clamp(pid->integral, pid->integral_min, pid->integral_max);

    /* ── Derivative (on measurement, not error, to avoid derivative kick) */
    float derivative = (error - pid->prev_error) / dt;
    pid->prev_error  = error;
    
    /* ── Output ──────────────────────────────────────────────────────── */
    float output = (pid->kp * error)
                 + (pid->ki * pid->integral)
                 + (pid->kd * derivative);

    return Clamp(output, pid->output_min, pid->output_max);
}