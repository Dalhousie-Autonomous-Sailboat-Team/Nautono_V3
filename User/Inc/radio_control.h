/** @file radio_control.h
 *
 * @brief Parsing Radio Control Commands.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST.  All rights reserved.
 */

#ifndef RADIO_CONTROL_H
#define RADIO_CONTROL_H

#define TX_FLAG 0x01
#define RX_FLAG 0x02
#define ERR_FLAG 0x04
#define CMD_FLAG 0x08

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef struct radio_control_t
{
    int rudder;
    int mast;
    int flap;
} radio_control_t;



#endif /* RADIO_CONTROL_H */

/*** end of file ***/