/** @file assert.c
 *
 * @brief A custom assert macro.
 * Please use this instead of using assert function/macros included in the
 * standard C library or any microcontroller HALs.
 *
 * Following this guide:
 * https://interrupt.memfault.com/blog/asserts-in-embedded-systems
 */

#include <string.h>
#include <stdint.h>
#include "cmsis_gcc.h"
#include "FreeRTOS.h"

#include "assert.h"

/* The maximum length of the message that the assert caller may store: */
#define LAST_MSG_LEN 64

/* A buffer used for storing the assert caller's message: */
static char last_words[LAST_MSG_LEN] = {0};

/**
 * @brief Custom assert function that stores the assert message and triggers a breakpoint.
 */
void assert(const char *msg)
{
    const size_t msg_len = strnlen(msg, LAST_MSG_LEN - 1);
    memset(last_words, 0, LAST_MSG_LEN);
    memcpy(last_words, msg, msg_len);

    __BKPT(100);
}