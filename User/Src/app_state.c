#include "app_state.h"

#include "cmsis_os.h"

/* =================== WIND VARIABLES =========================*/

extern osMutexId_t windMutexHandle;
static WindSample_t wind_latest = {0};

/* ================== XBEE VARIABLES ==========================*/

extern osMutexId_t xbeeMutexHandle;
static XbeeCommand_t xbee_latest = {0};

/* ================== RPI VARIABLES ==========================*/
extern osMutexId_t rpiMutexHandle;
static RPiSample_t rpi_latest;


/* ========================= WIND STUFF ================================*/

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

/* ========================= XBEE STUFF ================================*/

void Xbee_UpdateLatest(const XbeeCommand_t *cmd)
{
    osMutexAcquire(xbeeMutexHandle, osWaitForever);
    xbee_latest = *cmd;
    xbee_latest.ever_received = true;
    xbee_latest.last_updated_ms = osKernelGetTickCount();
    osMutexRelease(xbeeMutexHandle);
}

void Xbee_GetLatest(XbeeCommand_t *cmd)
{
    osMutexAcquire(xbeeMutexHandle, osWaitForever);
    *cmd = xbee_latest;
    osMutexRelease(xbeeMutexHandle);
}

/* ========================= RPI STUFF ================================*/

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
