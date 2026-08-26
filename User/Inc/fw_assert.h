/** @file assert.h
 *
 * @brief A custom assert macro.
 * Please use this instead of using assert function/macros included in the
 * standard C library or any microcontroller HALs.
 *
 * Following this guide:
 * https://interrupt.memfault.com/blog/asserts-in-embedded-systems
 */

#ifndef ASSERT_H
#define ASSERT_H

void assert(const char *msg);

#define ASSERT(expr, msg) \
    do                    \
    {                     \
        if (!(expr))      \
        {                 \
            assert(msg);  \
        }                 \
    } while (0)

#endif /* ASSERT_H */

/*** end of file ***/