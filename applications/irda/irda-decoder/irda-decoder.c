#include "irda-decoder.h"

IrDADecoder* alloc_decoder(void) {
    IrDADecoder* decoder = malloc(sizeof(IrDADecoder));

    // init decoders
    reset_decoder_nec(&decoder->nec);

    return decoder;
}

void free_decoder(IrDADecoder* decoder) {
    free(decoder);
}

bool process_decoder(
    IrDADecoder* decoder,
    bool start_polarity,
    uint32_t* timings,
    uint32_t timings_length,
    IrDADecoderOutputData* out) {
    bool result = false;

    // zero result
    memset(out->data, 0, out->data_length);
    out->protocol = IRDA_UNKNOWN;
    out->flags = 0;

    // process data
    for(uint32_t timings_index = 0; timings_index < timings_length; timings_index++) {
        if(process_decoder_nec(&decoder->nec, start_polarity, timings[timings_index], out)) {
            out->protocol = IRDA_NEC;
            result = true;
            break;
        }

        start_polarity = !start_polarity;
    }

    return result;
}