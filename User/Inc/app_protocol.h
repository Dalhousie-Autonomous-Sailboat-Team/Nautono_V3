#ifndef APP_PROTOCOL_H
#define APP_PROTOCOL_H

#include <stdbool.h>

#include "app_state.h"

bool AppProtocol_ParseWindNmea(const char *sentence, WindSample_t *sample);
bool AppProtocol_ParseXbeeCommand(const char *packet, XbeeCommand_t *cmd);
bool AppProtocol_ParseRpiSample(const char *packet, RPiSample_t *rpi);

float StringToFloat(const char *str);
void FloatToString(float value, char *buf);

#endif /* APP_PROTOCOL_H */
