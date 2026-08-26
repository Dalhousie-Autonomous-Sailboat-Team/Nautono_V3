#ifndef TEST_FAKE_MAIN_H
#define TEST_FAKE_MAIN_H

#include "stm32h5xx_hal.h"

extern GPIO_TypeDef test_debug_led1_port;

#define DEBUG_LED1_GPIO_Port (&test_debug_led1_port)
#define DEBUG_LED1_Pin 1U

#endif /* TEST_FAKE_MAIN_H */
