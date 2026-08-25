#ifndef WIND_H
#define WIND_H
#include "L2_app_types.h"


void Wind_UpdateLatest(const WindSample_t *sample);
void Wind_GetLatest(WindSample_t *sample);


#endif /* WIND_H */