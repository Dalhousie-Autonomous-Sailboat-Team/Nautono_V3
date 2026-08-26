/** @file user_comms.h
 *
 * @brief Driver code for interfacing with UART and I2C peripherals.
 */

#ifndef BOARD_IO_H
#define BOARD_IO_H

#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os2.h"
#include "stm32h5xx_hal.h"

/* I2C completion semaphore — accessible by callbacks */
extern osSemaphoreId_t i2c2_semaphore;

/**
 * @brief Initialize the I2C driver
 * Creates the semaphore used for interrupt synchronization
 */
void UserI2C_Init(void);

/**
 * @brief Write bytes to an I2C device
 *
 * @param address   7-bit device address shifted left by 1
 * @param data      Pointer to data buffer to send
 * @param length    Number of bytes to send
 * @param timeout   Timeout in milliseconds
 * @return true     If transaction completed successfully
 * @return false    If transaction failed or timed out
 */
bool BoardI2C_Write(uint16_t address, uint8_t *data, uint16_t length, uint32_t timeout);

/**
 * @brief Read bytes from an I2C device
 *
 * @param address   7-bit device address shifted left by 1
 * @param buffer    Pointer to buffer to store received data
 * @param length    Number of bytes to read
 * @param timeout   Timeout in milliseconds
 * @return true     If transaction completed successfully
 * @return false    If transaction failed or timed out
 */
bool BoardI2C_Read(uint16_t address, uint8_t *buffer, uint16_t length, uint32_t timeout);

/* UART peripheral code */

typedef enum
{
    UART_PORT_1,
    UART_PORT_2,
    UART_PORT_3,
    UART_PORT_4,
    UART_PORT_5,
    UART_PORT_6,
    UART_PORT_7,
    UART_PORT_8,
} UART_Port_t;

typedef struct
{
    UART_Port_t port;
    uint8_t data;
} UART_Char_t;

void BoardUART_Init(void);
void Debug_Print(const char *string);

bool Radio_Send(const char *string);
bool RPi_Send(const char *string);

#endif /* BOARD_IO_H */