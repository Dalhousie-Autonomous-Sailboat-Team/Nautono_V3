#include "cmsis_os2.h"
#include "main.h"
#include "test_harness.h"

#include <stdbool.h>
#include <setjmp.h>

void InitTask(void *argument);
void HeartbeatTask(void *argument);
void PreSleepProcessing(uint32_t expected_idle_time);
void PostSleepProcessing(uint32_t expected_idle_time);

GPIO_TypeDef test_debug_led1_port = {1};

static unsigned int pwm_init_calls;
static unsigned int thread_exit_calls;
static unsigned int suspend_tick_calls;
static unsigned int resume_tick_calls;
static unsigned int enter_sleep_calls;
static uint32_t sleep_regulator;
static uint8_t sleep_entry;
static GPIO_PinState gpio_states[4];
static uint32_t delay_values[4];
static unsigned int gpio_write_calls;
static unsigned int delay_calls;
static bool escape_heartbeat;
static jmp_buf heartbeat_escape;

void PWM_Init(void)
{
    pwm_init_calls++;
}

void osThreadExit(void)
{
    thread_exit_calls++;
}

osStatus_t osDelay(uint32_t ticks)
{
    if (delay_calls < 4U)
        delay_values[delay_calls] = ticks;
    delay_calls++;
    if (escape_heartbeat && delay_calls == 2U)
        longjmp(heartbeat_escape, 1);
    return osOK;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
    TEST_EXPECT_PTR(DEBUG_LED1_GPIO_Port, port);
    TEST_EXPECT_INT(DEBUG_LED1_Pin, pin);
    if (gpio_write_calls < 4U)
        gpio_states[gpio_write_calls] = state;
    gpio_write_calls++;
}

void HAL_SuspendTick(void)
{
    suspend_tick_calls++;
}

void HAL_ResumeTick(void)
{
    resume_tick_calls++;
}

void HAL_PWR_EnterSLEEPMode(uint32_t regulator, uint8_t entry)
{
    enter_sleep_calls++;
    sleep_regulator = regulator;
    sleep_entry = entry;
}

static void init_task_initializes_pwm_and_exits(void)
{
    pwm_init_calls = 0U;
    thread_exit_calls = 0U;

    InitTask(NULL);

    TEST_EXPECT_INT(1U, pwm_init_calls);
    TEST_EXPECT_INT(1U, thread_exit_calls);
}

static void heartbeat_toggles_led_with_expected_period(void)
{
    gpio_write_calls = 0U;
    delay_calls = 0U;
    escape_heartbeat = true;

    if (setjmp(heartbeat_escape) == 0)
        HeartbeatTask(NULL);

    escape_heartbeat = false;
    TEST_EXPECT_INT(2U, gpio_write_calls);
    TEST_EXPECT_INT(GPIO_PIN_SET, gpio_states[0]);
    TEST_EXPECT_INT(GPIO_PIN_RESET, gpio_states[1]);
    TEST_EXPECT_INT(1U, delay_values[0]);
    TEST_EXPECT_INT(1999U, delay_values[1]);
}

static void pre_sleep_suspends_tick_and_enters_sleep(void)
{
    suspend_tick_calls = 0U;
    enter_sleep_calls = 0U;

    PreSleepProcessing(123U);

    TEST_EXPECT_INT(1U, suspend_tick_calls);
    TEST_EXPECT_INT(1U, enter_sleep_calls);
    TEST_EXPECT_INT(PWR_MAINREGULATOR_ON, sleep_regulator);
    TEST_EXPECT_INT(PWR_SLEEPENTRY_WFI, sleep_entry);
}

static void post_sleep_resumes_tick(void)
{
    resume_tick_calls = 0U;
    PostSleepProcessing(123U);
    TEST_EXPECT_INT(1U, resume_tick_calls);
}

int main(void)
{
    TestHarness_Run("init task initializes PWM and exits", init_task_initializes_pwm_and_exits);
    TestHarness_Run("heartbeat toggles LED", heartbeat_toggles_led_with_expected_period);
    TestHarness_Run("pre-sleep suspends tick", pre_sleep_suspends_tick_and_enters_sleep);
    TestHarness_Run("post-sleep resumes tick", post_sleep_resumes_tick);
    return TestHarness_Finish();
}
