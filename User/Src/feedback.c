/** @file feedback.c
 *
 * @brief Tasks and functions for feedback control loops.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"

#include "feedback.h"
#include "motor.h"
#include "sensor.h"
#include "debug.h"
#include "motor.h"

extern osEventFlagsId_t Motor_Control_EventHandle;
extern osMutexId_t AngleDataHandle;
extern int16_t raw_angle_data[ANGLE_SENSOR_COUNT][2]; /* Shared memory for raw angle data */

float output = 0.0f;

/**
 * @brief Compute shortest signed error between setpoint and measurement in degrees.
 *
 * @param setpoint Desired setpoint in degrees
 * @param measurement Current measurement in degrees
 * @return Shortest signed error in degrees
 */
static float angle_error(float setpoint, float measurement)
{
    float error = setpoint - measurement;
    while (error > 180.0f)
    {
        error -= 360.0f;
    }
    while (error < -180.0f)
    {
        error += 360.0f;
    }
    return error;
}

/**
 * @brief PI controller update with anti-windup.
 *
 * @param c Pointer to feedback controller structure
 * @param measurement Current measurement
 * @param dt Time step in seconds
 * @return Control output in the range [-1, 1]
 */
static float PI_Update(feedback_t *c, float measurement, float dt)
{
    float error = angle_error(c->setpoint, measurement);

    /* Proportional term */
    float p_term = c->kp * error;

    /* Tentative integral update */
    float new_integral = c->integral + (c->ki * error * dt);

    /* Compute unclamped output */
    output = p_term + new_integral;

    /* Clamp and apply anti-windup */
    if (output > c->output_max)
    {
        output = c->output_max;
        if (error > 0)
            new_integral = c->integral; /* freeze integral growth */
    }
    else if (output < c->output_min)
    {
        output = c->output_min;
        if (error < 0)
            new_integral = c->integral;
    }

    /* Commit integral */
    c->integral = new_integral;

    return output;
}

/**
 * @brief PI Control Loop for Mast Position
 *
 * This task implements a PI control loop to adjust the mast position based on feedback from the Mast AS5600.
 * This task is called from the measurement task after reading the mast angle sensor.
 *
 * @param argument
 */
void Mast_Control(void *argument)
{
    /* Initialize the feedback controller parameters */
    static feedback_t mast_controller = {
        .kp = 0.10f,
        .ki = 0.01f,
        .dt = 0.001f, /* Default time step (will be updated dynamically) */
        .integral = 0.0f,
        .setpoint = 80.0f, /* Desired setpoint (to be set externally) */
        .output_min = -100,
        .output_max = 100};

    uint32_t last_tick = osKernelGetTickCount();

    while (true)
    {
        /* Wait for the Mast Angle Ready Flag */
        osEventFlagsWait(Motor_Control_EventHandle, MAST_ANGLE_READY_FLAG, osFlagsWaitAny, osWaitForever);

        /* Acquire Mutex to read from shared memory */
        osMutexAcquire(AngleDataHandle, osWaitForever);
        int16_t raw_angle = (raw_angle_data[0][0] << 8) | raw_angle_data[0][1]; /* Assuming mast is channel 0 */
        osMutexRelease(AngleDataHandle);

        /* Convert to degrees (0–360) */
        float current_angle = (raw_angle / 4095.0f) * 360.0f;

        /* Compute elapsed time in seconds */
        uint32_t now = osKernelGetTickCount();
        float dt = (now - last_tick) / 1000.0f;
        last_tick = now;

        /* Update PI controller */
        output = PI_Update(&mast_controller, current_angle, dt);

        /* Set mast speed based on controller output */
        Set_Mast_Speed(output);
    }
    UNUSED(argument);
}
