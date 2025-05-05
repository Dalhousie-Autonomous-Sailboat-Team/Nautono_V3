/** @file radio_control.c
 *
 * @brief Parsing Radio Control Commands.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

 #include "main.h"
 #include "cmsis_os2.h"
 #include "sensor.h"
 #include "FreeRTOS.h"
 
 #include "debug.h"
 #include "radio_control.h"
 
 #include "jsmn.h"
 
 #define RX_BUFFER_SIZE 64
 uint8_t uart_rx_buf[RX_BUFFER_SIZE];
 char json_str[RX_BUFFER_SIZE];

 volatile radio_control_t latest_radio_command;
 extern osEventFlagsId_t Radio_EventHandle;
 
 volatile uint16_t uart8_rx_len = 0;
 
 extern osEventFlagsId_t UART8_EventHandle;
 extern UART_HandleTypeDef huart8;
 
 void UART8_ClearStateAndStartReceive(void)
 {
   __HAL_UART_CLEAR_OREFLAG(&huart8);
   __HAL_UART_CLEAR_FEFLAG(&huart8);
   __HAL_UART_CLEAR_NEFLAG(&huart8);
   __HAL_UART_CLEAR_PEFLAG(&huart8);
   __HAL_UART_CLEAR_IDLEFLAG(&huart8);
   __HAL_UART_FLUSH_DRREGISTER(&huart8);
 
   HAL_UARTEx_ReceiveToIdle_IT(&huart8, uart_rx_buf, RX_BUFFER_SIZE);
 }
 

 int jsoneq(const char *json, jsmntok_t *tok, const char *key)
 {
   if (tok->type == JSMN_STRING &&
       (int)strlen(key) == tok->end - tok->start &&
       strncmp(json + tok->start, key, tok->end - tok->start) == 0)
   {
     return 0;
   }
   return -1;
 }
 
 
 /**
 * @brief Function implementing the Radio_Control thread.
 * @param argument: Not used
 * @retval None
 */
void Radio_Control(void *argument)
{
    int rudder_val = 0;
    int mast_val = 0;
    int flap_val = 0;
    jsmn_parser parser;
    jsmntok_t tokens[16];  /* Adjust based on expected tokens */

    /* Arm UART reception */
    UART8_ClearStateAndStartReceive();
    if (HAL_UARTEx_ReceiveToIdle_IT(&huart8, uart_rx_buf, RX_BUFFER_SIZE) != HAL_OK)
    {
        DEBUG_PRINT("UART8 receive failed to start\n");
    }
    

    while (true)
    {
        osEventFlagsWait(UART8_EventHandle, RX_FLAG, osFlagsWaitAny, osWaitForever);

        memcpy(json_str, uart_rx_buf, uart8_rx_len);
        json_str[uart8_rx_len] = '\0';  /* Null-terminate safely */

        jsmn_init(&parser);
        int ret = jsmn_parse(&parser, json_str, uart8_rx_len, tokens, 16);

        if (ret < 0)
        {
            /* Skip invalid JSON data */
        }
        else
        {
            for (int i = 1; i < ret; i++)
            {
                if (jsoneq(json_str, &tokens[i], "rudder") == 0)
                {
                    rudder_val = atoi(json_str + tokens[i + 1].start);
                    i++;
                }
                else if (jsoneq(json_str, &tokens[i], "mast") == 0)
                {
                    mast_val = atoi(json_str + tokens[i + 1].start);
                    i++;
                }
                else if (jsoneq(json_str, &tokens[i], "flap") == 0)
                {
                    flap_val = atoi(json_str + tokens[i + 1].start);
                    i++;
                }
            }
            latest_radio_command.rudder = rudder_val;
            latest_radio_command.mast = mast_val;
            latest_radio_command.flap = flap_val;

            osEventFlagsSet(Radio_EventHandle, CMD_FLAG);
            //DEBUG_PRINT("Rudder: %d, Mast: %d, Flap: %d\n", rudder_val, mast_val, flap_val);
        }

        /* Re-arm UART reception */
        UART8_ClearStateAndStartReceive();
        HAL_UARTEx_ReceiveToIdle_IT(&huart8, uart_rx_buf, RX_BUFFER_SIZE);

    }

    UNUSED(argument);
}
