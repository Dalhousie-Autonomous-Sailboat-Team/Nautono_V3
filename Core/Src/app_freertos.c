/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "debug.h"
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
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for Measure_Power */
osThreadId_t Measure_PowerHandle;
const osThreadAttr_t Measure_Power_attributes = {
  .name = "Measure_Power",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for DebugUART */
osThreadId_t DebugUARTHandle;
const osThreadAttr_t DebugUART_attributes = {
  .name = "DebugUART",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for Debug_Blink_On */
osTimerId_t Debug_Blink_OnHandle;
const osTimerAttr_t Debug_Blink_On_attributes = {
  .name = "Debug_Blink_On"
};
/* Definitions for Debug_Blink_Off */
osTimerId_t Debug_Blink_OffHandle;
const osTimerAttr_t Debug_Blink_Off_attributes = {
  .name = "Debug_Blink_Off"
};
/* Definitions for PrintMessageQueue */
osMessageQueueId_t PrintMessageQueueHandle;
const osMessageQueueAttr_t PrintMessageQueue_attributes = {
  .name = "PrintMessageQueue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* USER CODE BEGIN PREPOSTSLEEP */
__weak void PreSleepProcessing(uint32_t ulExpectedIdleTime)
{
/* place for user code */
UNUSED(ulExpectedIdleTime);
}

__weak void PostSleepProcessing(uint32_t ulExpectedIdleTime)
{
/* place for user code */
UNUSED(ulExpectedIdleTime);
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

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */
  /* creation of Debug_Blink_On */
  Debug_Blink_OnHandle = osTimerNew(Set_LED, osTimerPeriodic, NULL, &Debug_Blink_On_attributes);

  /* creation of Debug_Blink_Off */
  Debug_Blink_OffHandle = osTimerNew(Clear_LED, osTimerOnce, NULL, &Debug_Blink_Off_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of PrintMessageQueue */
  PrintMessageQueueHandle = osMessageQueueNew (16, sizeof(DebugMessage_t), &PrintMessageQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Measure_Power */
  Measure_PowerHandle = osThreadNew(MeasurePower, NULL, &Measure_Power_attributes);

  /* creation of DebugUART */
  DebugUARTHandle = osThreadNew(DebugUART, NULL, &DebugUART_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the defaultTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
__weak void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN defaultTask */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END defaultTask */
}

/* USER CODE BEGIN Header_MeasurePower */
/**
* @brief Function implementing the Measure_Power thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_MeasurePower */
__weak void MeasurePower(void *argument)
{
  /* USER CODE BEGIN Measure_Power */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Measure_Power */
}

/* USER CODE BEGIN Header_DebugUART */
/**
* @brief Function implementing the DebugUART thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_DebugUART */
__weak void DebugUART(void *argument)
{
  /* USER CODE BEGIN DebugUART */
  UNUSED(argument);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END DebugUART */
}

/* Set_LED function */
__weak void Set_LED(void *argument)
{
  /* USER CODE BEGIN Set_LED */
  UNUSED(argument);
  /* USER CODE END Set_LED */
}

/* Clear_LED function */
__weak void Clear_LED(void *argument)
{
  /* USER CODE BEGIN Clear_LED */
  UNUSED(argument);
  /* USER CODE END Clear_LED */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

