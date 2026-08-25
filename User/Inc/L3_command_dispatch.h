/** @file command_dispatch.h
 *
 * @brief Router code for dispatching commands.
 */

#ifndef COMMAND_DISPATCH_H
#define COMMAND_DISPATCH_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_COMMAND_ARGUMENTS 4
#define MAX_COMMAND_LEN 32
#define MAX_ARGUMENT_LEN 32

typedef struct
{
    char command[MAX_COMMAND_LEN];
    char arguments[MAX_COMMAND_ARGUMENTS][MAX_ARGUMENT_LEN];
    uint8_t arg_count;
} Command_Message_t;

typedef enum
{
    COMMAND_STATUS_SUCCESS = 0,
    COMMAND_STATUS_UNKNOWN_COMMAND,
    COMMAND_STATUS_INVALID_ARGUMENT,
    COMMAND_STATUS_ERROR
} Command_Status_t;

Command_Status_t Dispatch_Command(Command_Message_t *command_message);

#endif /* COMMAND_DISPATCH_H */

/*** end of file ***/