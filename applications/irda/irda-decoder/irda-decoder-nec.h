#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "irda-decoder-types.h"

typedef enum {
    WAIT_PREAMBULA_HIGH,
    WAIT_PREAMBULA_LOW,
    WAIT_RETRY_HIGH,
    WAIT_BIT_HIGH,
    WAIT_BIT_LOW,
    WAIT_BIT_STOP_HIGH,
} IrDANecDecoderState;

typedef struct {
    uint8_t addr2;
    uint8_t addr1;
    uint8_t cmd_inverse;
    uint8_t cmd;
} IrDANecData;

typedef uint32_t IrDANecDataType;

typedef struct {
    union {
        IrDANecData simple;
        IrDANecDataType data;
    } data;
    uint8_t current_data_index;
    IrDANecDecoderState state;
} IrDANecDecoder;

bool process_decoder_nec(
    IrDANecDecoder* decoder,
    bool polarity,
    uint32_t time,
    IrDADecoderOutputData* out);

void reset_decoder_nec(IrDANecDecoder* decoder);