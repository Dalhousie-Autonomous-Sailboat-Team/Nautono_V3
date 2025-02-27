/** @file blink.c
 *
 * @brief Debug LED blinking module.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2024 DalMAST. All rights reserved.
 */

 #include "blink.h"
 
 /* Include Main.h For Pin Names */
 #include "main.h"
 
 /*!
  * @brief blink initialization function: Initializes the debug LED
  */
 void
 blink_init (void)
 {
 
 }
 
 /*!
  * @brief blink superloop function: Blinks the debug LED once per second
  */
 void
 blink_superloop (void)
 {
     static uint32_t last_time = 0;
     uint32_t current_time = HAL_GetTick();
 
     if( current_time - last_time < MINIMUM_BLINK_PERIOD )
     {
         return;
     }
     HAL_GPIO_TogglePin(DEBUG_LED1_GPIO_Port, DEBUG_LED1_Pin);
     last_time = current_time;
 }