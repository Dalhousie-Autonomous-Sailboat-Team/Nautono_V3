#ifndef BOAT_MODE_H
#define BOAT_MODE_H

typedef enum
{
    MODE_AUTONOMOUS, // Rpi controls sail and rudder. Can be overriden by Xbee commands
    MODE_MANUAL, // Xbee controls sail and rudder, Rpi is ignored
    MODE_WIND_FOLLOWING // Windvane controls sail, Rpi is ignored. Can be overriden by Xbee commands
} BoatMode_t;

extern const BoatMode_t boat_mode;

#endif
