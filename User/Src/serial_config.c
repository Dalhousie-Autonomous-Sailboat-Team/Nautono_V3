/** @file serial_config.c
 *
 * @brief Serial Callbacks and Configuration.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

 #include "main.h"

extern uint16_t debug_receive_size;
extern volatile uint8_t debug_RxDone;

 void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
 {
   if(huart->Instance == UART4)
   {
        /* Set the RxDone flag */
        debug_RxDone = 1;
        debug_receive_size = Size;
   }
 }