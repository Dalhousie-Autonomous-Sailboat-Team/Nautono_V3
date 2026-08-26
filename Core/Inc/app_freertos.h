/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : app_freertos.h
 * Description        : FreeRTOS applicative header file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_FREERTOS_H__
#define __APP_FREERTOS_H__

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Exported macro -------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */
extern osThreadId_t initTaskHandle;
extern osThreadId_t uartParserTaskHandle;
extern osThreadId_t heartbeatTaskHandle;
extern osThreadId_t encoderTaskHandle;
extern osThreadId_t telemetryTaskHandle;
extern osThreadId_t rpiTransmitTaskHandle;
extern osThreadId_t sailMotorTaskHandle;
extern osMutexId_t debugPrintStringMutexHandle;
extern osMutexId_t encoderMutexHandle;
extern osMutexId_t rpiMutexHandle;
extern osMutexId_t windMutexHandle;
extern osMutexId_t xbeeMutexHandle;
extern osMessageQueueId_t uart_rx_queueHandle;
extern osSemaphoreId_t i2c2_semaphoreHandle;
extern osSemaphoreId_t radio_tx_semaphoreHandle;
extern osSemaphoreId_t raspberry_tx_semaphoreHandle;

/* Exported function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

extern void InitTask(void *argument);
extern void UARTParserTask(void *argument);
extern void HeartbeatTask(void *argument);
extern void EncoderTask(void *argument);
extern void TelemetryTask(void *argument);
extern void RpiTransmitTask(void *argument);
extern void BoatControlTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

#ifdef __cplusplus
}
#endif
#endif /* __APP_FREERTOS_H__ */
