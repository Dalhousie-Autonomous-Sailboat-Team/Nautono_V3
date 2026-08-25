/**
 * @file serial_parser.c
 *
 * @brief Router code for parsing serial data.
 */

/* Module Header */
#include "L2_serial_parser.h"

/* System Headers */
#include "main.h"
#include "cmsis_os2.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* User Includes */
#include "L1_user_comms.h"
#include "L3_command_dispatch.h"
#include "L2_app_types.h"
#include "L2_conversions.h"
#include "L2_rpi.h"
#include "L2_wind.h"
#include "L2_xbee.h"

extern osMessageQueueId_t uart_rx_queueHandle;

#define BACKSPACE_CHAR '\177'
#define NULL_CHAR '\0'
#define CARRIAGE_RETURN_CHAR '\r'

#define PARSE_STORAGE_LEN 32
#define NMEA_IIMWV "$IIMWV"
#define NMEA_IIMWV_LEN 6

static bool WindVane_Parse_NMEA_Sentence(const char *sentence, WindSample_t *sample)
{

    /* --- Parse fields --- */
    char buf[64];
    strncpy(buf, sentence, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *saveptr;
    char *token;

    // Field 0: "$IIMWV" — skip
    token = strtok_r(buf, ",", &saveptr);
    if (token == NULL || strncmp(token, "$IIMWV", 6) != 0)
        return false;

    // Field 1: Wind direction
    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->direction = Conversions_StringToFloat(token);

    // Field 2: Reference
    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->reference = token[0];

    // Field 3: Wind speed
    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->speed = Conversions_StringToFloat(token);

    // Field 4: Speed unit
    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->speed_unit = token[0];

    // Field 5: Status
    token = strtok_r(NULL, "*", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->status = token[0];

    /* --- Timestamp --- */
    // sample->timestamp = xTaskGetTickCount();

    return true;
}

static const char *JSON_FindValue(const char *packet, const char *key)
{
    if (packet == NULL || key == NULL)
        return NULL;

    const char *pos = packet;

    while ((pos = strstr(pos, key)) != NULL)
    {
        /* Walk back over whitespace to find the real preceding character */
        if (pos > packet)
        {
            const char *prev = pos - 1;
            while (prev > packet && *prev == ' ')
                prev--;

            char c = *prev;
            if (c != '"' && c != '{' && c != ',')
            {
                pos++;
                continue; /* Substring match inside a longer key, skip it */
            }
        }

        /* Valid key match --- advance past it and skip whitespace */
        pos += strlen(key);
        while (*pos == ' ')
            pos++;

        return pos;
    }

    return NULL;
}

static bool XBee_Parse_JSON(const char *packet, XbeeCommand_t *cmd)
{
    if (packet == NULL || cmd == NULL)
        return false;

    // Validate packet starts with '{' and contains '}'
    if (packet[0] != '{')
        return false;

    if (strchr(packet, '}') == NULL)
        return false;

    // Extract sail_angle
    const char *sa_val = JSON_FindValue(packet, "sa:");
    if (sa_val == NULL)
        return false;
    cmd->sail_angle = Conversions_StringToFloat(sa_val);

    // Extract rud_angle
    const char *ra_val = JSON_FindValue(packet, "ra:");
    if (ra_val == NULL)
        return false;
    cmd->rud_angle = Conversions_StringToFloat(ra_val);

    return true;
}

static bool RPi_Parse_JSON(const char *packet, RPiSample_t *rpi)
{
    if (packet == NULL || rpi == NULL)
        return false;

    /* Locate the inner object inside "TargetsOutput":[{ ... }] */
    const char *obj = strchr(packet, '[');
    if (obj == NULL)
        return false;
    obj = strchr(obj, '{');
    if (obj == NULL)
        return false;
    if (strchr(obj, '}') == NULL)
        return false;

    const char *val;

    /* Targets */
    val = JSON_FindValue(obj, "targetBearing\":");
    if (val == NULL)
        return false;
    rpi->target_bearing = Conversions_StringToFloat(val);

    val = JSON_FindValue(obj, "waypointLat\":");
    if (val == NULL)
        return false;
    rpi->target_lat = Conversions_StringToFloat(val);

    val = JSON_FindValue(obj, "waypointLon\":");
    if (val == NULL)
        return false;
    rpi->target_lon = Conversions_StringToFloat(val);

    val = JSON_FindValue(obj, "targetSailAngle\":");
    if (val == NULL)
        return false;
    rpi->target_sail_angle = Conversions_StringToFloat(val);

    val = JSON_FindValue(obj, "targetFlapAngle\":");
    if (val == NULL)
        return false;
    rpi->target_flap_angle = Conversions_StringToFloat(val);

    val = JSON_FindValue(obj, "targetRudderAngle\":");
    if (val == NULL)
        return false;
    rpi->target_rudder_angle = Conversions_StringToFloat(val);

    /* Navigation state */
    val = JSON_FindValue(obj, "latitude\":");
    if (val == NULL)
        return false;
    rpi->current_lat = Conversions_StringToFloat(val);

    val = JSON_FindValue(obj, "longitude\":");
    if (val == NULL)
        return false;
    rpi->current_lon = Conversions_StringToFloat(val);

    val = JSON_FindValue(obj, "headingAngle\":");
    if (val == NULL)
        return false;
    rpi->current_bearing = Conversions_StringToFloat(val);

    val = JSON_FindValue(obj, "windAngle\":");
    if (val == NULL)
        return false;
    rpi->current_wind_angle = Conversions_StringToFloat(val);

    return true;
}

static void ProcessWindvaneData(uint8_t data)
{
    /* We only want the $IIMWV NMEA sentence from the windvane */

    static char nmea_sentence[64];
    static uint8_t index = 0;
    static bool collecting = false;
    static uint8_t match_index = 0;

    if (data == '$')
    {
        index = 0;
        match_index = 0;
        collecting = true;
    }

    if (!collecting)
    {
        return;
    }

    // overflow protection
    if (index >= sizeof(nmea_sentence) - 1)
    {
        collecting = false;
        index = 0;
        return;
    }

    nmea_sentence[index++] = data;

    // check character match every ISR, else discard immediately
    if (match_index < NMEA_IIMWV_LEN)
    {
        if (data == NMEA_IIMWV[match_index])
            match_index++;
        else
            collecting = false;
    }

    if (data == '\n')
    {
        collecting = false;
        nmea_sentence[index] = NULL_CHAR;

        if (match_index == NMEA_IIMWV_LEN)
        {
            // We have a full $IIMWV sentence, send to windvane parser
            WindSample_t sample;
            if (WindVane_Parse_NMEA_Sentence(nmea_sentence, &sample))
            {
                // osMessageQueuePut(wind_queueHandle, &sample, 0, 0);
                Wind_UpdateLatest(&sample);
                // Debug_Print_String("Parsed windvane data\r\n");
            }
        }

        index = 0;
    }
}

static void ProcessXbeeData(uint8_t data)
{
    /* Future implementation for UART8 data from Xbee */
    // Debug_Print_String("Received data from Xbee\r\n");

    static char xbee_packet[64];
    static uint8_t index = 0;
    static bool collecting = false;

    // Debug_Print_String("Received data from Xbee\r\n");
    // snprintf(xbee_packet, sizeof(xbee_packet), "Received char: %c\r\n", data);
    // Debug_Print_String((char[]){(char)data, '\0'});

    // Start collecting on '{'
    if (data == '{')
    {
        index = 0;
        collecting = true;
    }

    if (!collecting)
        return;

    // Overflow protection
    if (index >= sizeof(xbee_packet) - 1)
    {
        collecting = false;
        index = 0;
        return;
    }

    xbee_packet[index++] = data;

    // End of packet on '\n'
    if (data == '\n')
    {
        collecting = false;
        xbee_packet[index] = '\0';
        index = 0;

        XbeeCommand_t cmd;
        if (XBee_Parse_JSON(xbee_packet, &cmd))
        {
            Xbee_UpdateLatest(&cmd);
            // Debug_Print_String("Parsed Xbee command\r\n");
        }
    }
}

static void ProcessRaspberryData(uint8_t data)
{
    static char rpi_packet[256]; // also bumped size — your packet is ~120 chars
    static uint8_t index = 0;
    static bool collecting = false;
    static uint8_t brace_depth = 0;

    // Debug_Print_String((char[]){(char) data, '\0'});

    if (data == '{' && !collecting)
    {
        index = 0;
        brace_depth = 0;
        collecting = true;
    }

    if (!collecting)
        return;

    if (index >= sizeof(rpi_packet) - 1)
    {
        collecting = false;
        index = 0;
        brace_depth = 0;
        return;
    }

    rpi_packet[index++] = data;

    if (data == '{')
        brace_depth++;
    if (data == '}')
        brace_depth--;

    if (data == '}' && brace_depth == 0)
    {
        collecting = false;
        rpi_packet[index] = '\0';
        index = 0;

        RPiSample_t RPi_sample;
        Debug_Print_String("Got full string\r\n");

        if (RPi_Parse_JSON(rpi_packet, &RPi_sample))
        {
            RPi_UpdateLatest(&RPi_sample);
            Debug_Print_String("RPi data parsed and stored\r\n");
        }
    }
}

void UARTParserTask(void *argument)

{
    UART_Char_t uart_char;
    User_UART_Init();
    while (true)
    {
        osMessageQueueGet(uart_rx_queueHandle, &uart_char, NULL, osWaitForever);

        switch (uart_char.port)
        {
        case UART_PORT_4: // data from PC debug
            // ProcessDebugData(uart_char.data);
            break;

        case UART_PORT_3: // data from windvane
            ProcessWindvaneData(uart_char.data);
            break;

        case UART_PORT_8: // data from radio
            ProcessXbeeData(uart_char.data);
            break;

        case UART_PORT_7: // data from Raspberry Pi
            ProcessRaspberryData(uart_char.data);
            break;

        default:
            continue;
        }
    }
}