#include "infrared_common_i.h"

#include <stdlib.h>
#include <core/check.h>
#include <core/common_defines.h>

static void infrared_common_decoder_reset_state(InfraredCommonDecoder* decoder);

static inline size_t consume_samples(uint32_t* array, size_t len, size_t shift) {
    furi_assert(len >= shift);
    len -= shift;
    for(size_t i = 0; i < len; ++i) {
        array[i] = array[i + shift];
    }

    return len;
}

static inline void accumulate_lsb(InfraredCommonDecoder* decoder, bool bit) {
    uint16_t index = decoder->databit_cnt / 8;
    uint8_t shift = decoder->databit_cnt % 8; // LSB first

    if(!shift) decoder->data[index] = 0;

    if(bit) {
        decoder->data[index] |= (0x1 << shift); // add 1
    } else {
        (void)decoder->data[index]; // add 0
    }

    ++decoder->databit_cnt;
}

static bool infrared_check_preamble(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;
    bool start_level = (decoder->level + decoder->timings_cnt + 1) % 2;

    if(decoder->timings_cnt == 0) return false;

    // align to start at Mark timing
    if(!start_level) {
        decoder->timings_cnt = consume_samples(decoder->timings, decoder->timings_cnt, 1);
    }

    if(decoder->protocol->timings.preamble_mark == 0) {
        return true;
    }

    while((!result) && (decoder->timings_cnt >= 2)) {
        float preamble_tolerance = decoder->protocol->timings.preamble_tolerance;
        uint16_t preamble_mark = decoder->protocol->timings.preamble_mark;
        uint16_t preamble_space = decoder->protocol->timings.preamble_space;

        if((MATCH_TIMING(decoder->timings[0], preamble_mark, preamble_tolerance)) &&
           (MATCH_TIMING(decoder->timings[1], preamble_space, preamble_tolerance))) {
            result = true;
        }

        decoder->timings_cnt = consume_samples(decoder->timings, decoder->timings_cnt, 2);
    }

    return result;
}

/**
 * decoder->protocol->databit_len[0] contains biggest amount of bits, for this protocol.
 * decoder->protocol->databit_len[1...] contains lesser values, but which can be decoded
 * for some protocol modifications.
 */
static InfraredStatus infrared_common_decode_bits(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    InfraredStatus status = InfraredStatusOk;
    const InfraredTimings* timings = &decoder->protocol->timings;

    while(decoder->timings_cnt && (status == InfraredStatusOk)) {
        bool level = (decoder->level + decoder->timings_cnt + 1) % 2;
        uint32_t timing = decoder->timings[0];

        if(timings->min_split_time && !level) {
            if(timing > timings->min_split_time) {
                /* long low timing - check if we're ready for any of protocol modification */
                for(size_t i = 0; i < COUNT_OF(decoder->protocol->databit_len) &&
                                  decoder->protocol->databit_len[i];
                    ++i) {
                    if(decoder->protocol->databit_len[i] == decoder->databit_cnt) {
                        return InfraredStatusReady;
                    }
                }
            } else if(decoder->protocol->databit_len[0] == decoder->databit_cnt) {
                /* short low timing for longest protocol - this is signal is longer than we expected */
                return InfraredStatusError;
            }
        }

        status = decoder->protocol->decode(decoder, level, timing);
        furi_check(decoder->databit_cnt <= decoder->protocol->databit_len[0]);
        furi_assert(status == InfraredStatusError || status == InfraredStatusOk);
        if(status == InfraredStatusError) {
            break;
        }
        decoder->timings_cnt = consume_samples(decoder->timings, decoder->timings_cnt, 1);

        /* check if largest protocol version can be decoded */
        if(level && (decoder->protocol->databit_len[0] == decoder->databit_cnt) && //-V1051
           !timings->min_split_time) {
            status = InfraredStatusReady;
            break;
        }
    }

    return status;
}

/* Pulse Distance-Width Modulation */
InfraredStatus
    infrared_common_decode_pdwm(InfraredCommonDecoder* decoder, bool level, uint32_t timing) {
    furi_assert(decoder);

    InfraredStatus status = InfraredStatusOk;
    uint32_t bit_tolerance = decoder->protocol->timings.bit_tolerance;
    uint16_t bit1_mark = decoder->protocol->timings.bit1_mark;
    uint16_t bit1_space = decoder->protocol->timings.bit1_space;
    uint16_t bit0_mark = decoder->protocol->timings.bit0_mark;
    uint16_t bit0_space = decoder->protocol->timings.bit0_space;

    bool analyze_timing = level ^ (bit1_mark == bit0_mark);
    uint16_t bit1 = level ? bit1_mark : bit1_space;
    uint16_t bit0 = level ? bit0_mark : bit0_space;
    uint16_t no_info_timing = (bit1_mark == bit0_mark) ? bit1_mark : bit1_space;

    if(analyze_timing) {
        if(MATCH_TIMING(timing, bit1, bit_tolerance)) {
            accumulate_lsb(decoder, 1);
        } else if(MATCH_TIMING(timing, bit0, bit_tolerance)) {
            accumulate_lsb(decoder, 0);
        } else {
            status = InfraredStatusError;
        }
    } else {
        if(!MATCH_TIMING(timing, no_info_timing, bit_tolerance)) {
            status = InfraredStatusError;
        }
    }

    return status;
}

/* level switch detection goes in middle of time-quant */
InfraredStatus
    infrared_common_decode_manchester(InfraredCommonDecoder* decoder, bool level, uint32_t timing) {
    furi_assert(decoder);
    uint32_t bit = decoder->protocol->timings.bit1_mark;
    uint32_t tolerance = decoder->protocol->timings.bit_tolerance;

    bool* switch_detect = &decoder->switch_detect;
    furi_assert((*switch_detect == true) || (*switch_detect == false));

    bool single_timing = MATCH_TIMING(timing, bit, tolerance);
    bool double_timing = MATCH_TIMING(timing, 2 * bit, tolerance);

    if(!single_timing && !double_timing) {
        return InfraredStatusError;
    }

    if(decoder->protocol->manchester_start_from_space && (decoder->databit_cnt == 0)) {
        *switch_detect = 1; /* fake as we were previously in the middle of time-quant */
        accumulate_lsb(decoder, 0);
    }

    if(*switch_detect == 0) {
        if(double_timing) {
            return InfraredStatusError;
        }
        /* only single timing - level switch required in the middle of time-quant */
        *switch_detect = 1;
    } else {
        /* double timing means we're in the middle of time-quant again */
        if(single_timing) *switch_detect = 0;
    }

    if(*switch_detect) {
        if(decoder->protocol->databit_len[0] == decoder->databit_cnt) {
            return InfraredStatusError;
        }
        accumulate_lsb(decoder, level);
    }

    return InfraredStatusOk;
}

InfraredMessage* infrared_common_decoder_check_ready(InfraredCommonDecoder* decoder) {
    InfraredMessage* message = NULL;
    bool found_length = false;

    for(size_t i = 0;
        i < COUNT_OF(decoder->protocol->databit_len) && decoder->protocol->databit_len[i];
        ++i) {
        if(decoder->protocol->databit_len[i] == decoder->databit_cnt) {
            found_length = true;
            break;
        }
    }

    if(found_length && decoder->protocol->interpret(decoder)) {
        decoder->databit_cnt = 0;
        message = &decoder->message;
        if(decoder->protocol->decode_repeat) {
            decoder->state = InfraredCommonDecoderStateProcessRepeat;
        } else {
            decoder->state = InfraredCommonDecoderStateWaitPreamble;
        }
    }

    return message;
}

InfraredMessage*
    infrared_common_decode(InfraredCommonDecoder* decoder, bool level, uint32_t duration) {
    furi_assert(decoder);

    InfraredMessage* message = 0;
    InfraredStatus status = InfraredStatusError;

    if(decoder->level == level) {
        infrared_common_decoder_reset(decoder);
    }
    decoder->level = level; // start with low level (Space timing)

    decoder->timings[decoder->timings_cnt] = duration;
    decoder->timings_cnt++;
    furi_check(decoder->timings_cnt <= sizeof(decoder->timings));

    while(1) {
        switch(decoder->state) {
        case InfraredCommonDecoderStateWaitPreamble:
            if(infrared_check_preamble(decoder)) {
                decoder->state = InfraredCommonDecoderStateDecode;
                decoder->databit_cnt = 0;
                decoder->switch_detect = false;
                continue;
            }
            break;
        case InfraredCommonDecoderStateDecode:
            status = infrared_common_decode_bits(decoder);
            if(status == InfraredStatusReady) {
                message = infrared_common_decoder_check_ready(decoder);
                if(message) {
                    continue;
                } else if(decoder->protocol->databit_len[0] == decoder->databit_cnt) {
                    /* error: can't decode largest protocol - begin decoding from start */
                    decoder->state = InfraredCommonDecoderStateWaitPreamble;
                }
            } else if(status == InfraredStatusError) {
                infrared_common_decoder_reset_state(decoder);
                continue;
            }
            break;
        case InfraredCommonDecoderStateProcessRepeat:
            status = decoder->protocol->decode_repeat(decoder);
            if(status == InfraredStatusError) {
                infrared_common_decoder_reset_state(decoder);
                continue;
            } else if(status == InfraredStatusReady) {
                decoder->message.repeat = true;
                message = &decoder->message;
            }
            break;
        }
        break;
    }

    return message;
}

void* infrared_common_decoder_alloc(const InfraredCommonProtocolSpec* protocol) {
    furi_assert(protocol);

    /* protocol->databit_len[0] has to contain biggest value of bits that can be decoded */
    for(size_t i = 1; i < COUNT_OF(protocol->databit_len); ++i) {
        furi_assert(protocol->databit_len[i] <= protocol->databit_len[0]);
    }

    uint32_t alloc_size = sizeof(InfraredCommonDecoder) + protocol->databit_len[0] / 8 +
                          !!(protocol->databit_len[0] % 8);
    InfraredCommonDecoder* decoder = malloc(alloc_size);
    decoder->protocol = protocol;
    decoder->level = true;
    return decoder;
}

void infrared_common_decoder_free(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);
    free(decoder);
}

void infrared_common_decoder_reset_state(InfraredCommonDecoder* decoder) {
    decoder->state = InfraredCommonDecoderStateWaitPreamble;
    decoder->databit_cnt = 0;
    decoder->switch_detect = false;
    decoder->message.protocol = InfraredProtocolUnknown;
    if(decoder->protocol->timings.preamble_mark == 0) {
        if(decoder->timings_cnt > 0) {
            decoder->timings_cnt = consume_samples(decoder->timings, decoder->timings_cnt, 1);
        }
    }
}

void infrared_common_decoder_reset(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    infrared_common_decoder_reset_state(decoder);
    decoder->timings_cnt = 0;
}
