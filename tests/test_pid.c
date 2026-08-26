#include "pid.h"
#include "test_harness.h"

static void init_stores_configuration_and_resets_state(void)
{
    PID_t pid;
    PID_Init(&pid, 2.0f, 0.5f, 0.25f, -10.0f, 10.0f, -3.0f, 3.0f);

    TEST_EXPECT_FLOAT_NEAR(2.0f, pid.kp, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(0.5f, pid.ki, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(0.25f, pid.kd, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-10.0f, pid.output_min, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(10.0f, pid.output_max, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-3.0f, pid.integral_min, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(3.0f, pid.integral_max, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(0.0f, pid.integral, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(0.0f, pid.prev_error, 0.0001f);
}

static void proportional_term_tracks_error(void)
{
    PID_t pid;
    PID_Init(&pid, 2.0f, 0.0f, 0.0f, -100.0f, 100.0f, -10.0f, 10.0f);

    float output = PID_Update(&pid, 10.0f, 7.0f, 1.0f);

    TEST_EXPECT_FLOAT_NEAR(6.0f, output, 0.0001f);
}

static void integral_term_accumulates_and_clamps(void)
{
    PID_t pid;
    PID_Init(&pid, 0.0f, 1.0f, 0.0f, -100.0f, 100.0f, -2.0f, 2.0f);

    float first = PID_Update(&pid, 3.0f, 0.0f, 1.0f);
    float second = PID_Update(&pid, 3.0f, 0.0f, 1.0f);

    TEST_EXPECT_FLOAT_NEAR(2.0f, first, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(2.0f, second, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(2.0f, pid.integral, 0.0001f);
}

static void derivative_term_uses_error_delta(void)
{
    PID_t pid;
    PID_Init(&pid, 0.0f, 0.0f, 1.0f, -100.0f, 100.0f, -10.0f, 10.0f);

    float first = PID_Update(&pid, 4.0f, 0.0f, 2.0f);
    float second = PID_Update(&pid, 2.0f, 0.0f, 2.0f);

    TEST_EXPECT_FLOAT_NEAR(2.0f, first, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-1.0f, second, 0.0001f);
}

static void output_is_clamped_to_configured_limits(void)
{
    PID_t pid;
    PID_Init(&pid, 10.0f, 0.0f, 0.0f, -5.0f, 5.0f, -10.0f, 10.0f);

    TEST_EXPECT_FLOAT_NEAR(5.0f, PID_Update(&pid, 10.0f, 0.0f, 1.0f), 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(-5.0f, PID_Update(&pid, -10.0f, 0.0f, 1.0f), 0.0001f);
}

static void reset_clears_accumulated_state(void)
{
    PID_t pid;
    PID_Init(&pid, 1.0f, 1.0f, 1.0f, -100.0f, 100.0f, -10.0f, 10.0f);
    (void)PID_Update(&pid, 5.0f, 0.0f, 1.0f);

    PID_Reset(&pid);

    TEST_EXPECT_FLOAT_NEAR(0.0f, pid.integral, 0.0001f);
    TEST_EXPECT_FLOAT_NEAR(0.0f, pid.prev_error, 0.0001f);
}

int main(void)
{
    TestHarness_Run("PID init stores configuration and resets state",
                    init_stores_configuration_and_resets_state);
    TestHarness_Run("PID proportional term tracks error", proportional_term_tracks_error);
    TestHarness_Run("PID integral term accumulates and clamps",
                    integral_term_accumulates_and_clamps);
    TestHarness_Run("PID derivative term uses error delta", derivative_term_uses_error_delta);
    TestHarness_Run("PID output is clamped", output_is_clamped_to_configured_limits);
    TestHarness_Run("PID reset clears accumulated state", reset_clears_accumulated_state);
    return TestHarness_Finish();
}
