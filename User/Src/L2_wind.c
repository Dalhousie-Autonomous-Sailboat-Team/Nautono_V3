#include "L2_wind.h"
#include "L2_app_types.h"
#include "cmsis_os.h"


extern osMutexId_t windMutexHandle;

static WindSample_t wind_latest = {0};

void Wind_UpdateLatest(const WindSample_t *sample)
{
    osMutexAcquire(windMutexHandle, osWaitForever);
    wind_latest = *sample;
    osMutexRelease(windMutexHandle);
}

void Wind_GetLatest(WindSample_t *sample)
{
    osMutexAcquire(windMutexHandle, osWaitForever);
    *sample = wind_latest;
    osMutexRelease(windMutexHandle);
}