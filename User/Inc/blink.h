/** @file blink.h
 *
 * @brief Debug LED blinking module.
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2024 DalMAST.  All rights reserved.
 */

 #ifndef BLINK_H
 #define BLINK_H
 
 #include <stdint.h>
 
 #define MINIMUM_BLINK_PERIOD 1000
 
 void blink_init(void);
 void blink_superloop(void);
 
 #endif /* BLINK_H */
 
 /*** end of file ***/