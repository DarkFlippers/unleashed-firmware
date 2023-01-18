#include "infrared_protocol_samsung_i.h"
#include <core/check.h>

InfraredMessage* infrared_decoder_samsung32_check_ready(void* ctx) {
    return infrared_common_decoder_check_ready(ctx);
}

bool infrared_decoder_samsung32_interpret(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;
    uint8_t address1 = decoder->data[0];
    uint8_t address2 = decoder->data[1];
    uint8_t command = decoder->data[2];
    uint8_t command_inverse = decoder->data[3];
    uint8_t inverse_command_inverse = (uint8_t)~command_inverse;

    if((address1 == address2) && (command == inverse_command_inverse)) {
        decoder->message.command = command;
        decoder->message.address = address1;
        decoder->message.protocol = InfraredProtocolSamsung32;
        decoder->message.repeat = false;
        result = true;
    }

    return result;
}

// timings start from Space (delay between message and repeat)
InfraredStatus infrared_decoder_samsung32_decode_repeat(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    float preamble_tolerance = decoder->protocol->timings.preamble_tolerance;
    uint32_t bit_tolerance = decoder->protocol->timings.bit_tolerance;
    InfraredStatus status = InfraredStatusError;

    if(decoder->timings_cnt < 6) return InfraredStatusOk;

    if((decoder->timings[0] > INFRARED_SAMSUNG_REPEAT_PAUSE_MIN) &&
       (decoder->timings[0] < INFRARED_SAMSUNG_REPEAT_PAUSE_MAX) &&
       MATCH_TIMING(decoder->timings[1], INFRARED_SAMSUNG_REPEAT_MARK, preamble_tolerance) &&
       MATCH_TIMING(decoder->timings[2], INFRARED_SAMSUNG_REPEAT_SPACE, preamble_tolerance) &&
       MATCH_TIMING(decoder->timings[3], decoder->protocol->timings.bit1_mark, bit_tolerance) &&
       MATCH_TIMING(decoder->timings[4], decoder->protocol->timings.bit1_space, bit_tolerance) &&
       MATCH_TIMING(decoder->timings[5], decoder->protocol->timings.bit1_mark, bit_tolerance)) {
        status = InfraredStatusReady;
        decoder->timings_cnt = 0;
    } else {
        status = InfraredStatusError;
    }

    return status;
}

void* infrared_decoder_samsung32_alloc(void) {
    return infrared_common_decoder_alloc(&infrared_protocol_samsung32);
}

InfraredMessage* infrared_decoder_samsung32_decode(void* decoder, bool level, uint32_t duration) {
    return infrared_common_decode(decoder, level, duration);
}

void infrared_decoder_samsung32_free(void* decoder) {
    infrared_common_decoder_free(decoder);
}

void infrared_decoder_samsung32_reset(void* decoder) {
    infrared_common_decoder_reset(decoder);
}
