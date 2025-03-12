/** @file debug.h
 *
 * @brief Implement debug functionality.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST.  All rights reserved.
 */

#ifndef DEBUG_H
#define DEBUG_H

#define ENABLE_DEBUG_PRINT_STATEMENTS

#include <stdint.h>
#include <stdbool.h>

#define DEBUG_MESSAGE_SIZE 32
typedef struct {
    char message[DEBUG_MESSAGE_SIZE];
} DebugMessage_t;

void DebugPrintf(const char *format, ...);

#ifdef ENABLE_DEBUG_PRINT_STATEMENTS
#define DEBUG_PRINT(...) DebugPrintf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif


#endif /* DEBUG_H */

/*** end of file ***/