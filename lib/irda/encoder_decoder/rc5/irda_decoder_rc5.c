#include "irda.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <furi.h>
#include "../irda_i.h"
#include "../irda_protocol_defs_i.h"

typedef struct {
    IrdaCommonDecoder* common_decoder;
    bool toggle;
} IrdaRc5Decoder;

bool irda_decoder_rc5_interpret(IrdaCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;
    uint32_t* data = (void*) &decoder->data[0];
    /* Manchester (inverse):
     *      0->1 : 1
     *      1->0 : 0
     */
    decoder->data[0] = ~decoder->data[0];
    decoder->data[1] = ~decoder->data[1];

    // MSB first
    uint8_t address = reverse((uint8_t) decoder->data[0]) & 0x1F;
    uint8_t command = (reverse((uint8_t) decoder->data[1]) >> 2) & 0x3F;
    bool start_bit1 = *data & 0x01;
    bool start_bit2 = *data & 0x02;
    bool toggle = !!(*data & 0x04);

    if (start_bit1 == 1) {
        IrdaProtocol protocol = start_bit2 ? IrdaProtocolRC5 : IrdaProtocolRC5X;
        IrdaMessage* message = &decoder->message;
        IrdaRc5Decoder *rc5_decoder = decoder->context;
        bool *prev_toggle = &rc5_decoder->toggle;
        if ((message->address == address)
            && (message->command == command)
            && (message->protocol == protocol)) {
            message->repeat = (toggle == *prev_toggle);
        } else {
            message->repeat = false;
        }
        *prev_toggle = toggle;
        message->command = command;
        message->address = address;
        message->protocol = protocol;

        result = true;
    }

    return result;
}

void* irda_decoder_rc5_alloc(void) {
    IrdaRc5Decoder* decoder = furi_alloc(sizeof(IrdaRc5Decoder));
    decoder->toggle = false;
    decoder->common_decoder = irda_common_decoder_alloc(&protocol_rc5);
    irda_common_decoder_set_context(decoder->common_decoder, decoder);
    return decoder;
}

IrdaMessage* irda_decoder_rc5_decode(void* decoder, bool level, uint32_t duration) {
    IrdaRc5Decoder* decoder_rc5 = decoder;
    return irda_common_decode(decoder_rc5->common_decoder, level, duration);
}

void irda_decoder_rc5_free(void* decoder) {
    IrdaRc5Decoder* decoder_rc5 = decoder;
    irda_common_decoder_free(decoder_rc5->common_decoder);
    free(decoder_rc5);
}

void irda_decoder_rc5_reset(void* decoder) {
    IrdaRc5Decoder* decoder_rc5 = decoder;
    irda_common_decoder_reset(decoder_rc5->common_decoder);
}

