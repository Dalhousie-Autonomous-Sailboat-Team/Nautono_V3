/** @file xsens.c
 *
 * @brief Used to receive GPS, heading, and orientation packets from IMU
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

 #include "main.h"
 #include "cmsis_os2.h"
 #include "xsens.h"

 
/**
* @brief Function implementing the Xsens_Comms thread.
* @param argument: Not used
* @retval None
*/
\
void Xsens_Comms(void *argument)
{

  /* Infinite loop */
 while(true) 
  {
    osDelay(1);
  }

}

