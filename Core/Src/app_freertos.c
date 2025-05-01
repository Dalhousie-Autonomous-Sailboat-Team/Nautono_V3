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
/* Definitions for Measure_Angles */
osThreadId_t Measure_AnglesHandle;
const osThreadAttr_t Measure_Angles_attributes = {
  .name = "Measure_Angles",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 256 * 4
};
/* Definitions for Control_Motors */
osThreadId_t Control_MotorsHandle;
const osThreadAttr_t Control_Motors_attributes = {
  .name = "Control_Motors",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 4
};
/* Definitions for Radio_Control */
osThreadId_t Radio_ControlHandle;
const osThreadAttr_t Radio_Control_attributes = {
  .name = "Radio_Control",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for PowerConversionData */
osMutexId_t PowerConversionDataHandle;
const osMutexAttr_t PowerConversionData_attributes = {
  .name = "PowerConversionData"
};
/* Definitions for AngleData */
osMutexId_t AngleDataHandle;
const osMutexAttr_t AngleData_attributes = {
  .name = "AngleData"
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
/* Definitions for I2C1_Event */
osEventFlagsId_t I2C1_EventHandle;
const osEventFlagsAttr_t I2C1_Event_attributes = {
  .name = "I2C1_Event"
};
/* Definitions for Power_Event */
osEventFlagsId_t Power_EventHandle;
const osEventFlagsAttr_t Power_Event_attributes = {
  .name = "Power_Event"
};
/* Definitions for UART4_Event */
osEventFlagsId_t UART4_EventHandle;
const osEventFlagsAttr_t UART4_Event_attributes = {
  .name = "UART4_Event"
};
/* Definitions for I2C2_Event */
osEventFlagsId_t I2C2_EventHandle;
const osEventFlagsAttr_t I2C2_Event_attributes = {
  .name = "I2C2_Event"
};
/* Definitions for UART8_Event */
osEventFlagsId_t UART8_EventHandle;
const osEventFlagsAttr_t UART8_Event_attributes = {
  .name = "UART8_Event"
};
/* Definitions for Radio_Event */
osEventFlagsId_t Radio_EventHandle;
const osEventFlagsAttr_t Radio_Event_attributes = {
  .name = "Radio_Event"
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
  /* creation of PowerConversionData */
  PowerConversionDataHandle = osMutexNew(&PowerConversionData_attributes);

  /* creation of AngleData */
  AngleDataHandle = osMutexNew(&AngleData_attributes);

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

  /* creation of Measure_Angles */
  Measure_AnglesHandle = osThreadNew(Measure_Angles, NULL, &Measure_Angles_attributes);

  /* creation of Control_Motors */
  Control_MotorsHandle = osThreadNew(Control_Motors, NULL, &Control_Motors_attributes);

  /* creation of Radio_Control */
  Radio_ControlHandle = osThreadNew(Radio_Control, NULL, &Radio_Control_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* creation of I2C1_Event */
  I2C1_EventHandle = osEventFlagsNew(&I2C1_Event_attributes);

  /* creation of Power_Event */
  Power_EventHandle = osEventFlagsNew(&Power_Event_attributes);

  /* creation of UART4_Event */
  UART4_EventHandle = osEventFlagsNew(&UART4_Event_attributes);

  /* creation of I2C2_Event */
  I2C2_EventHandle = osEventFlagsNew(&I2C2_Event_attributes);

  /* creation of UART8_Event */
  UART8_EventHandle = osEventFlagsNew(&UART8_Event_attributes);

  /* creation of Radio_Event */
  Radio_EventHandle = osEventFlagsNew(&Radio_Event_attributes);

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

/* USER CODE BEGIN Header_Measure_Angles */
/**
* @brief Function implementing the Measure_Angles thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Measure_Angles */
__weak void Measure_Angles(void *argument)
{
  /* USER CODE BEGIN Measure_Angles */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  UNUSED(argument);
  /* USER CODE END Measure_Angles */
}

/* USER CODE BEGIN Header_Control_Motors */
/**
* @brief Function implementing the Control_Motors thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Control_Motors */
__weak void Control_Motors(void *argument)
{
  /* USER CODE BEGIN Control_Motors */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  UNUSED(argument);
  /* USER CODE END Control_Motors */
}

/* USER CODE BEGIN Header_Radio_Control */
/**
* @brief Function implementing the Radio_Control thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Radio_Control */
__weak void Radio_Control(void *argument)
{
  /* USER CODE BEGIN Radio_Control */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  UNUSED(argument);
  /* USER CODE END Radio_Control */
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

