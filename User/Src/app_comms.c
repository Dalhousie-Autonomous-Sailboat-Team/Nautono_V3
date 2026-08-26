/**
 * @file app_comms.c
 *
 * @brief Application communication code for handling serial and JSON data parsing.
 */

/* Module Header */
#include "app_comms.h"

/* System Headers */
#include "main.h"
#include "cmsis_os2.h"

/* Standard Includes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

/* User Includes */
#include "board_io.h"
#include "command_dispatch.h"
#include "app_state.h"
#include "app_comms.h"
#include "encoder.h"


#define BACKSPACE_CHAR '\177'
#define NULL_CHAR '\0'
#define CARRIAGE_RETURN_CHAR '\r'

#define PARSE_STORAGE_LEN 32
#define NMEA_IIMWV "$IIMWV"
#define NMEA_IIMWV_LEN 6

#define TELEMETRY_PERIOD_MS 1000
#define TELEMETRY_BUF_LEN 256

extern osMessageQueueId_t uart_rx_queueHandle;

extern osMessageQueueId_t wind_queueHandle;

static char tx_buf[128];

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
    sample->direction = StringToFloat(token);

    // Field 2: Reference
    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->reference = token[0];

    // Field 3: Wind speed
    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->speed = StringToFloat(token);

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
    cmd->sail_angle = StringToFloat(sa_val);

    // Extract rud_angle
    const char *ra_val = JSON_FindValue(packet, "ra:");
    if (ra_val == NULL)
        return false;
    cmd->rud_angle = StringToFloat(ra_val);

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
    rpi->target_bearing = StringToFloat(val);

    val = JSON_FindValue(obj, "waypointLat\":");
    if (val == NULL)
        return false;
    rpi->target_lat = StringToFloat(val);

    val = JSON_FindValue(obj, "waypointLon\":");
    if (val == NULL)
        return false;
    rpi->target_lon = StringToFloat(val);

    val = JSON_FindValue(obj, "targetSailAngle\":");
    if (val == NULL)
        return false;
    rpi->target_sail_angle = StringToFloat(val);

    val = JSON_FindValue(obj, "targetFlapAngle\":");
    if (val == NULL)
        return false;
    rpi->target_flap_angle = StringToFloat(val);

    val = JSON_FindValue(obj, "targetRudderAngle\":");
    if (val == NULL)
        return false;
    rpi->target_rudder_angle = StringToFloat(val);

    /* Navigation state */
    val = JSON_FindValue(obj, "latitude\":");
    if (val == NULL)
        return false;
    rpi->current_lat = StringToFloat(val);

    val = JSON_FindValue(obj, "longitude\":");
    if (val == NULL)
        return false;
    rpi->current_lon = StringToFloat(val);

    val = JSON_FindValue(obj, "headingAngle\":");
    if (val == NULL)
        return false;
    rpi->current_bearing = StringToFloat(val);

    val = JSON_FindValue(obj, "windAngle\":");
    if (val == NULL)
        return false;
    rpi->current_wind_angle = StringToFloat(val);

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
                // Debug_Print("Parsed windvane data\r\n");
            }
        }

        index = 0;
    }
}

static void ProcessXbeeData(uint8_t data)
{
    /* Future implementation for UART8 data from Xbee */
    // Debug_Print("Received data from Xbee\r\n");

    static char xbee_packet[64];
    static uint8_t index = 0;
    static bool collecting = false;

    // Debug_Print("Received data from Xbee\r\n");
    // snprintf(xbee_packet, sizeof(xbee_packet), "Received char: %c\r\n", data);
    // Debug_Print((char[]){(char)data, '\0'});

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
            // Debug_Print("Parsed Xbee command\r\n");
        }
    }
}

static void ProcessRaspberryData(uint8_t data)
{
    static char rpi_packet[256]; // also bumped size — your packet is ~120 chars
    static uint8_t index = 0;
    static bool collecting = false;
    static uint8_t brace_depth = 0;

    // Debug_Print((char[]){(char) data, '\0'});

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
        Debug_Print("Got full string\r\n");

        if (RPi_Parse_JSON(rpi_packet, &RPi_sample))
        {
            RPi_UpdateLatest(&RPi_sample);
            Debug_Print("RPi data parsed and stored\r\n");
        }
    }
}

void UARTParserTask(void *argument)

{
    UART_Char_t uart_char;
    BoardUART_Init();
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

void TelemetryTask(void *argument)
{
    (void)argument;

    /* Local shadow of latest data — updated whenever a queue delivers */
    RPiSample_t rpi = {0};
    EncoderSample_t sail_enc = {0};
    //EncoderSample_t flap_enc = {0};

    while (true)
    {

        // get data
        RPi_GetLatest(&rpi);
        Encoder_GetLatest(&sail_enc);
        
        /* ── 3. Format and transmit ──────────────────────────────── */
        char buf[TELEMETRY_BUF_LEN];
        snprintf(buf, sizeof(buf),
                 "{"
                 "\"tb\":%d,"
                 "\"tlat\":%d,"
                 "\"tlon\":%d,"
                 "\"tsa\":%d,"
                 "\"tfa\":%d,"
                 "\"tra\":%d,"
                 "\"clat\":%d,"
                 "\"clon\":%d,"
                 "\"cb\":%d,"
                 "\"wa\":%d,"
                 "\"sa\":%d,"
                 "}\r\n",
                 (int)rpi.target_bearing,
                 (int)rpi.target_lat,
                 (int)rpi.target_lon,
                 (int)rpi.target_sail_angle,
                 (int)rpi.target_flap_angle,
                 (int)rpi.target_rudder_angle,
                 (int)rpi.current_lat,
                 (int)rpi.current_lon,
                 (int)rpi.current_bearing,
                 (int)rpi.current_wind_angle,
                 (int)sail_enc.angle);
        // UserUART_Transmit(UART_PORT_XBEE, (uint8_t *)buf, strlen(buf));
        Radio_Send(buf);
        //Debug_Print(buf);
        /* ── 4. Fixed period ─────────────────────────────────────── */
        osDelay(TELEMETRY_PERIOD_MS);
    }
}

void RpiTransmitTask(void *argument)
{
    WindSample_t sample = {0};

    while (true)
    {

        Wind_GetLatest(&sample);
        snprintf(tx_buf, sizeof(tx_buf),
                 "{\"SensorInput\":[{\"windAngle\":%d}]}\r\n",
                 (int)sample.direction);

        RPi_Send(tx_buf);
        osDelay(500);

    }
}

/* -------------------------------------------------------------------------
 * StringToFloat
 * ------------------------------------------------------------------------- */
float StringToFloat(const char *str)
{
    if (str == NULL || *str == '\0')
        return 0.0f;

    char *tmpSign = (str[0] == '-') ? "-" : "";
    if (str[0] == '-' || str[0] == '+')
        str++;

    int tmpInt1 = 0;
    while (*str && *str != '.')
    {
        if (*str < '0' || *str > '9') break;
        tmpInt1 = tmpInt1 * 10 + (*str - '0');
        str++;
    }

    int tmpInt2        = 0;
    int decimal_digits = 0;
    if (*str == '.')
    {
        str++;
        while (*str && decimal_digits < 4)
        {
            if (*str < '0' || *str > '9') break;
            tmpInt2 = tmpInt2 * 10 + (*str - '0');
            decimal_digits++;
            str++;
        }
    }

    // Pad tmpInt2 to 4 decimal places to match FloatToString
    while (decimal_digits < 4)
    {
        tmpInt2 *= 10;
        decimal_digits++;
    }

    float result = (float)tmpInt1 + ((float)tmpInt2 / 10000.0f);

    return (tmpSign[0] == '-') ? -result : result;
}

/* -------------------------------------------------------------------------
 * FloatToString
 * ------------------------------------------------------------------------- */

void FloatToString(float value, char *buf)
{

    int rounded = (int)(value < 0 ? value - 0.5f : value + 0.5f);
    sprintf(buf, "%d", rounded);
    
}