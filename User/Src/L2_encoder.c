#include "L2_encoder.h"
#include "L1_user_comms.h"
#include <stdio.h>

/* AS5600 constants */
#define AS5600_I2C_ADDRESS      (0x36 << 1)
#define AS5600_REG_ANGLE_HIGH   0x0E
#define AS5600_REG_ANGLE_LOW    0x0F
#define AS5600_RAW_MAX          4096.0f
#define AS5600_DEGREES_MAX      360.0f
#define AS5600_I2C_TIMEOUT_MS   100

/* TCA9548A mux constants */
#define TCA9548A_I2C_ADDRESS    (0x70 << 1)  
#define TCA9548A_DISABLE_ALL    0x00           
#define ENCODER_MUX_CHANNEL     4              
#define MUX_TIMEOUT_MS          10
#define ENCODER_TASK_DELAY_MS   100


extern osMutexId_t encoderMutexHandle;
static EncoderSample_t encoder_latest; 

static bool AS5600_ReadAngle(uint16_t *angle_degrees)
{
    if (angle_degrees == NULL)
        return false;

    // address slave and register
    uint8_t reg = AS5600_REG_ANGLE_HIGH;
    if (!UserI2C_Write(AS5600_I2C_ADDRESS, &reg, 1, AS5600_I2C_TIMEOUT_MS))
        return false;

    // read 2 bytes
    uint8_t rx_buf[2] = {0};
    if (!UserI2C_Read(AS5600_I2C_ADDRESS, rx_buf, 2, AS5600_I2C_TIMEOUT_MS))
        return false;

    // bit manipulation
    uint16_t raw = ((uint16_t)(rx_buf[0] & 0x0F) << 8) | rx_buf[1];

    // convert to degrees
    *angle_degrees = ((uint16_t)raw / AS5600_RAW_MAX) * AS5600_DEGREES_MAX;

    return true;
} 

bool Encoder_SelectMuxChannel(uint8_t channel)
{
    uint8_t reg;

    if (channel > 7)
    {
        /* Out of range — disable all channels instead */
        reg = TCA9548A_DISABLE_ALL;
    }
    else
    {
        reg = (uint8_t)(1U << channel);   /* channel 4 → 0b00010000 = 0x10 */
    }

    return UserI2C_Write(TCA9548A_I2C_ADDRESS, &reg, 1, MUX_TIMEOUT_MS);
}

void Encoder_UpdateLatest(const EncoderSample_t *sample)
{
    if (sample == NULL)
        return;

    osMutexAcquire(encoderMutexHandle, osWaitForever);
    encoder_latest = *sample;
    osMutexRelease(encoderMutexHandle);
}

bool Encoder_GetLatest(EncoderSample_t *out)
{
    if (out == NULL)
        return false;

    osMutexAcquire(encoderMutexHandle, osWaitForever);
    *out = encoder_latest;
    osMutexRelease(encoderMutexHandle);

    return true;
}

void EncoderTask(void *argument)
{
    //(void)argument;
    char buf[64];
    EncoderSample_t sample;
    sample.channel = ENCODER_MUX_CHANNEL;

    while (true)
    {
        /* open up channel 4 on MUX */
        if (!Encoder_SelectMuxChannel(ENCODER_MUX_CHANNEL))
        {
            continue;
        }

        /* read data from AS5600 on channel 4*/
        if (!AS5600_ReadAngle(&sample.angle))
        {
            continue;
        }

        /* update struct via Mutex */
        Encoder_UpdateLatest(&sample);
        // snprintf(buf, sizeof(buf), "Encoder angle: %d degrees\r\n", (int)sample.angle);
        // Debug_Print_String(buf);
        //Debug_Print_String("Encoder angle: %d degrees\r\n", (int)sample.angle);
    
        //DebugPrint("Encoder angle: %.u degrees\r\n", sample.angle);
    
        osDelay(ENCODER_TASK_DELAY_MS);
    }
}



 