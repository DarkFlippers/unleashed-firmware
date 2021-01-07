#pragma once
#include "flipper.h"
#include "flipper_v2.h"
#include "irda-decoder-nec.h"
#include "irda-decoder-types.h"

typedef struct {
    IrDANecDecoder nec;
} IrDADecoder;

IrDADecoder* alloc_decoder(void);
void free_decoder(IrDADecoder* decoder);
bool process_decoder(
    IrDADecoder* decoder,
    bool start_polarity,
    uint32_t* timings,
    uint32_t timings_length,
    IrDADecoderOutputData* out);