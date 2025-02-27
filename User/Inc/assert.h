/** @file assert.h
 *
 * @brief A custom assert macro.
 * Please use this instead of using assert function/macros included in the
 * standard C library or any microcontroller HALs.
 *
 * Following this guide:
 * https://interrupt.memfault.com/blog/asserts-in-embedded-systems
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2025 DalMAST. All rights reserved.
 */

 #ifndef ASSERT_H
 #define ASSERT_H
 
 #define ASSERT(expr, msg) \
     do                        \
     {                         \
         if (!(expr))          \
         {                     \
             assert(msg);  \
         }                     \
     } while (0)
 
 void assert(const char *msg);
 
 #endif /* ASSERT_H */
 
 /*** end of file ***/