#include "irda.h"
#include "irda_protocol_defs_i.h"
#include <stdbool.h>
#include <stdint.h>
#include <furi.h>
#include "../irda_i.h"


IrdaMessage* irda_decoder_samsung32_check_ready(void* ctx) {
    return irda_common_decoder_check_ready(ctx);
}

bool irda_decoder_samsung32_interpret(IrdaCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;
    uint8_t address1 = decoder->data[0];
    uint8_t address2 = decoder->data[1];
    uint8_t command = decoder->data[2];
    uint8_t command_inverse = decoder->data[3];

    if ((address1 == address2) && (command == (uint8_t) ~command_inverse)) {
        decoder->message.command = command;
        decoder->message.address = address1;
        decoder->message.protocol = IrdaProtocolSamsung32;
        decoder->message.repeat = false;
        result = true;
    }

    return result;
}

// timings start from Space (delay between message and repeat)
IrdaStatus irda_decoder_samsung32_decode_repeat(IrdaCommonDecoder* decoder) {
    furi_assert(decoder);

    float preamble_tolerance = decoder->protocol->timings.preamble_tolerance;
    uint32_t bit_tolerance = decoder->protocol->timings.bit_tolerance;
    IrdaStatus status = IrdaStatusError;

    if (decoder->timings_cnt < 6)
        return IrdaStatusOk;

    if ((decoder->timings[0] > IRDA_SAMSUNG_REPEAT_PAUSE_MIN)
        && (decoder->timings[0] < IRDA_SAMSUNG_REPEAT_PAUSE_MAX)
        && MATCH_TIMING(decoder->timings[1], IRDA_SAMSUNG_REPEAT_MARK, preamble_tolerance)
        && MATCH_TIMING(decoder->timings[2], IRDA_SAMSUNG_REPEAT_SPACE, preamble_tolerance)
        && MATCH_TIMING(decoder->timings[3], decoder->protocol->timings.bit1_mark, bit_tolerance)
        && MATCH_TIMING(decoder->timings[4], decoder->protocol->timings.bit1_space, bit_tolerance)
        && MATCH_TIMING(decoder->timings[5], decoder->protocol->timings.bit1_mark, bit_tolerance)
        ) {
        status = IrdaStatusReady;
        decoder->timings_cnt = 0;
    } else {
        status = IrdaStatusError;
    }

    return status;
}

void* irda_decoder_samsung32_alloc(void) {
    return irda_common_decoder_alloc(&protocol_samsung32);
}

IrdaMessage* irda_decoder_samsung32_decode(void* decoder, bool level, uint32_t duration) {
    return irda_common_decode(decoder, level, duration);
}

void irda_decoder_samsung32_free(void* decoder) {
    irda_common_decoder_free(decoder);
}

void irda_decoder_samsung32_reset(void* decoder) {
    irda_common_decoder_reset(decoder);
}

