#ifndef TEST_FAKE_CMSIS_GCC_H
#define TEST_FAKE_CMSIS_GCC_H

void Test_FakeBreakpoint(int value);

#define __BKPT(value) Test_FakeBreakpoint(value)

#endif /* TEST_FAKE_CMSIS_GCC_H */
