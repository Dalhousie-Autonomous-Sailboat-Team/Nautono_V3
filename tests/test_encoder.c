#include "encoder.h"
#include "cmsis_os.h"
#include "test_harness.h"

#include <setjmp.h>

osMutexId_t encoderMutexHandle = (osMutexId_t)(uintptr_t)1U;

static bool write_result = true;
static bool read_result = true;
static uint16_t last_write_address;
static uint8_t last_write_data;
static uint16_t last_write_length;
static uint32_t last_write_timeout;
static unsigned int write_calls;
static uint8_t read_high_byte;
static uint8_t read_low_byte;
static jmp_buf task_escape;
static bool escape_on_delay;

bool BoardI2C_Write(uint16_t address, uint8_t *data, uint16_t length, uint32_t timeout)
{
    last_write_address = address;
    last_write_data = data == NULL ? 0U : data[0];
    last_write_length = length;
    last_write_timeout = timeout;
    write_calls++;
    return write_result;
}

bool BoardI2C_Read(uint16_t address, uint8_t *buffer, uint16_t length, uint32_t timeout)
{
    (void)address;
    (void)timeout;
    if (read_result && buffer != NULL && length >= 2U)
    {
        buffer[0] = read_high_byte;
        buffer[1] = read_low_byte;
    }
    return read_result;
}

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

osStatus_t osDelay(uint32_t ticks)
{
    TEST_EXPECT_INT(100U, ticks);
    if (escape_on_delay)
        longjmp(task_escape, 1);
    return osOK;
}

static void selecting_mux_channel_writes_one_hot_mask(void)
{
    write_result = true;
    write_calls = 0U;

    bool selected = Encoder_SelectMuxChannel(4U);

    TEST_EXPECT_TRUE(selected);
    TEST_EXPECT_INT(1U, write_calls);
    TEST_EXPECT_INT(0xE0U, last_write_address);
    TEST_EXPECT_INT(0x10U, last_write_data);
    TEST_EXPECT_INT(1U, last_write_length);
    TEST_EXPECT_INT(10U, last_write_timeout);
}

static void invalid_mux_channel_disables_all_channels(void)
{
    write_result = true;

    bool selected = Encoder_SelectMuxChannel(8U);

    TEST_EXPECT_TRUE(selected);
    TEST_EXPECT_INT(0x00U, last_write_data);
}

static void mux_selection_propagates_i2c_failure(void)
{
    write_result = false;
    TEST_EXPECT_FALSE(Encoder_SelectMuxChannel(4U));
    write_result = true;
}

static void latest_encoder_sample_round_trips(void)
{
    EncoderSample_t expected = {.channel = 4U, .angle = 270U};
    EncoderSample_t actual = {0};

    Encoder_UpdateLatest(&expected);
    TEST_EXPECT_TRUE(Encoder_GetLatest(&actual));

    TEST_EXPECT_INT(4U, actual.channel);
    TEST_EXPECT_INT(270U, actual.angle);
}

static void null_encoder_arguments_are_rejected(void)
{
    Encoder_UpdateLatest(NULL);
    TEST_EXPECT_FALSE(Encoder_GetLatest(NULL));
}

static void encoder_task_reads_sensor_and_updates_latest_sample(void)
{
    EncoderSample_t actual = {0};
    write_result = true;
    read_result = true;
    read_high_byte = 0x08U;
    read_low_byte = 0x00U;
    write_calls = 0U;
    escape_on_delay = true;

    if (setjmp(task_escape) == 0)
        EncoderTask(NULL);

    escape_on_delay = false;
    TEST_EXPECT_TRUE(Encoder_GetLatest(&actual));
    TEST_EXPECT_INT(4U, actual.channel);
    TEST_EXPECT_INT(180U, actual.angle);
    TEST_EXPECT_INT(2U, write_calls);
    TEST_EXPECT_INT(0x0EU, last_write_data);
}

int main(void)
{
    TestHarness_Run("encoder selects mux channel", selecting_mux_channel_writes_one_hot_mask);
    TestHarness_Run("invalid mux channel disables all channels",
                    invalid_mux_channel_disables_all_channels);
    TestHarness_Run("mux selection propagates I2C failure",
                    mux_selection_propagates_i2c_failure);
    TestHarness_Run("latest encoder sample round-trips", latest_encoder_sample_round_trips);
    TestHarness_Run("null encoder arguments are rejected", null_encoder_arguments_are_rejected);
    TestHarness_Run("encoder task reads sensor and updates state",
                    encoder_task_reads_sensor_and_updates_latest_sample);
    return TestHarness_Finish();
}
