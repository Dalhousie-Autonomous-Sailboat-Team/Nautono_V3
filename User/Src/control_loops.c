/** @file control_loop.c
 *
 * @brief PI control loop implementations for mast and flap motors.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

/**
 * @brief Function implementing the Mast_Control_Loop thread.
 * This task is initiated by the reception of a new angle measurement from the mast angle sensor
 * @param argument: Not used
 * @retval None
 */
void Mast_Control_Loop(void *argument)
{
    /* Infinite loop */
    for (;;)
    {
        osDelay(1000);
    }
}

/**
 * @brief Function implementing the Flap_Control_Loop thread.
 * This task is initiated by the reception of a new angle measurement from the flap angle sensor
 * @param argument: Not used
 * @retval None
 */
void Flap_Control_Loop(void *argument)
{
    /* Infinite loop */
    for (;;)
    {
        osDelay(1000);
    }
}
