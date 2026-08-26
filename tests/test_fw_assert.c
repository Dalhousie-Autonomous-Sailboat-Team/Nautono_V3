#include "fw_assert.h"
#include "test_harness.h"

static unsigned int breakpoint_calls;
static int last_breakpoint_value;

void Test_FakeBreakpoint(int value)
{
    breakpoint_calls++;
    last_breakpoint_value = value;
}

static void true_assertion_does_not_break(void)
{
    breakpoint_calls = 0U;
    ASSERT(1, "should not fire");
    TEST_EXPECT_INT(0U, breakpoint_calls);
}

static void false_assertion_triggers_expected_breakpoint(void)
{
    breakpoint_calls = 0U;
    ASSERT(0, "expected failure");
    TEST_EXPECT_INT(1U, breakpoint_calls);
    TEST_EXPECT_INT(100, last_breakpoint_value);
}

int main(void)
{
    TestHarness_Run("true assertion does not break", true_assertion_does_not_break);
    TestHarness_Run("false assertion triggers breakpoint",
                    false_assertion_triggers_expected_breakpoint);
    return TestHarness_Finish();
}
