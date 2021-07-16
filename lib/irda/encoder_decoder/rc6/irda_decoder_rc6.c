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
} IrdaRc6Decoder;

bool irda_decoder_rc6_interpret(IrdaCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;
    uint32_t* data = (void*) &decoder->data[0];
    // MSB first
    uint8_t address = reverse((uint8_t) (*data >> 5));
    uint8_t command = reverse((uint8_t) (*data >> 13));
    bool start_bit = *data & 0x01;
    bool toggle = !!(*data & 0x10);
    uint8_t mode = (*data >> 1) & 0x7;

    if ((start_bit == 1) && (mode == 0)) {
        IrdaMessage* message = &decoder->message;
        IrdaRc6Decoder *rc6_decoder = decoder->context;
        bool *prev_toggle = &rc6_decoder->toggle;
        if ((message->address == address)
            && (message->command == command)
            && (message->protocol == IrdaProtocolRC6)) {
            message->repeat = (toggle == *prev_toggle);
        } else {
            message->repeat = false;
        }
        *prev_toggle = toggle;
        message->command = command;
        message->address = address;
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
IrdaStatus irda_decoder_rc6_decode_manchester(IrdaCommonDecoder* decoder) {
    // 4th bit lasts 2x times more
    IrdaStatus status = IrdaStatusError;
    uint16_t bit = decoder->protocol->timings.bit1_mark;
    uint16_t tolerance = decoder->protocol->timings.bit_tolerance;
    uint16_t timing = decoder->timings[0];

    bool single_timing = MATCH_BIT_TIMING(timing, bit, tolerance);
    bool double_timing = MATCH_BIT_TIMING(timing, 2*bit, tolerance);
    bool triple_timing = MATCH_BIT_TIMING(timing, 3*bit, tolerance);

    if (decoder->databit_cnt == 4) {
        furi_assert(decoder->timings_cnt == 1);
        furi_assert(decoder->switch_detect == true);

        if (single_timing ^ triple_timing) {
            --decoder->timings_cnt;
            ++decoder->databit_cnt;
            decoder->data[0] |= (single_timing ? !decoder->level : decoder->level) << 4;
            status = IrdaStatusOk;
        }
    } else if (decoder->databit_cnt == 5) {
        if (single_timing || triple_timing) {
            if (triple_timing)
                decoder->timings[0] = bit;
            decoder->switch_detect = false;
            status = irda_common_decode_manchester(decoder);
        } else if (double_timing) {
            --decoder->timings_cnt;
            status = IrdaStatusOk;
        }
    } else {
        status = irda_common_decode_manchester(decoder);
    }

    return status;
}

void* irda_decoder_rc6_alloc(void) {
    IrdaRc6Decoder* decoder = furi_alloc(sizeof(IrdaRc6Decoder));
    decoder->toggle = false;
    decoder->common_decoder = irda_common_decoder_alloc(&protocol_rc6);
    irda_common_decoder_set_context(decoder->common_decoder, decoder);
    return decoder;
}

IrdaMessage* irda_decoder_rc6_decode(void* decoder, bool level, uint32_t duration) {
    IrdaRc6Decoder* decoder_rc6 = decoder;
    return irda_common_decode(decoder_rc6->common_decoder, level, duration);
}

void irda_decoder_rc6_free(void* decoder) {
    IrdaRc6Decoder* decoder_rc6 = decoder;
    irda_common_decoder_free(decoder_rc6->common_decoder);
    free(decoder_rc6);
}

void irda_decoder_rc6_reset(void* decoder) {
    IrdaRc6Decoder* decoder_rc6 = decoder;
    irda_common_decoder_reset(decoder_rc6->common_decoder);
}

