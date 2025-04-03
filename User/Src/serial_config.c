/** @file serial_config.c
 *
 * @brief Serial Callbacks and Configuration.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

#include "main.h"
#include "cmsis_os2.h"

#include "sensor.h"

#include <stdbool.h>

extern osEventFlagsId_t I2C1_EventHandle;
extern osEventFlagsId_t I2C2_EventHandle;
extern osEventFlagsId_t UART4_EventHandle;

/**
 * @brief UART Receive Interrupt Callback
 * 
 * @param huart 
 * @param Size 
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
 {
   if(huart->Instance == UART4)
   {
      osEventFlagsSet(UART4_EventHandle, RX_FLAG);  // Notify task that data is ready
   }
   UNUSED(Size);
 }

 /**
  * @brief UART Transmit Interrupt Callback
  * 
  * @param huart 
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)  // Check if it is UART4
    {
        osEventFlagsSet(UART4_EventHandle, TX_FLAG);  // Notify task that transmission is complete
    }
}

/**
 * @brief I2C Receive Complete Callback
 * 
 * @param hi2c 
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if(hi2c->Instance == I2C1)
  {
    osEventFlagsSet(I2C1_EventHandle, RX_FLAG);  // Notify I2C1 task that data is ready
  }
  else if(hi2c->Instance == I2C2)
  {
    osEventFlagsSet(I2C2_EventHandle, RX_FLAG);  // Notify I2C2 task that data is ready
  }
}

/**
 * @brief I2C Memory Transmit Complete Callback
 * - Called when a write to specific slave memory address is complete
 * 
 * @param hi2c 
 */
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if(hi2c->Instance == I2C1)
  {
    osEventFlagsSet(I2C1_EventHandle, TX_FLAG);  // Notify I2C1 task that transmission is complete
  }
  else if(hi2c->Instance == I2C2)
  {
    osEventFlagsSet(I2C2_EventHandle, TX_FLAG);  // Notify I2C2 task that transmission is complete
  }
}

/**
 * @brief I2C Master Transmit Complete Callback
 * - Called when a write to a slave without specifying a memory address is complete
 * 
 * @param hi2c
 */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if(hi2c->Instance == I2C1)
  {
    osEventFlagsSet(I2C1_EventHandle, TX_FLAG);  // Notify I2C1 task that transmission is complete
  }
  else if(hi2c->Instance == I2C2)
  {
    osEventFlagsSet(I2C2_EventHandle, TX_FLAG);  // Notify I2C2 task that transmission is complete
  }
}

/**
 * @brief I2C Error Interrupt Callback
 * 
 * @param hi2c 
 */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    uint32_t error_code = hi2c->ErrorCode;

    if (error_code & HAL_I2C_ERROR_BERR) 
    {
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_BERR);
    }
    if (error_code & HAL_I2C_ERROR_ARLO) 
    {
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_ARLO);
    }
    if (error_code & HAL_I2C_ERROR_AF) 
    {
        // Acknowledge failure - Could mean sensor is disconnected
    }
    if (error_code & HAL_I2C_ERROR_OVR) 
    {
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_OVR);
    }
    if (error_code & HAL_I2C_ERROR_TIMEOUT) 
    {
        // Reset the I2C bus if needed
    }

    if(hi2c->Instance == I2C1)
    {
        osEventFlagsSet(I2C1_EventHandle, ERR_FLAG);  // Notify I2C1 task that an error occurred
    }
    else if(hi2c->Instance == I2C2)
    {
        osEventFlagsSet(I2C2_EventHandle, ERR_FLAG);  // Notify I2C2 task that an error occurred
    }
}