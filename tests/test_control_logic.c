#include "control_logic.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int assertion_failures = 0;
static unsigned int tests_run = 0;

#define EXPECT_TRUE(actual)                                                                    \
    do                                                                                         \
    {                                                                                          \
        if (!(actual))                                                                         \
        {                                                                                      \
            printf("  line %d: expected true: %s\n", __LINE__, #actual);                      \
            assertion_failures++;                                                              \
        }                                                                                      \
    } while (0)

#define EXPECT_FALSE(actual) EXPECT_TRUE(!(actual))

#define EXPECT_U16(expected, actual)                                                           \
    do                                                                                         \
    {                                                                                          \
        uint16_t expected_value = (uint16_t)(expected);                                        \
        uint16_t actual_value = (uint16_t)(actual);                                            \
        if (expected_value != actual_value)                                                    \
        {                                                                                      \
            printf("  line %d: expected %u, got %u: %s\n",                                   \
                   __LINE__,                                                                   \
                   (unsigned int)expected_value,                                               \
                   (unsigned int)actual_value,                                                 \
                   #actual);                                                                   \
            assertion_failures++;                                                              \
        }                                                                                      \
    } while (0)

#define EXPECT_FLOAT_NEAR(expected, actual, tolerance)                                         \
    do                                                                                         \
    {                                                                                          \
        float expected_value = (float)(expected);                                              \
        float actual_value = (float)(actual);                                                  \
        float difference = expected_value - actual_value;                                      \
        if (difference < 0.0f)                                                                 \
            difference = -difference;                                                         \
        if (difference > (float)(tolerance))                                                   \
        {                                                                                      \
            printf("  line %d: expected %.3f, got %.3f: %s\n",                               \
                   __LINE__,                                                                   \
                   (double)expected_value,                                                     \
                   (double)actual_value,                                                       \
                   #actual);                                                                   \
            assertion_failures++;                                                              \
        }                                                                                      \
    } while (0)

static ControlLogicInput_t default_input(void)
{
    ControlLogicInput_t input = {0};
    input.now_ms = 1000U;
    input.encoder.angle = 180U;
    return input;
}

static ControlLogicOutput_t evaluate(const ControlLogicInput_t *input)
{
    ControlLogicOutput_t output;
    memset(&output, 0xA5, sizeof(output));
    ControlLogic_Evaluate(input, &output);
    return output;
}

static void no_commands_never_drive_sail_motor(void)
{
    const uint16_t encoder_angles[] = {0U, 45U, 159U, 180U, 201U, 270U, 359U};

    for (size_t index = 0U; index < sizeof(encoder_angles) / sizeof(encoder_angles[0]); index++)
    {
        ControlLogicInput_t input = default_input();
        input.encoder.angle = encoder_angles[index];

        ControlLogicOutput_t output = evaluate(&input);

        EXPECT_FALSE(output.xbee_valid);
        EXPECT_FALSE(output.rpi_valid);
        EXPECT_FLOAT_NEAR(0.0f, output.target_sail_angle, 0.001f);
        EXPECT_FLOAT_NEAR(0.0f, output.target_rudder_angle, 0.001f);
        EXPECT_U16(0U, output.sail_motor_channel_1_pwm);
        EXPECT_U16(0U, output.sail_motor_channel_2_pwm);
        EXPECT_U16(1370U, output.rudder_pwm);
    }
}

static void expired_commands_never_drive_sail_motor(void)
{
    ControlLogicInput_t input = default_input();
    input.now_ms = 20000U;
    input.encoder.angle = 0U;
    input.xbee.ever_received = true;
    input.xbee.last_updated_ms = 1000U;
    input.xbee.sail_angle = 30.0f;
    input.rpi.ever_received = true;
    input.rpi.last_updated_ms = 1000U;
    input.rpi.target_sail_angle = 30.0f;

    ControlLogicOutput_t output = evaluate(&input);

    EXPECT_FALSE(output.xbee_valid);
    EXPECT_FALSE(output.rpi_valid);
    EXPECT_U16(0U, output.sail_motor_channel_1_pwm);
    EXPECT_U16(0U, output.sail_motor_channel_2_pwm);
}

static void fresh_positive_xbee_command_drives_channel_one(void)
{
    ControlLogicInput_t input = default_input();
    input.xbee.ever_received = true;
    input.xbee.last_updated_ms = 500U;
    input.xbee.sail_angle = 30.0f;
    input.xbee.rud_angle = 45.0f;

    ControlLogicOutput_t output = evaluate(&input);

    EXPECT_TRUE(output.xbee_valid);
    EXPECT_FALSE(output.rpi_valid);
    EXPECT_FLOAT_NEAR(30.0f, output.target_sail_angle, 0.001f);
    EXPECT_U16(12800U, output.sail_motor_channel_1_pwm);
    EXPECT_U16(0U, output.sail_motor_channel_2_pwm);
    EXPECT_U16(1770U, output.rudder_pwm);
}

static void fresh_negative_xbee_command_drives_channel_two(void)
{
    ControlLogicInput_t input = default_input();
    input.xbee.ever_received = true;
    input.xbee.last_updated_ms = 500U;
    input.xbee.sail_angle = -30.0f;

    ControlLogicOutput_t output = evaluate(&input);

    EXPECT_TRUE(output.xbee_valid);
    EXPECT_U16(0U, output.sail_motor_channel_1_pwm);
    EXPECT_U16(12800U, output.sail_motor_channel_2_pwm);
}

static void xbee_timeout_boundary_falls_back_to_fresh_rpi(void)
{
    ControlLogicInput_t input = default_input();
    input.now_ms = 3000U;
    input.xbee.ever_received = true;
    input.xbee.last_updated_ms = 1000U;
    input.xbee.sail_angle = 30.0f;
    input.rpi.ever_received = true;
    input.rpi.last_updated_ms = 1000U;
    input.rpi.target_sail_angle = -30.0f;
    input.rpi.target_rudder_angle = 10.0f;

    ControlLogicOutput_t output = evaluate(&input);

    EXPECT_FALSE(output.xbee_valid);
    EXPECT_TRUE(output.rpi_valid);
    EXPECT_FLOAT_NEAR(-30.0f, output.target_sail_angle, 0.001f);
    EXPECT_FLOAT_NEAR(10.0f, output.target_rudder_angle, 0.001f);
    EXPECT_U16(0U, output.sail_motor_channel_1_pwm);
    EXPECT_U16(12800U, output.sail_motor_channel_2_pwm);
    EXPECT_U16(1458U, output.rudder_pwm);
}

static void nonzero_xbee_values_override_fresh_rpi_values(void)
{
    ControlLogicInput_t input = default_input();
    input.xbee.ever_received = true;
    input.xbee.last_updated_ms = 900U;
    input.xbee.sail_angle = 25.0f;
    input.xbee.rud_angle = 15.0f;
    input.rpi.ever_received = true;
    input.rpi.last_updated_ms = 900U;
    input.rpi.target_sail_angle = -25.0f;
    input.rpi.target_rudder_angle = -15.0f;

    ControlLogicOutput_t output = evaluate(&input);

    EXPECT_TRUE(output.xbee_valid);
    EXPECT_TRUE(output.rpi_valid);
    EXPECT_FLOAT_NEAR(25.0f, output.target_sail_angle, 0.001f);
    EXPECT_FLOAT_NEAR(15.0f, output.target_rudder_angle, 0.001f);
}

static void zero_xbee_values_defer_to_fresh_rpi_values(void)
{
    ControlLogicInput_t input = default_input();
    input.xbee.ever_received = true;
    input.xbee.last_updated_ms = 900U;
    input.xbee.sail_angle = 0.0f;
    input.xbee.rud_angle = 0.0f;
    input.rpi.ever_received = true;
    input.rpi.last_updated_ms = 900U;
    input.rpi.target_sail_angle = 25.0f;
    input.rpi.target_rudder_angle = -45.0f;

    ControlLogicOutput_t output = evaluate(&input);

    EXPECT_FLOAT_NEAR(25.0f, output.target_sail_angle, 0.001f);
    EXPECT_FLOAT_NEAR(-45.0f, output.target_rudder_angle, 0.001f);
    EXPECT_U16(12800U, output.sail_motor_channel_1_pwm);
    EXPECT_U16(0U, output.sail_motor_channel_2_pwm);
    EXPECT_U16(970U, output.rudder_pwm);
}

static void sail_error_at_dead_band_boundary_stops_motor(void)
{
    ControlLogicInput_t input = default_input();
    input.xbee.ever_received = true;
    input.xbee.last_updated_ms = 900U;
    input.xbee.sail_angle = 20.0f;

    ControlLogicOutput_t output = evaluate(&input);

    EXPECT_U16(0U, output.sail_motor_channel_1_pwm);
    EXPECT_U16(0U, output.sail_motor_channel_2_pwm);
}

static void sail_error_uses_shortest_path_across_encoder_wrap(void)
{
    ControlLogicInput_t input = default_input();
    input.encoder.angle = 10U;
    input.xbee.ever_received = true;
    input.xbee.last_updated_ms = 900U;
    input.xbee.sail_angle = 160.0f;

    ControlLogicOutput_t output = evaluate(&input);

    EXPECT_U16(0U, output.sail_motor_channel_1_pwm);
    EXPECT_U16(12800U, output.sail_motor_channel_2_pwm);
}

static void tick_counter_wrap_preserves_recent_xbee_command(void)
{
    ControlLogicInput_t input = default_input();
    input.now_ms = 100U;
    input.xbee.ever_received = true;
    input.xbee.last_updated_ms = UINT32_MAX - 1000U;

    ControlLogicOutput_t output = evaluate(&input);

    EXPECT_TRUE(output.xbee_valid);
}

static void null_input_produces_zeroed_output(void)
{
    ControlLogicOutput_t output = evaluate(NULL);

    EXPECT_FALSE(output.xbee_valid);
    EXPECT_FALSE(output.rpi_valid);
    EXPECT_U16(0U, output.sail_motor_channel_1_pwm);
    EXPECT_U16(0U, output.sail_motor_channel_2_pwm);
    EXPECT_U16(0U, output.rudder_pwm);
}

typedef void (*TestFunction_t)(void);

static void run_test(const char *name, TestFunction_t test_function)
{
    unsigned int failures_before = assertion_failures;
    tests_run++;
    test_function();

    if (assertion_failures == failures_before)
        printf("PASS %s\n", name);
    else
        printf("FAIL %s\n", name);
}

int main(void)
{
    run_test("no commands never drive sail motor", no_commands_never_drive_sail_motor);
    run_test("expired commands never drive sail motor", expired_commands_never_drive_sail_motor);
    run_test("fresh positive XBee command drives channel one",
             fresh_positive_xbee_command_drives_channel_one);
    run_test("fresh negative XBee command drives channel two",
             fresh_negative_xbee_command_drives_channel_two);
    run_test("XBee timeout boundary falls back to fresh RPi",
             xbee_timeout_boundary_falls_back_to_fresh_rpi);
    run_test("nonzero XBee values override fresh RPi values",
             nonzero_xbee_values_override_fresh_rpi_values);
    run_test("zero XBee values defer to fresh RPi values",
             zero_xbee_values_defer_to_fresh_rpi_values);
    run_test("sail error at dead-band boundary stops motor",
             sail_error_at_dead_band_boundary_stops_motor);
    run_test("sail error uses shortest path across encoder wrap",
             sail_error_uses_shortest_path_across_encoder_wrap);
    run_test("tick counter wrap preserves recent XBee command",
             tick_counter_wrap_preserves_recent_xbee_command);
    run_test("null input produces zeroed output", null_input_produces_zeroed_output);

    printf("%u tests, %u assertion failures\n", tests_run, assertion_failures);
    return assertion_failures == 0U ? EXIT_SUCCESS : EXIT_FAILURE;
}
