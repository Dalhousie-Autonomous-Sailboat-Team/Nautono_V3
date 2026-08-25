#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdint.h>
#include <stdbool.h>

// Struct to store $IIMWV NMEA sentence data
typedef struct
{
    float direction; // in degrees
    char reference;  // 'R' for relative, 'T' for true
    float speed;
    char speed_unit;  // 'K' for km/h, 'M' for m/s, 'N' for knots
    char status;      // 'A' for valid, 'V' for invalid
    char checksum[3]; // two hex digits + null terminator
    uint32_t timestamp;

} WindSample_t;

// Struct to store data from Xbee
typedef struct
{
    float sail_angle;
    float rud_angle;
    uint32_t last_updated_ms;
    bool ever_received;

} XbeeCommand_t;

typedef struct
{
    /* Targets (from autopilot) */
    float target_bearing;      /* degrees, East = 0,    [0, 360]      */
    float target_lat;          /* degrees, Equator = 0, [-90, 90]     */
    float target_lon;          /* degrees, Prime = 0,   [-180, 180]   */
    float target_sail_angle;   /* degrees, bow = 0,     [0, 360]      */
    float target_flap_angle;   /* degrees                             */
    float target_rudder_angle; /* degrees, centre = 0,  [-45, 45]     */

    /* Navigation state */
    float current_lat;
    float current_lon;
    float current_bearing; /* degrees, [0, 360]                   */
    float current_wind_angle;
    uint32_t last_updated_ms;
    bool ever_received;

} RPiSample_t;

void Wind_UpdateLatest(const WindSample_t *sample);
void Wind_GetLatest(WindSample_t *sample);

void Xbee_UpdateLatest(const XbeeCommand_t *cmd);
void Xbee_GetLatest(XbeeCommand_t *cmd);

void RPi_UpdateLatest(const RPiSample_t *sample);
void RPi_GetLatest(RPiSample_t *out);

#endif /* APP_STATE_H */ /*** end of file ***/