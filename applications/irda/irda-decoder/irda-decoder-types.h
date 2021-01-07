#pragma once
#include <stdint.h>

typedef enum { IRDA_UNKNOWN, IRDA_NEC, IRDA_SAMSUNG } IrDAProtocolType;
typedef enum { IRDA_REPEAT = (1 << 0), IRDA_TOO_SHORT_BUFFER = (1 << 1) } IrDAProtocolFlags;

typedef struct {
    IrDAProtocolType protocol;
    uint8_t flags;
    uint8_t* data; /** < ponter to output data, filled by app */
    uint32_t data_length; /** < output data length, filled by app */
} IrDADecoderOutputData;