/** @file serial_config.c
 *
 * @brief Serial Callbacks and Configuration.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

 #include "main.h"

 #include <stdbool.h>

extern volatile uint8_t uart4_rx_busy;
extern volatile uint8_t uart4_tx_busy;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
 {
   if(huart->Instance == UART4)
   {
        /* Set the RxDone flag */
        uart4_rx_busy = false;
   }
   UNUSED(Size);
 }

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)  // Check if it is UART4
    {
        /* Set the TxDone flag */
        uart4_tx_busy = false;
    }
}