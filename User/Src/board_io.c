/**
 * @file board_io.c
 *
 * @brief Driver code for interfacing with UART AND I2C peripherals.
 */

/* Module Header */
#include "board_io.h"

/* System Headers */
#include "main.h"
#include "cmsis_os2.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* User Includes */
#include "system_tasks.h"

/* I2C includes*/
#include "stm32h5xx_hal.h"
#include "app_freertos.h"

#define DEBUG_PORT UART_PORT_4

// Define externs to be used in i2c comms
extern I2C_HandleTypeDef hi2c2;
extern osSemaphoreId_t i2c2_semaphoreHandle;

// Define externs to be used in uart comms
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3; // for windvane
extern UART_HandleTypeDef huart4; // for PC debug
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart7; // for Rpi
extern UART_HandleTypeDef huart8; // for radio

extern osMessageQueueId_t uart_rx_queueHandle;
extern osMutexId_t debugPrintStringMutexHandle;
extern osSemaphoreId_t radio_tx_semaphoreHandle;
extern osSemaphoreId_t raspberry_tx_semaphoreHandle;

uint8_t uart1_rx_byte;
uint8_t uart2_rx_byte;
uint8_t uart3_rx_byte;
uint8_t uart4_rx_byte;
uint8_t uart5_rx_byte;
uint8_t uart6_rx_byte;
uint8_t uart7_rx_byte;
uint8_t uart8_rx_byte;

UART_HandleTypeDef *uart_handle_lookup[] = {
    &huart1,
    &huart2,
    &huart3,
    &huart4,
    &huart5,
    &huart6,
    &huart7,
    &huart8,
};

/* ============================================== I2C =======================================================*/

/* -------------------------------------------------------------------------
 * BoardI2C_Write
 * ------------------------------------------------------------------------- */
bool BoardI2C_Write(uint16_t address, uint8_t *data, uint16_t length, uint32_t timeout)
{
    /* Kick off interrupt driven transmit */
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_IT(&hi2c2, address, data, length);
    if (status != HAL_OK)
        return false;

    /* Sleep until interrupt fires and gives the semaphore */
    if (osSemaphoreAcquire(i2c2_semaphoreHandle, timeout) != osOK)
        return false;

    return true;
}

/* -------------------------------------------------------------------------
 * BoardI2C_Read
 * ------------------------------------------------------------------------- */
bool BoardI2C_Read(uint16_t address, uint8_t *buffer, uint16_t length, uint32_t timeout)
{
    /* Kick off interrupt driven receive */
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive_IT(&hi2c2, address, buffer, length);
    if (status != HAL_OK)
        return false;

    /* Sleep until interrupt fires and gives the semaphore */
    if (osSemaphoreAcquire(i2c2_semaphoreHandle, timeout) != osOK)
        return false;

    return true;
}

/* -------------------------------------------------------------------------
 * HAL Callbacks — called from interrupt context when transaction completes
 * ------------------------------------------------------------------------- */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == &hi2c2)
        osSemaphoreRelease(i2c2_semaphoreHandle);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == &hi2c2)
        osSemaphoreRelease(i2c2_semaphoreHandle);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == &hi2c2)
        osSemaphoreRelease(i2c2_semaphoreHandle);
}

/* ================================================ UART ======================================================*/

/**
 * @brief Initializes UART peripherals for interrupt-driven reception.
 */
void BoardUART_Init(void)
{
    /* Start Interrupt Character Reception for all UART ports */

    HAL_UART_Receive_IT(&huart4, &uart4_rx_byte, 1);
    HAL_UART_Receive_IT(&huart3, &uart3_rx_byte, 1);
    HAL_UART_Receive_IT(&huart8, &uart8_rx_byte, 1);
    HAL_UART_Receive_IT(&huart7, &uart7_rx_byte, 1);
}

/**
 * @brief Local function to print a char to the specified UART port.
 * @param ch
 * @param port
 */
static void Print_Char_Local(char ch, UART_Port_t port)
{
    UART_HandleTypeDef *huart = uart_handle_lookup[(int)port];
    HAL_UART_Transmit(huart, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
}

/**
 * @brief Local function to print a string to a specified UART port.
 * @param string
 * @param port
 */
static void Print_String_Local(const char *string, UART_Port_t port)
{
    while (*string != '\0')
    {
        Print_Char_Local(*string, port);
        string++;
    }
}

/**
 * @brief Thread-safe function to print a string to the debug UART port.
 * @param string
 */
void Debug_Print(const char *string)
{
    osMutexAcquire(debugPrintStringMutexHandle, osWaitForever);
    Print_String_Local(string, DEBUG_PORT);
    osMutexRelease(debugPrintStringMutexHandle);
}

bool RPi_Send(const char *string)
{
    if (string == NULL)
        return false;

    uint16_t len = (uint16_t)strlen(string);
    if (len == 0)
        return false;

    // acquire semaphore
    osSemaphoreAcquire(raspberry_tx_semaphoreHandle, osWaitForever);
    // send
    HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&huart7, (uint8_t *)string, len);

    if (status != HAL_OK)
    {
        osSemaphoreRelease(raspberry_tx_semaphoreHandle);
        return false;
    }

    return true;
}

bool Radio_Send(const char *string)
{
    if (string == NULL)
        return false;

    uint16_t len = (uint16_t)strlen(string);
    if (len == 0)
        return false;

    /* acquire semaphore */
    osSemaphoreAcquire(radio_tx_semaphoreHandle, osWaitForever);
    // send
    HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&huart8, (uint8_t *)string, len);

    if (status != HAL_OK)
    {
        osSemaphoreRelease(radio_tx_semaphoreHandle);
        return false;
    }

    return true;
}

/**
 * @brief Receive Complete Callback
 * @param huart Pointer to UART handle
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart4)
    {
        UART_Char_t uart_char;
        uart_char.port = UART_PORT_4;
        uart_char.data = uart4_rx_byte;
        osMessageQueuePut(uart_rx_queueHandle, &uart_char, 0, 0);
        HAL_UART_Receive_IT(huart, &uart4_rx_byte, 1);
        /* Echo byte back to host */
        HAL_UART_Transmit(huart, &uart4_rx_byte, 1, 0);
    }
    else if (huart == &huart3)
    {
        UART_Char_t uart_char;
        uart_char.port = UART_PORT_3;
        uart_char.data = uart3_rx_byte;
        osMessageQueuePut(uart_rx_queueHandle, &uart_char, 0, 0);
        HAL_UART_Receive_IT(huart, &uart3_rx_byte, 1);
    }

    else if (huart == &huart8)
    {
        UART_Char_t uart_char;
        uart_char.port = UART_PORT_8;
        uart_char.data = uart8_rx_byte;
        osMessageQueuePut(uart_rx_queueHandle, &uart_char, 0, 0);
        HAL_UART_Receive_IT(huart, &uart8_rx_byte, 1);
    }

    else if (huart == &huart7)
    {
        UART_Char_t uart_char;
        uart_char.port = UART_PORT_7;
        uart_char.data = uart7_rx_byte;
        osMessageQueuePut(uart_rx_queueHandle, &uart_char, 0, 0);
        HAL_UART_Receive_IT(huart, &uart7_rx_byte, 1);
    }

    else
    {
    };
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart8)
    {
        osSemaphoreRelease(radio_tx_semaphoreHandle);
    }
    else if (huart == &huart7)
    {
        osSemaphoreRelease(raspberry_tx_semaphoreHandle);
    }
}

/**
 * @brief UART Error Callback
 * @param huart
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart4)
    {
        /* Restart Interrupt Character Reception for UART4 */
        HAL_UART_Receive_IT(huart, &uart4_rx_byte, 1);
    }
}