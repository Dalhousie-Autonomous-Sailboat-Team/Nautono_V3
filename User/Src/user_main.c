/** @file user_main.c
 *
 * @brief Init task for user application.
 */

/* Module Headers */
#include "user_main.h"

/* System Headers */
#include "main.h"
#include "cmsis_os2.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* User Headers */
#include "L1_pwm.h"

void InitTask(void *argument)
{
    /* Initialize PWM module */
    PWM_Init();

    /* Delete Init Task */
    osThreadExit();
}