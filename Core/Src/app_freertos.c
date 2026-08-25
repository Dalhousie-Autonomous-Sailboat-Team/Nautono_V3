/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : app_freertos.c
 * Description        : FreeRTOS applicative file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "L1_user_comms.h"
#include "L2_app_types.h"
#include "L3_telemetry.h"
#include "L2_rpi.h"
#include "L2_motor_control.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for initTask */
osThreadId_t initTaskHandle;
const osThreadAttr_t initTask_attributes = {
  .name = "initTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 4
};
/* Definitions for uartParserTask */
osThreadId_t uartParserTaskHandle;
const osThreadAttr_t uartParserTask_attributes = {
  .name = "uartParserTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for heartbeatTask */
osThreadId_t heartbeatTaskHandle;
const osThreadAttr_t heartbeatTask_attributes = {
  .name = "heartbeatTask",
  .priority = (osPriority_t) osPriorityBelowNormal,
  .stack_size = 128 * 4
};
/* Definitions for encoderTask */
osThreadId_t encoderTaskHandle;
const osThreadAttr_t encoderTask_attributes = {
  .name = "encoderTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for telemetryTask */
osThreadId_t telemetryTaskHandle;
const osThreadAttr_t telemetryTask_attributes = {
  .name = "telemetryTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for rpiTransmitTask */
osThreadId_t rpiTransmitTaskHandle;
const osThreadAttr_t rpiTransmitTask_attributes = {
  .name = "rpiTransmitTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for sailMotorTask */
osThreadId_t sailMotorTaskHandle;
const osThreadAttr_t sailMotorTask_attributes = {
  .name = "sailMotorTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for debugPrintStringMutex */
osMutexId_t debugPrintStringMutexHandle;
const osMutexAttr_t debugPrintStringMutex_attributes = {
  .name = "debugPrintStringMutex"
};
/* Definitions for encoderMutex */
osMutexId_t encoderMutexHandle;
const osMutexAttr_t encoderMutex_attributes = {
  .name = "encoderMutex"
};
/* Definitions for rpiMutex */
osMutexId_t rpiMutexHandle;
const osMutexAttr_t rpiMutex_attributes = {
  .name = "rpiMutex"
};
/* Definitions for windMutex */
osMutexId_t windMutexHandle;
const osMutexAttr_t windMutex_attributes = {
  .name = "windMutex"
};
/* Definitions for xbeeMutex */
osMutexId_t xbeeMutexHandle;
const osMutexAttr_t xbeeMutex_attributes = {
  .name = "xbeeMutex"
};
/* Definitions for uart_rx_queue */
osMessageQueueId_t uart_rx_queueHandle;
const osMessageQueueAttr_t uart_rx_queue_attributes = {
  .name = "uart_rx_queue"
};
/* Definitions for i2c2_semaphore */
osSemaphoreId_t i2c2_semaphoreHandle;
const osSemaphoreAttr_t i2c2_semaphore_attributes = {
  .name = "i2c2_semaphore"
};
/* Definitions for radio_tx_semaphore */
osSemaphoreId_t radio_tx_semaphoreHandle;
const osSemaphoreAttr_t radio_tx_semaphore_attributes = {
  .name = "radio_tx_semaphore"
};
/* Definitions for raspberry_tx_semaphore */
osSemaphoreId_t raspberry_tx_semaphoreHandle;
const osSemaphoreAttr_t raspberry_tx_semaphore_attributes = {
  .name = "raspberry_tx_semaphore"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{
    /* Enable TRC */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    /* Reset the cycle counter */
    DWT->CYCCNT = 0;

    /* Enable the cycle counter */
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

__weak unsigned long getRunTimeCounterValue(void)
{
    return DWT->CYCCNT;
}
/* USER CODE END 1 */

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t ulExpectedIdleTime)
{
    /* place for user code */
}

__weak void PostSleepProcessing(uint32_t ulExpectedIdleTime)
{
    /* place for user code */
}
/* USER CODE END PREPOSTSLEEP */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* creation of debugPrintStringMutex */
  debugPrintStringMutexHandle = osMutexNew(&debugPrintStringMutex_attributes);

  /* creation of encoderMutex */
  encoderMutexHandle = osMutexNew(&encoderMutex_attributes);

  /* creation of rpiMutex */
  rpiMutexHandle = osMutexNew(&rpiMutex_attributes);

  /* creation of windMutex */
  windMutexHandle = osMutexNew(&windMutex_attributes);

  /* creation of xbeeMutex */
  xbeeMutexHandle = osMutexNew(&xbeeMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */
  /* creation of i2c2_semaphore */
  i2c2_semaphoreHandle = osSemaphoreNew(1, 0, &i2c2_semaphore_attributes);

  /* creation of radio_tx_semaphore */
  radio_tx_semaphoreHandle = osSemaphoreNew(1, 1, &radio_tx_semaphore_attributes);

  /* creation of raspberry_tx_semaphore */
  raspberry_tx_semaphoreHandle = osSemaphoreNew(1, 1, &raspberry_tx_semaphore_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of uart_rx_queue */
  uart_rx_queueHandle = osMessageQueueNew (32, sizeof(UART_Char_t), &uart_rx_queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of initTask */
  initTaskHandle = osThreadNew(InitTask, NULL, &initTask_attributes);

  /* creation of uartParserTask */
  uartParserTaskHandle = osThreadNew(UARTParserTask, NULL, &uartParserTask_attributes);

  /* creation of heartbeatTask */
  heartbeatTaskHandle = osThreadNew(HeartbeatTask, NULL, &heartbeatTask_attributes);

  /* creation of encoderTask */
  encoderTaskHandle = osThreadNew(EncoderTask, NULL, &encoderTask_attributes);

  /* creation of telemetryTask */
  telemetryTaskHandle = osThreadNew(TelemetryTask, NULL, &telemetryTask_attributes);

  /* creation of rpiTransmitTask */
  rpiTransmitTaskHandle = osThreadNew(RpiTransmitTask, NULL, &rpiTransmitTask_attributes);

  /* creation of sailMotorTask */
  sailMotorTaskHandle = osThreadNew(SailMotorTask, NULL, &sailMotorTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

