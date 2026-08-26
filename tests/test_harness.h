#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned int test_assertion_failures = 0;
static unsigned int test_count = 0;

#define TEST_EXPECT_TRUE(actual)                                                               \
    do                                                                                         \
    {                                                                                          \
        if (!(actual))                                                                         \
        {                                                                                      \
            printf("  line %d: expected true: %s\n", __LINE__, #actual);                      \
            test_assertion_failures++;                                                         \
        }                                                                                      \
    } while (0)

#define TEST_EXPECT_FALSE(actual) TEST_EXPECT_TRUE(!(actual))

#define TEST_EXPECT_INT(expected, actual)                                                      \
    do                                                                                         \
    {                                                                                          \
        long long expected_value = (long long)(expected);                                     \
        long long actual_value = (long long)(actual);                                         \
        if (expected_value != actual_value)                                                    \
        {                                                                                      \
            printf("  line %d: expected %lld, got %lld: %s\n",                               \
                   __LINE__, expected_value, actual_value, #actual);                           \
            test_assertion_failures++;                                                         \
        }                                                                                      \
    } while (0)

#define TEST_EXPECT_PTR(expected, actual)                                                      \
    do                                                                                         \
    {                                                                                          \
        const void *expected_value = (const void *)(expected);                                \
        const void *actual_value = (const void *)(actual);                                    \
        if (expected_value != actual_value)                                                    \
        {                                                                                      \
            printf("  line %d: expected %p, got %p: %s\n",                                  \
                   __LINE__, expected_value, actual_value, #actual);                           \
            test_assertion_failures++;                                                         \
        }                                                                                      \
    } while (0)

#define TEST_EXPECT_STRING(expected, actual)                                                   \
    do                                                                                         \
    {                                                                                          \
        const char *expected_value = (expected);                                               \
        const char *actual_value = (actual);                                                   \
        if (actual_value == NULL || strcmp(expected_value, actual_value) != 0)                 \
        {                                                                                      \
            printf("  line %d: expected \"%s\", got \"%s\": %s\n",                       \
                   __LINE__, expected_value, actual_value == NULL ? "(null)" : actual_value,   \
                   #actual);                                                                   \
            test_assertion_failures++;                                                         \
        }                                                                                      \
    } while (0)

#define TEST_EXPECT_FLOAT_NEAR(expected, actual, tolerance)                                    \
    do                                                                                         \
    {                                                                                          \
        float expected_value = (float)(expected);                                              \
        float actual_value = (float)(actual);                                                  \
        float difference = expected_value - actual_value;                                      \
        if (difference < 0.0f)                                                                 \
            difference = -difference;                                                         \
        if (difference > (float)(tolerance))                                                   \
        {                                                                                      \
            printf("  line %d: expected %.4f, got %.4f: %s\n",                               \
                   __LINE__, (double)expected_value, (double)actual_value, #actual);            \
            test_assertion_failures++;                                                         \
        }                                                                                      \
    } while (0)

typedef void (*TestHarnessFunction_t)(void);

static void TestHarness_Run(const char *name, TestHarnessFunction_t function)
{
    unsigned int failures_before = test_assertion_failures;
    test_count++;
    function();
    printf("%s %s\n", test_assertion_failures == failures_before ? "PASS" : "FAIL", name);
}

static int TestHarness_Finish(void)
{
    printf("%u tests, %u assertion failures\n", test_count, test_assertion_failures);
    return test_assertion_failures == 0U ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif /* TEST_HARNESS_H */
