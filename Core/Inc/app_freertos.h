/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.h
  * Description        : FreeRTOS applicative header file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
extern osThreadId_t defaultTaskHandle;
extern osThreadId_t DebugUARTHandle;
extern osThreadId_t Measure_AnglesHandle;
extern osThreadId_t Control_MotorsHandle;
extern osThreadId_t Radio_ControlHandle;
extern osThreadId_t Mast_ControlHandle;
extern osMutexId_t PowerConversionDataHandle;
extern osMutexId_t AngleDataHandle;
extern osTimerId_t Debug_Blink_OnHandle;
extern osTimerId_t Debug_Blink_OffHandle;
extern osMessageQueueId_t PrintMessageQueueHandle;
extern osEventFlagsId_t I2C1_EventHandle;
extern osEventFlagsId_t Power_EventHandle;
extern osEventFlagsId_t UART4_EventHandle;
extern osEventFlagsId_t I2C2_EventHandle;
extern osEventFlagsId_t UART8_EventHandle;
extern osEventFlagsId_t Radio_EventHandle;
extern osEventFlagsId_t Motor_Control_EventHandle;

/* Exported function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

extern void StartDefaultTask(void *argument);
extern void DebugUART(void *argument);
extern void Measure_Angles(void *argument);
extern void Control_Motors(void *argument);
extern void Radio_Control(void *argument);
extern void Mast_Control(void *argument);
void Set_LED(void *argument);
void Clear_LED(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

#ifdef __cplusplus
}
#endif
#endif /* __APP_FREERTOS_H__ */
