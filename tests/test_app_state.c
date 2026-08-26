#include "app_state.h"
#include "cmsis_os.h"
#include "test_harness.h"

osMutexId_t windMutexHandle = (osMutexId_t)(uintptr_t)1U;
osMutexId_t xbeeMutexHandle = (osMutexId_t)(uintptr_t)2U;
osMutexId_t rpiMutexHandle = (osMutexId_t)(uintptr_t)3U;

static uint32_t fake_tick;
static unsigned int acquire_calls;
static unsigned int release_calls;

osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
    (void)mutex_id;
    TEST_EXPECT_INT(osWaitForever, timeout);
    acquire_calls++;
    return osOK;
}

osStatus_t osMutexRelease(osMutexId_t mutex_id)
{
    (void)mutex_id;
    release_calls++;
    return osOK;
}

uint32_t osKernelGetTickCount(void)
{
    return fake_tick;
}

static void wind_state_round_trips_latest_sample(void)
{
    WindSample_t expected = {0};
    WindSample_t actual = {0};
    expected.direction = 123.5f;
    expected.reference = 'R';
    expected.speed = 6.25f;
    expected.status = 'A';

    Wind_UpdateLatest(&expected);
    Wind_GetLatest(&actual);

    TEST_EXPECT_FLOAT_NEAR(expected.direction, actual.direction, 0.0001f);
    TEST_EXPECT_INT(expected.reference, actual.reference);
    TEST_EXPECT_FLOAT_NEAR(expected.speed, actual.speed, 0.0001f);
    TEST_EXPECT_INT(expected.status, actual.status);
}

static void xbee_update_stamps_freshness_metadata(void)
{
    XbeeCommand_t input = {0};
    XbeeCommand_t actual = {0};
    input.sail_angle = 20.0f;
    input.rud_angle = -10.0f;
    fake_tick = 1234U;

    Xbee_UpdateLatest(&input);
    Xbee_GetLatest(&actual);

    TEST_EXPECT_FLOAT_NEAR(20.0f, actual.sail_angle, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-10.0f, actual.rud_angle, 0.0001f);
    TEST_EXPECT_TRUE(actual.ever_received);
    TEST_EXPECT_INT(1234U, actual.last_updated_ms);
}

static void rpi_update_stamps_freshness_metadata(void)
{
    RPiSample_t input = {0};
    RPiSample_t actual = {0};
    input.target_sail_angle = 35.0f;
    input.target_rudder_angle = 5.0f;
    fake_tick = 5678U;

    RPi_UpdateLatest(&input);
    RPi_GetLatest(&actual);

    TEST_EXPECT_FLOAT_NEAR(35.0f, actual.target_sail_angle, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(5.0f, actual.target_rudder_angle, 0.0001f);
    TEST_EXPECT_TRUE(actual.ever_received);
    TEST_EXPECT_INT(5678U, actual.last_updated_ms);
}

static void rpi_null_arguments_are_ignored(void)
{
    RPiSample_t unchanged = {0};

    RPi_UpdateLatest(NULL);
    RPi_GetLatest(NULL);
    RPi_GetLatest(&unchanged);

    TEST_EXPECT_TRUE(unchanged.ever_received);
    TEST_EXPECT_INT(5678U, unchanged.last_updated_ms);
}

static void state_access_uses_mutexes(void)
{
    TEST_EXPECT_TRUE(acquire_calls > 0U);
    TEST_EXPECT_INT(acquire_calls, release_calls);
}

int main(void)
{
    TestHarness_Run("wind state round-trips latest sample",
                    wind_state_round_trips_latest_sample);
    TestHarness_Run("XBee update stamps freshness", xbee_update_stamps_freshness_metadata);
    TestHarness_Run("RPi update stamps freshness", rpi_update_stamps_freshness_metadata);
    TestHarness_Run("RPi null arguments are ignored", rpi_null_arguments_are_ignored);
    TestHarness_Run("state access uses mutexes", state_access_uses_mutexes);
    return TestHarness_Finish();
}
