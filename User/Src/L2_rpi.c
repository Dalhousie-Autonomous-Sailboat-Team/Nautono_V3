#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"

#include "L1_user_comms.h"
#include "L2_rpi.h"
#include "L2_app_types.h"
#include "L2_wind.h"
#include "L3_boat_mode.h"


extern osMutexId_t rpiMutexHandle;

static char tx_buf[128];
static RPiSample_t rpi_latest;

void RpiTransmitTask(void *argument)
{
    WindSample_t sample = {0};

    while (true)
    {

        Wind_GetLatest(&sample);
        snprintf(tx_buf, sizeof(tx_buf),
                 "{\"SensorInput\":[{\"windAngle\":%d}]}\r\n",
                 (int)sample.direction);

        RPi_Print_String(tx_buf);
        osDelay(500);

    }
}

void RPi_UpdateLatest(const RPiSample_t *sample)
{
    if (sample == NULL || rpiMutexHandle == NULL)
        return;

    osMutexAcquire(rpiMutexHandle, osWaitForever);
    rpi_latest = *sample;
    rpi_latest.ever_received = true;
    rpi_latest.last_updated_ms = osKernelGetTickCount();
    osMutexRelease(rpiMutexHandle);
}

void RPi_GetLatest(RPiSample_t *out)
{
    if (out == NULL || rpiMutexHandle == NULL)
        return;

    osMutexAcquire(rpiMutexHandle, osWaitForever);
    *out = rpi_latest;
    osMutexRelease(rpiMutexHandle);
}
