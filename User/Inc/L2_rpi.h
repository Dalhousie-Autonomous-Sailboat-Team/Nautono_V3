#ifndef RPI_H
#define RPI_H

#include "L2_app_types.h"

void RpiTransmitTask(void *argument);
void RPi_UpdateLatest(const RPiSample_t *sample);
void RPi_GetLatest(RPiSample_t *out);

#endif /* RPI_H */