/** @file sensor.h
 *
 * @brief Tasks for polling sensors.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST.  All rights reserved.
 */

#ifndef SENSOR_H
#define SENSOR_H

#define ANGLE_SENSOR_COUNT 4

#define TX_FLAG 0x01
#define RX_FLAG 0x02
#define ERR_FLAG 0x04
#define PWR_RDY_FLAG 0x01
#define MAST_ANGLE_READY_FLAG 0x01
#define RUDDER_ANGLE_READY_FLAG 0x02
#define FLAP1_ANGLE_READY_FLAG 0x04
#define FLAP2_ANGLE_READY_FLAG 0x08

#include <stdint.h>
#include <stdbool.h>

enum sensor_addresses
{
    INA1 = 0x40,
    INA2 = 0x41,
    INA3 = 0x43
};

enum register_addresses
{
    CONFIGURATION = 0x00,
    REG_CH1_SHUNT = 0x01,
    REG_CH1_VOLTAGE = 0x02,
    REG_CH2_SHUNT = 0x03,
    REG_CH2_VOLTAGE = 0x04,
    REG_CH3_SHUNT = 0x05,
    REG_CH3_VOLTAGE = 0x06,
    MASK_ENABLE = 0x0F,
    MANUFACTURER_ID = 0xFE,
    DIE_ID = 0xFF
};

enum conversion_indices
{
    CH1_SHUNT = 0,
    CH1_VOLTAGE = 2,
    CH2_SHUNT = 4,
    CH2_VOLTAGE = 6,
    CH3_SHUNT = 8,
    CH3_VOLTAGE = 10
};

#endif /* SENSOR_H */

/*** end of file ***/