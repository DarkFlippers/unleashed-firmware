#include "infrared_protocol_rc6_i.h"

#include <stdlib.h>
#include <core/check.h>

typedef struct {
    InfraredCommonDecoder* common_decoder;
    bool toggle;
} InfraredRc6Decoder;

InfraredMessage* infrared_decoder_rc6_check_ready(void* ctx) {
    InfraredRc6Decoder* decoder_rc6 = ctx;
    return infrared_common_decoder_check_ready(decoder_rc6->common_decoder);
}

bool infrared_decoder_rc6_interpret(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;
    uint32_t* data = (void*)&decoder->data[0];
    // MSB first
    uint8_t address = reverse((uint8_t)(*data >> 5));
    uint8_t command = reverse((uint8_t)(*data >> 13));
    bool start_bit = *data & 0x01;
    bool toggle = !!(*data & 0x10);
    uint8_t mode = (*data >> 1) & 0x7;

    if((start_bit == 1) && (mode == 0)) {
        InfraredMessage* message = &decoder->message;
        InfraredRc6Decoder* rc6_decoder = decoder->context;
        bool* prev_toggle = &rc6_decoder->toggle;
        if((message->address == address) && (message->command == command) &&
           (message->protocol == InfraredProtocolRC6)) {
            message->repeat = (toggle == *prev_toggle);
        } else {
            message->repeat = false;
        }
        *prev_toggle = toggle;
        message->command = command;
        message->address = address;
        message->protocol = InfraredProtocolRC6;
        result = true;
    }

    return result;
}

/*
 * RC6 Uses manchester encoding, but it has twice longer
 * 4-th bit (toggle bit) time quant, so we need to decode
 * it separately and than pass decoding for other bits to
 * common manchester decode function.
 */
InfraredStatus infrared_decoder_rc6_decode_manchester(
    InfraredCommonDecoder* decoder,
    bool level,
    uint32_t timing) {
    // 4th bit lasts 2x times more
    InfraredStatus status = InfraredStatusError;
    uint32_t bit = decoder->protocol->timings.bit1_mark;
    uint32_t tolerance = decoder->protocol->timings.bit_tolerance;

    bool single_timing = MATCH_TIMING(timing, bit, tolerance);
    bool double_timing = MATCH_TIMING(timing, 2 * bit, tolerance);
    bool triple_timing = MATCH_TIMING(timing, 3 * bit, tolerance);

    if(decoder->databit_cnt == 4) {
        furi_assert(decoder->switch_detect == true);

        if(single_timing ^ triple_timing) {
            ++decoder->databit_cnt;
            decoder->data[0] |= (single_timing ? !level : level) << 4;
            status = InfraredStatusOk;
        }
    } else if(decoder->databit_cnt == 5) {
        if(single_timing || triple_timing) {
            if(triple_timing) timing = bit;
            decoder->switch_detect = false;
            status = infrared_common_decode_manchester(decoder, level, timing);
        } else if(double_timing) {
            status = InfraredStatusOk;
        }
    } else {
        status = infrared_common_decode_manchester(decoder, level, timing);
    }

    return status;
}

void* infrared_decoder_rc6_alloc(void) {
    InfraredRc6Decoder* decoder = malloc(sizeof(InfraredRc6Decoder));
    decoder->toggle = false;
    decoder->common_decoder = infrared_common_decoder_alloc(&infrared_protocol_rc6);
    decoder->common_decoder->context = decoder;
    return decoder;
}

InfraredMessage* infrared_decoder_rc6_decode(void* decoder, bool level, uint32_t duration) {
    InfraredRc6Decoder* decoder_rc6 = decoder;
    return infrared_common_decode(decoder_rc6->common_decoder, level, duration);
}

void infrared_decoder_rc6_free(void* decoder) {
    InfraredRc6Decoder* decoder_rc6 = decoder;
    infrared_common_decoder_free(decoder_rc6->common_decoder);
    free(decoder_rc6);
}

void infrared_decoder_rc6_reset(void* decoder) {
    InfraredRc6Decoder* decoder_rc6 = decoder;
    infrared_common_decoder_reset(decoder_rc6->common_decoder);
}
