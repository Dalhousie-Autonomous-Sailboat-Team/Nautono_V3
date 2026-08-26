#ifndef TEST_FAKE_BOARD_IO_H
#define TEST_FAKE_BOARD_IO_H

#include <stdbool.h>
#include <stdint.h>

#include "cmsis_os.h"

bool BoardI2C_Write(uint16_t address, uint8_t *data, uint16_t length, uint32_t timeout);
bool BoardI2C_Read(uint16_t address, uint8_t *buffer, uint16_t length, uint32_t timeout);

#endif /* TEST_FAKE_BOARD_IO_H */
