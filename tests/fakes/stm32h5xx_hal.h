#ifndef TEST_FAKE_STM32H5XX_HAL_H
#define TEST_FAKE_STM32H5XX_HAL_H

#include <stdint.h>

typedef struct
{
    int id;
} I2C_HandleTypeDef;

typedef struct
{
    int id;
} UART_HandleTypeDef;

typedef struct
{
    int id;
} GPIO_TypeDef;

typedef enum
{
    HAL_OK = 0,
    HAL_ERROR = 1,
    HAL_BUSY = 2,
    HAL_TIMEOUT = 3
} HAL_StatusTypeDef;

typedef enum
{
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET = 1
} GPIO_PinState;

#define HAL_MAX_DELAY 0xFFFFFFFFU
#define PWR_MAINREGULATOR_ON 1U
#define PWR_SLEEPENTRY_WFI 2U

HAL_StatusTypeDef HAL_I2C_Master_Transmit_IT(I2C_HandleTypeDef *handle,
                                             uint16_t address,
                                             uint8_t *data,
                                             uint16_t length);
HAL_StatusTypeDef HAL_I2C_Master_Receive_IT(I2C_HandleTypeDef *handle,
                                            uint16_t address,
                                            uint8_t *data,
                                            uint16_t length);
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *handle,
                                     uint8_t *data,
                                     uint16_t length);
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *handle,
                                      uint8_t *data,
                                      uint16_t length);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *handle,
                                   uint8_t *data,
                                   uint16_t length,
                                   uint32_t timeout);
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
void HAL_SuspendTick(void);
void HAL_ResumeTick(void);
void HAL_PWR_EnterSLEEPMode(uint32_t regulator, uint8_t entry);

#endif /* TEST_FAKE_STM32H5XX_HAL_H */
