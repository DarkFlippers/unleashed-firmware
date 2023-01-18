#include "infrared_protocol_rc5_i.h"

#include <stdlib.h>
#include <core/check.h>

typedef struct {
    InfraredCommonDecoder* common_decoder;
    bool toggle;
} InfraredRc5Decoder;

InfraredMessage* infrared_decoder_rc5_check_ready(void* ctx) {
    InfraredRc5Decoder* decoder = ctx;
    return infrared_common_decoder_check_ready(decoder->common_decoder);
}

bool infrared_decoder_rc5_interpret(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;
    uint32_t* data = (void*)&decoder->data[0];
    /* Manchester (inverse):
     *      0->1 : 1
     *      1->0 : 0
     */
    decoder->data[0] = ~decoder->data[0];
    decoder->data[1] = ~decoder->data[1];

    // MSB first
    uint8_t address = reverse((uint8_t)decoder->data[0]) & 0x1F;
    uint8_t command = (reverse((uint8_t)decoder->data[1]) >> 2) & 0x3F;
    bool start_bit1 = *data & 0x01;
    bool start_bit2 = *data & 0x02;
    bool toggle = !!(*data & 0x04);

    if(start_bit1 == 1) {
        InfraredProtocol protocol = start_bit2 ? InfraredProtocolRC5 : InfraredProtocolRC5X;
        InfraredMessage* message = &decoder->message;
        InfraredRc5Decoder* rc5_decoder = decoder->context;
        bool* prev_toggle = &rc5_decoder->toggle;
        if((message->address == address) && (message->command == command) &&
           (message->protocol == protocol)) {
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

void* infrared_decoder_rc5_alloc(void) {
    InfraredRc5Decoder* decoder = malloc(sizeof(InfraredRc5Decoder));
    decoder->toggle = false;
    decoder->common_decoder = infrared_common_decoder_alloc(&infrared_protocol_rc5);
    decoder->common_decoder->context = decoder;
    return decoder;
}

InfraredMessage* infrared_decoder_rc5_decode(void* decoder, bool level, uint32_t duration) {
    InfraredRc5Decoder* decoder_rc5 = decoder;
    return infrared_common_decode(decoder_rc5->common_decoder, level, duration);
}

void infrared_decoder_rc5_free(void* decoder) {
    InfraredRc5Decoder* decoder_rc5 = decoder;
    infrared_common_decoder_free(decoder_rc5->common_decoder);
    free(decoder_rc5);
}

void infrared_decoder_rc5_reset(void* decoder) {
    InfraredRc5Decoder* decoder_rc5 = decoder;
    infrared_common_decoder_reset(decoder_rc5->common_decoder);
}
