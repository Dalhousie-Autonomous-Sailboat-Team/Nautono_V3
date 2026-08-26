#include "app_protocol.h"
#include "app_state.h"
#include "cmsis_os.h"
#include "control_logic.h"
#include "test_harness.h"

osMutexId_t windMutexHandle = (osMutexId_t)(uintptr_t)1U;
osMutexId_t xbeeMutexHandle = (osMutexId_t)(uintptr_t)2U;
osMutexId_t rpiMutexHandle = (osMutexId_t)(uintptr_t)3U;

static uint32_t fake_tick;

osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
    (void)mutex_id;
    (void)timeout;
    return osOK;
}

osStatus_t osMutexRelease(osMutexId_t mutex_id)
{
    (void)mutex_id;
    return osOK;
}

uint32_t osKernelGetTickCount(void)
{
    return fake_tick;
}

static void parsed_xbee_command_reaches_actuator_decision(void)
{
    XbeeCommand_t parsed = {0};
    XbeeCommand_t stored = {0};
    ControlLogicOutput_t output = {0};

    TEST_EXPECT_TRUE(AppProtocol_ParseXbeeCommand("{sa:30,ra:-15}\n", &parsed));
    fake_tick = 1000U;
    Xbee_UpdateLatest(&parsed);
    Xbee_GetLatest(&stored);

    ControlLogicInput_t input = {0};
    input.now_ms = 1100U;
    input.xbee = stored;
    input.encoder.angle = 180U;
    ControlLogic_Evaluate(&input, &output);

    TEST_EXPECT_TRUE(output.xbee_valid);
    TEST_EXPECT_FLOAT_NEAR(30.0f, output.target_sail_angle, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-15.0f, output.target_rudder_angle, 0.0001f);
    TEST_EXPECT_INT(12800U, output.sail_motor_channel_1_pwm);
    TEST_EXPECT_INT(0U, output.sail_motor_channel_2_pwm);
    TEST_EXPECT_INT(1236U, output.rudder_pwm);
}

static void expired_xbee_command_falls_back_to_parsed_rpi_sample(void)
{
    const char *rpi_packet =
        "{\"TargetsOutput\":[{"
        "\"targetBearing\":90,\"waypointLat\":44,\"waypointLon\":-63,"
        "\"targetSailAngle\":-30,\"targetFlapAngle\":0,\"targetRudderAngle\":10,"
        "\"latitude\":44,\"longitude\":-63,\"headingAngle\":180,\"windAngle\":45"
        "}]}";
    RPiSample_t parsed = {0};
    RPiSample_t stored_rpi = {0};
    XbeeCommand_t stored_xbee = {0};
    ControlLogicOutput_t output = {0};

    TEST_EXPECT_TRUE(AppProtocol_ParseRpiSample(rpi_packet, &parsed));
    fake_tick = 2000U;
    RPi_UpdateLatest(&parsed);
    RPi_GetLatest(&stored_rpi);
    Xbee_GetLatest(&stored_xbee);

    ControlLogicInput_t input = {0};
    input.now_ms = 4000U;
    input.xbee = stored_xbee;
    input.rpi = stored_rpi;
    input.encoder.angle = 180U;
    ControlLogic_Evaluate(&input, &output);

    TEST_EXPECT_FALSE(output.xbee_valid);
    TEST_EXPECT_TRUE(output.rpi_valid);
    TEST_EXPECT_FLOAT_NEAR(-30.0f, output.target_sail_angle, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(10.0f, output.target_rudder_angle, 0.0001f);
    TEST_EXPECT_INT(0U, output.sail_motor_channel_1_pwm);
    TEST_EXPECT_INT(12800U, output.sail_motor_channel_2_pwm);
}

int main(void)
{
    TestHarness_Run("parsed XBee command reaches actuator decision",
                    parsed_xbee_command_reaches_actuator_decision);
    TestHarness_Run("expired XBee falls back to parsed RPi sample",
                    expired_xbee_command_falls_back_to_parsed_rpi_sample);
    return TestHarness_Finish();
}
