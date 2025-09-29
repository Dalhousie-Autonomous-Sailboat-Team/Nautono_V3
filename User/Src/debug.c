/** @file debug.c
 *
 * @brief Implement debug functionality.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "app_freertos.h"

#define LED_FLASH_PERIOD 2000
#define LED_FLASH_TIME 50

#define TX_FLAG 0x01
#define RX_FLAG 0x02
#define ERR_FLAG 0x04

#define DEBUG_TASK_FLAG (1 << 0)

extern osTimerId_t Debug_Blink_OnHandle;
extern osTimerId_t Debug_Blink_OffHandle;
extern osMessageQueueId_t PrintMessageQueueHandle;
extern UART_HandleTypeDef huart4;
extern osEventFlagsId_t UART4_EventHandle;

volatile uint8_t uart4_rx_busy = false;
volatile uint8_t uart4_tx_busy = false;

/* Task Overwrites */

/**
 * @brief Function implementing the defaultTask thread.
 * @param argument: Not used
 * @retval None
 */
void StartDefaultTask(void *argument)
{
#ifdef ENABLE_DEBUG_HEARTBEAT
  /* Start Debug Heartbeat Timer */
  osTimerStart(Debug_Blink_OnHandle, LED_FLASH_PERIOD);
#endif

  /* Suspend Task Indefinitely */
  vTaskSuspend(NULL);
  UNUSED(argument);
}

/* Timer Overwrites */

/**
 * @brief Set the LED on.  Start the off timer.
 *
 * @param argument
 */
void Set_LED(void *argument)
{
  HAL_GPIO_WritePin(DEBUG_LED1_GPIO_Port, DEBUG_LED1_Pin, GPIO_PIN_SET);
  osTimerStart(Debug_Blink_OffHandle, LED_FLASH_TIME);
  UNUSED(argument);
}

/**
 * @brief Clear the LED.  This is a one-shot timer.
 *
 * @param argument
 */
void Clear_LED(void *argument)
{
  HAL_GPIO_WritePin(DEBUG_LED1_GPIO_Port, DEBUG_LED1_Pin, GPIO_PIN_RESET);
  UNUSED(argument);
}

/**
 * @brief Send all debug messages to the UART.  Blocks until messages waiting.
 * @param argument: Not used
 * @retval None
 */
void DebugUART(void *argument)
{
  DebugMessage_t debugMessage;
  /* Infinite loop */
  while (true)
  {
    /* If there are messages in the queue, send them to the UART */
    while (osMessageQueueGet(PrintMessageQueueHandle, &debugMessage, NULL, 0) == osOK)
    {
      /* Ensure the UART bus is not busy before initiating the transfer */
      while (HAL_UART_GetState(&huart4) != HAL_UART_STATE_READY)
      {
        osThreadYield(); /* Yield to other equal priority tasks while waiting */
      }
      /* Clear any previous event flags before starting a new transaction */
      osEventFlagsClear(UART4_EventHandle, TX_FLAG | ERR_FLAG);
      HAL_UART_Transmit_DMA(&huart4, (uint8_t *)debugMessage.message, strlen(debugMessage.message));
      /* Block task until UART handle finishes TX or an error occurs */
      osEventFlagsWait(UART4_EventHandle, TX_FLAG | ERR_FLAG, osFlagsWaitAny, osWaitForever);
    }
    /* Wait for next message */
    osThreadFlagsWait(DEBUG_TASK_FLAG, osFlagsWaitAny, osWaitForever);
  }
  UNUSED(argument);
}

/**
 * @brief Asynchronous debug printf.  Note that this function is not thread-safe.
 * - If data is changed before being added to the queue, it may be corrupted.
 * - This can be a problem for frequent tasks that call this function.
 *
 */
void DebugPrintf(const char *format, ...)
{
  static DebugMessage_t debugMessage;

  va_list args;
  va_start(args, format);
  vsnprintf(debugMessage.message, DEBUG_MESSAGE_SIZE, format, args);
  va_end(args);
  if (osMessageQueuePut(PrintMessageQueueHandle, &debugMessage, 0, 0) == osOK)
  {
    /* Alert Debug Task of Waiting Messages */
    osThreadFlagsSet(DebugUARTHandle, DEBUG_TASK_FLAG);
  }
}