#include "app_protocol.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER)
#define strtok_r strtok_s
#endif

bool AppProtocol_ParseWindNmea(const char *sentence, WindSample_t *sample)
{
    if (sentence == NULL || sample == NULL)
        return false;

    char buf[64];
    strncpy(buf, sentence, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *saveptr;
    char *token;

    token = strtok_r(buf, ",", &saveptr);
    if (token == NULL || strncmp(token, "$IIMWV", 6) != 0)
        return false;

    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->direction = StringToFloat(token);

    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->reference = token[0];

    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->speed = StringToFloat(token);

    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->speed_unit = token[0];

    token = strtok_r(NULL, "*", &saveptr);
    if (token == NULL || *token == '\0')
        return false;
    sample->status = token[0];

    return true;
}

static const char *JSON_FindValue(const char *packet, const char *key)
{
    if (packet == NULL || key == NULL)
        return NULL;

    const char *pos = packet;

    while ((pos = strstr(pos, key)) != NULL)
    {
        if (pos > packet)
        {
            const char *prev = pos - 1;
            while (prev > packet && *prev == ' ')
                prev--;

            char c = *prev;
            if (c != '"' && c != '{' && c != ',')
            {
                pos++;
                continue;
            }
        }

        pos += strlen(key);
        while (*pos == ' ')
            pos++;

        return pos;
    }

    return NULL;
}

bool AppProtocol_ParseXbeeCommand(const char *packet, XbeeCommand_t *cmd)
{
    if (packet == NULL || cmd == NULL)
        return false;

    if (packet[0] != '{' || strchr(packet, '}') == NULL)
        return false;

    const char *sa_val = JSON_FindValue(packet, "sa:");
    if (sa_val == NULL)
        return false;
    cmd->sail_angle = StringToFloat(sa_val);

    const char *ra_val = JSON_FindValue(packet, "ra:");
    if (ra_val == NULL)
        return false;
    cmd->rud_angle = StringToFloat(ra_val);

    return true;
}

bool AppProtocol_ParseRpiSample(const char *packet, RPiSample_t *rpi)
{
    if (packet == NULL || rpi == NULL)
        return false;

    const char *obj = strchr(packet, '[');
    if (obj == NULL)
        return false;
    obj = strchr(obj, '{');
    if (obj == NULL || strchr(obj, '}') == NULL)
        return false;

    const char *val;

    val = JSON_FindValue(obj, "targetBearing\":");
    if (val == NULL)
        return false;
    rpi->target_bearing = StringToFloat(val);

    val = JSON_FindValue(obj, "waypointLat\":");
    if (val == NULL)
        return false;
    rpi->target_lat = StringToFloat(val);

    val = JSON_FindValue(obj, "waypointLon\":");
    if (val == NULL)
        return false;
    rpi->target_lon = StringToFloat(val);

    val = JSON_FindValue(obj, "targetSailAngle\":");
    if (val == NULL)
        return false;
    rpi->target_sail_angle = StringToFloat(val);

    val = JSON_FindValue(obj, "targetFlapAngle\":");
    if (val == NULL)
        return false;
    rpi->target_flap_angle = StringToFloat(val);

    val = JSON_FindValue(obj, "targetRudderAngle\":");
    if (val == NULL)
        return false;
    rpi->target_rudder_angle = StringToFloat(val);

    val = JSON_FindValue(obj, "latitude\":");
    if (val == NULL)
        return false;
    rpi->current_lat = StringToFloat(val);

    val = JSON_FindValue(obj, "longitude\":");
    if (val == NULL)
        return false;
    rpi->current_lon = StringToFloat(val);

    val = JSON_FindValue(obj, "headingAngle\":");
    if (val == NULL)
        return false;
    rpi->current_bearing = StringToFloat(val);

    val = JSON_FindValue(obj, "windAngle\":");
    if (val == NULL)
        return false;
    rpi->current_wind_angle = StringToFloat(val);

    return true;
}

float StringToFloat(const char *str)
{
    if (str == NULL || *str == '\0')
        return 0.0f;

    char *tmpSign = (str[0] == '-') ? "-" : "";
    if (str[0] == '-' || str[0] == '+')
        str++;

    int tmpInt1 = 0;
    while (*str && *str != '.')
    {
        if (*str < '0' || *str > '9')
            break;
        tmpInt1 = tmpInt1 * 10 + (*str - '0');
        str++;
    }

    int tmpInt2 = 0;
    int decimal_digits = 0;
    if (*str == '.')
    {
        str++;
        while (*str && decimal_digits < 4)
        {
            if (*str < '0' || *str > '9')
                break;
            tmpInt2 = tmpInt2 * 10 + (*str - '0');
            decimal_digits++;
            str++;
        }
    }

    while (decimal_digits < 4)
    {
        tmpInt2 *= 10;
        decimal_digits++;
    }

    float result = (float)tmpInt1 + ((float)tmpInt2 / 10000.0f);
    return (tmpSign[0] == '-') ? -result : result;
}

void FloatToString(float value, char *buf)
{
    int rounded = (int)(value < 0 ? value - 0.5f : value + 0.5f);
    sprintf(buf, "%d", rounded);
}
