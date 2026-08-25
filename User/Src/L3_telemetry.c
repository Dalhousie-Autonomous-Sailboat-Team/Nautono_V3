/**
 * @file telemetry.c
 * @brief Assembles sensor data and transmits JSON telemetry over XBee.
 */

#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"

#include "L1_user_comms.h"
#include "L2_encoder.h"
#include "L2_app_types.h"
#include "L2_rpi.h"
#include "L3_telemetry.h"


#define TELEMETRY_PERIOD_MS 1000
#define TELEMETRY_BUF_LEN 256

extern osMessageQueueId_t wind_queueHandle;

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
        Radio_Print_String(buf);
        //Debug_Print_String(buf);
        /* ── 4. Fixed period ─────────────────────────────────────── */
        osDelay(TELEMETRY_PERIOD_MS);
    }
}