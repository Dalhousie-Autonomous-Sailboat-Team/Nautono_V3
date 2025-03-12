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

#define DEBUG_TASK_FLAG (1 << 0)

extern osTimerId_t Debug_Blink_OnHandle;
extern osTimerId_t Debug_Blink_OffHandle;
extern osMessageQueueId_t PrintMessageQueueHandle;
extern UART_HandleTypeDef huart4;

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

    /* Infinite loop */
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
    DEBUG_PRINT("Set_LED\n");
    DEBUG_PRINT("Setting LED...\n");
    HAL_GPIO_WritePin(DEBUG_LED1_GPIO_Port, DEBUG_LED1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIO1_GPIO_Port, GPIO1_Pin, GPIO_PIN_SET);
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
    HAL_GPIO_WritePin(GPIO1_GPIO_Port, GPIO1_Pin, GPIO_PIN_RESET);
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
  while(true)
  {
    osThreadFlagsWait(DEBUG_TASK_FLAG, osFlagsWaitAny, osWaitForever);

    while(osMessageQueueGet(PrintMessageQueueHandle, &debugMessage, NULL, 0) == osOK)
    {
      while(uart4_tx_busy == true)
      {
        osDelay(1);
      }
      uart4_tx_busy = true;
      HAL_UART_Transmit_DMA(&huart4, (uint8_t *)debugMessage.message, strlen(debugMessage.message));
    }
    
  }
  UNUSED(argument);
}

/**
 * @brief Asynchronous debug printf.
 * 
 */
void DebugPrintf(const char *format, ...)
{
  DebugMessage_t debugMessage;

  va_list args;
  va_start(args, format);
  vsnprintf(debugMessage.message, DEBUG_MESSAGE_SIZE, format, args);
  va_end(args);
  if(osMessageQueuePut(PrintMessageQueueHandle, &debugMessage, 0, 0) == osOK)
  {
    /* Alert Debug Task of Waiting Messages */
    osThreadFlagsSet(DebugUARTHandle, DEBUG_TASK_FLAG);
  }
}