/** @file feedback.h
 *
 * @brief Tasks and functions for feedback control loops.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST.  All rights reserved.
 */

#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <stdint.h>
#include <stdbool.h>

typedef struct feedback_t
{
    float kp;         // Proportional gain
    float ki;         // Integral gain
    float dt;         // Time step
    float integral;   // Integral term accumulator
    float setpoint;   // Desired setpoint
    float output_min; // Minimum output limit
    float output_max; // Maximum output limit
} feedback_t;

#endif /* FEEDBACK_H */