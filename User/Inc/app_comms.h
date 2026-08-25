#ifndef APP_COMMS_H
#define APP_COMMS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Convert a numeric string to a float
 * Handles optional leading '-', integer and decimal parts.
 * Returns 0.0f if the string is NULL, empty, or non-numeric.
 *
 * @param str   Null-terminated input string e.g. "226.0", "-12.5"
 * @return float Parsed value
 */
float Conversions_StringToFloat(const char *str);

/**
 * @brief Convert a float to a string
 * Writes result into a caller-provided buffer.
 * Does not use sprintf/printf.
 *
 */
void Conversions_FloatToString(float value, char *buf);

void TelemetryTask(void *argument);

void RpiTransmitTask(void *argument);


#endif /* APP_COMMS_H */