#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>



typedef struct
{
    uint8_t channel;      /* Mux channel the reading came from (4)  */
    uint16_t   angle;        /* Degrees, 0.0 – 360.0                   */
} EncoderSample_t;


/* ---------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------*/
void Encoder_Init(void);
bool Encoder_SelectMuxChannel(uint8_t channel);
void Encoder_UpdateLatest(const EncoderSample_t *sample);
bool Encoder_GetLatest(EncoderSample_t *out);
void EncoderTask(void *argument);


#endif /* ENCODER_H */