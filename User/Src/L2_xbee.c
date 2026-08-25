#include "L2_xbee.h"
#include "L2_app_types.h"
#include "cmsis_os.h"

extern osMutexId_t xbeeMutexHandle;
static XbeeCommand_t xbee_latest = {0};

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