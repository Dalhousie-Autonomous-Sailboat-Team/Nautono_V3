// /**
//  * @file command_dispatch.c
//  *
//  * @brief Command dispatching for user application.
//  */

// /* Module Header */
// #include "L3_command_dispatch.h"

// /* System Headers */
// #include "main.h"
// #include "cmsis_os2.h"

// /* Standard Libraries */
// #include <stdint.h>
// #include <stdbool.h>
// #include <string.h>
// #include <stdlib.h>
// #include <stdio.h>

// /* User Headers */
// #include "user_main.h"
// #include "L1_user_comms.h"
// #include "L1_pwm.h"
// #include "L2_encoder.h"

// // static Command_Status_t Get_Angle_Handler(Command_Message_t *command_message);
// static Command_Status_t Set_PWM_Handler(Command_Message_t *command_message);
// static Command_Status_t Set_Angle_Handler(Command_Message_t *command_message);
// static Command_Status_t Get_Angle_Handler(Command_Message_t *command_message);

// /* Command Entry Structure */
// typedef struct COMMAND_ENTRY
// {
//     char command_string[16];
//     Command_Status_t (*handler_function)(Command_Message_t *command_message);
// } Command_Entry_t;

// /* Command Table */
// Command_Entry_t Command_Table[] = {
//     {"get_angle", Get_Angle_Handler}, // read from any of 4 encoders
//     {"set_pwm", Set_PWM_Handler},     // set motor PWM
//     {"set_angle", Set_Angle_Handler}      // set motor position - closed loop
// };

// #define COMMAND_TABLE_SIZE (sizeof(Command_Table) / sizeof(Command_Entry_t))
// /**
//  * @brief Dispatches a command to the appropriate handler function.
//  *
//  * @param command The command string.
//  * @param arguments The command arguments.
//  * @param arg_count The number of arguments.
//  */
// Command_Status_t Dispatch_Command(Command_Message_t *command_message)
// {
//     for (size_t i = 0; i < COMMAND_TABLE_SIZE; i++)
//     {
//         if (strcmp(command_message->command, Command_Table[i].command_string) == 0)
//         {
//             return Command_Table[i].handler_function(command_message);
//         }
//     }
//     char response[64];
//     snprintf(response, sizeof(response), "Unknown command >%s<\n", command_message->command);
//     Debug_Print_String(response);
//     return COMMAND_STATUS_UNKNOWN_COMMAND;
// }

// static Command_Status_t Get_Angle_Handler(Command_Message_t *command_message)
// {
//     char response[64];
//     if (command_message->arg_count != 1)
//     {
//         Debug_Print_String("Invalid arg count\n");
//         return COMMAND_STATUS_INVALID_ARGUMENT;
//     }
//     if (strcmp(command_message->arguments[0], "mast") == 0)
//     {
//         EncoderSample_t sample;
//         Encoder_GetLatest(&sample);
//         snprintf(response, sizeof(response), "MAST_ANGLE:%u\n", (uint16_t)sample.angle);
//         Debug_Print_String(response);
//     }
//     else
//     {
//         snprintf(response, sizeof(response), "Unknown argument >%s<\n", command_message->arguments[0]);
//         Debug_Print_String(response);
//         return COMMAND_STATUS_INVALID_ARGUMENT;
//     }
//     return COMMAND_STATUS_SUCCESS;
// }

// static Command_Status_t Set_PWM_Handler(Command_Message_t *command_message)
// {
//     char response[64];
//     if (command_message->arg_count != 2)
//     {
//         Debug_Print_String("Invalid arg count\n");
//         return COMMAND_STATUS_INVALID_ARGUMENT;
//     }

//     PWM_Channel_t channel;
//     if (strcmp(command_message->arguments[0], "rudder") == 0)
//     {
//         channel = PWM_CHANNEL_RUDDER_SERVO;
//     }
//     else if (strcmp(command_message->arguments[0], "mast_1") == 0)
//     {
//         channel = PWM_CHANNEL_MAST_1_MOTOR;
//     }
//     else if (strcmp(command_message->arguments[0], "mast_2") == 0)
//     {
//         channel = PWM_CHANNEL_MAST_2_MOTOR;
//     }
// #ifdef USE_FLAP_MOTOR
//     else if (strcmp(command_message->arguments[0], "flap_1") == 0)
//     {
//         channel = PWM_CHANNEL_FLAP_1_MOTOR;
//     }
//     else if (strcmp(command_message->arguments[0], "flap_2") == 0)
//     {
//         channel = PWM_CHANNEL_FLAP_2_MOTOR;
//     }
// #endif
// #ifdef USE_MOTOR_CHANNEL_4
//     else if (strcmp(command_message->arguments[0], "flap2_1") == 0)
//     {
//         channel = PWM_CHANNEL_FLAP2_1_MOTOR;
//     }
//     else if (strcmp(command_message->arguments[0], "flap2_2") == 0)
//     {
//         channel = PWM_CHANNEL_FLAP2_2_MOTOR;
//     }
// #endif
//     else
//     {
//         snprintf(response, sizeof(response), "Unknown channel >%s<\n", command_message->arguments[0]);
//         Debug_Print_String(response);
//         return COMMAND_STATUS_INVALID_ARGUMENT;
//     }

//     /* Parse duty cycle value */
//     char *end;
//     uint16_t value = strtol(command_message->arguments[1], &end, 10);

//     if (*end != '\0')
//     {
//         Debug_Print_String("Invalid duty cycle\n");
//         return COMMAND_STATUS_INVALID_ARGUMENT;
//     }

//     PWM_SetDutyCycle(channel, value);
//     return COMMAND_STATUS_SUCCESS;
// }

// static Command_Status_t Set_Angle_Handler(Command_Message_t *command_message)
// {
//     // Placeholder for future closed-loop position control command handler
    


//     return COMMAND_STATUS_SUCCESS;
// }