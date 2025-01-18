/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MPPT2_IN1_Pin GPIO_PIN_5
#define MPPT2_IN1_GPIO_Port GPIOE
#define MPPT2_IN2_Pin GPIO_PIN_6
#define MPPT2_IN2_GPIO_Port GPIOE
#define MPPT1_IN2_Pin GPIO_PIN_2
#define MPPT1_IN2_GPIO_Port GPIOC
#define MOTOR4_OUT1_Pin GPIO_PIN_0
#define MOTOR4_OUT1_GPIO_Port GPIOA
#define MOTOR2_OUT2_Pin GPIO_PIN_1
#define MOTOR2_OUT2_GPIO_Port GPIOA
#define MOTOR4_OUT2_Pin GPIO_PIN_2
#define MOTOR4_OUT2_GPIO_Port GPIOA
#define MOTOR1_OUT1_Pin GPIO_PIN_9
#define MOTOR1_OUT1_GPIO_Port GPIOE
#define MOTOR1_OUT2_Pin GPIO_PIN_11
#define MOTOR1_OUT2_GPIO_Port GPIOE
#define HSE_ENABLE_Pin GPIO_PIN_9
#define HSE_ENABLE_GPIO_Port GPIOC
#define DEBUG_LED1_Pin GPIO_PIN_8
#define DEBUG_LED1_GPIO_Port GPIOA
#define DEBUG_LED2_Pin GPIO_PIN_9
#define DEBUG_LED2_GPIO_Port GPIOA
#define GPIO1_Pin GPIO_PIN_10
#define GPIO1_GPIO_Port GPIOA
#define GPIO2_Pin GPIO_PIN_11
#define GPIO2_GPIO_Port GPIOA
#define GPIO3_Pin GPIO_PIN_12
#define GPIO3_GPIO_Port GPIOA
#define MOTOR2_OUT1_Pin GPIO_PIN_15
#define MOTOR2_OUT1_GPIO_Port GPIOA
#define GPIO4_Pin GPIO_PIN_10
#define GPIO4_GPIO_Port GPIOC
#define GPIO5_Pin GPIO_PIN_11
#define GPIO5_GPIO_Port GPIOC
#define GPIO6_Pin GPIO_PIN_12
#define GPIO6_GPIO_Port GPIOC
#define GPIO7_Pin GPIO_PIN_0
#define GPIO7_GPIO_Port GPIOD
#define GPIO8_Pin GPIO_PIN_1
#define GPIO8_GPIO_Port GPIOD
#define MOTOR3_OUT1_Pin GPIO_PIN_4
#define MOTOR3_OUT1_GPIO_Port GPIOB
#define MOTOR3_OUT2_Pin GPIO_PIN_5
#define MOTOR3_OUT2_GPIO_Port GPIOB
#define MPPT1_IN1_Pin GPIO_PIN_7
#define MPPT1_IN1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
