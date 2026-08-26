#ifndef TEST_FAKE_CMSIS_OS2_H
#define TEST_FAKE_CMSIS_OS2_H

#include <stdint.h>

typedef void *osMutexId_t;
typedef void *osSemaphoreId_t;
typedef void *osMessageQueueId_t;
typedef void *osThreadId_t;

typedef enum
{
    osOK = 0,
    osError = -1,
    osErrorTimeout = -2
} osStatus_t;

#define osWaitForever 0xFFFFFFFFU

osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout);
osStatus_t osMutexRelease(osMutexId_t mutex_id);
osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout);
osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id);
osStatus_t osMessageQueuePut(osMessageQueueId_t queue_id,
                             const void *message_ptr,
                             uint8_t message_priority,
                             uint32_t timeout);
uint32_t osKernelGetTickCount(void);
osStatus_t osDelay(uint32_t ticks);
void osThreadExit(void);

#endif /* TEST_FAKE_CMSIS_OS2_H */
