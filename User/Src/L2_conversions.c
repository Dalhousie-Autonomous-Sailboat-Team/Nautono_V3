#include "L2_conversions.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
 * StringToFloat
 * ------------------------------------------------------------------------- */
float Conversions_StringToFloat(const char *str)
{
    if (str == NULL || *str == '\0')
        return 0.0f;

    char *tmpSign = (str[0] == '-') ? "-" : "";
    if (str[0] == '-' || str[0] == '+')
        str++;

    int tmpInt1 = 0;
    while (*str && *str != '.')
    {
        if (*str < '0' || *str > '9') break;
        tmpInt1 = tmpInt1 * 10 + (*str - '0');
        str++;
    }

    int tmpInt2        = 0;
    int decimal_digits = 0;
    if (*str == '.')
    {
        str++;
        while (*str && decimal_digits < 4)
        {
            if (*str < '0' || *str > '9') break;
            tmpInt2 = tmpInt2 * 10 + (*str - '0');
            decimal_digits++;
            str++;
        }
    }

    // Pad tmpInt2 to 4 decimal places to match FloatToString
    while (decimal_digits < 4)
    {
        tmpInt2 *= 10;
        decimal_digits++;
    }

    float result = (float)tmpInt1 + ((float)tmpInt2 / 10000.0f);

    return (tmpSign[0] == '-') ? -result : result;
}

/* -------------------------------------------------------------------------
 * FloatToString
 * ------------------------------------------------------------------------- */

void Conversions_FloatToString(float value, char *buf)
{

    int rounded = (int)(value < 0 ? value - 0.5f : value + 0.5f);
    sprintf(buf, "%d", rounded);
    
}