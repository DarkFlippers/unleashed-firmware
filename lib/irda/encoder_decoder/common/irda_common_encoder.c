#include "furi/check.h"
#include "irda.h"
#include "irda_common_i.h"
#include <stdbool.h>
#include <furi.h>
#include "irda_i.h"
#include <stdint.h>

static IrdaStatus irda_common_encode_bits(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level) {
    IrdaStatus status = encoder->protocol->encode(encoder, duration, level);
    furi_assert(status == IrdaStatusOk);
    ++encoder->timings_encoded;
    encoder->timings_sum += *duration;
    if ((encoder->bits_encoded == encoder->bits_to_encode) && *level) {
        status = IrdaStatusDone;
    }

    return status;
}

/*
 *
 * 3:
 *      even_timing = 0
 *      level = 0 ^ 1 = 1
 * 4:
 *      even_timing = 1
 *      level = 1 ^ 1 = 0
 *      ++timing;
 *
 *
 *   0     1     2 | 3  4 |
 * _____-------_____---___
*/
IrdaStatus irda_common_encode_manchester(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level) {
    furi_assert(encoder);
    furi_assert(duration);
    furi_assert(level);

    const IrdaTimings* timings = &encoder->protocol->timings;
    uint8_t index = encoder->bits_encoded / 8;
    uint8_t shift = encoder->bits_encoded % 8;   // LSB first
    bool logic_value = !!(encoder->data[index] & (0x01 << shift));
    bool even_timing = !(encoder->timings_encoded % 2);

    *level = even_timing ^ logic_value;
    *duration = timings->bit1_mark;
    if (even_timing)
        ++encoder->bits_encoded;
    else if (*level && (encoder->bits_encoded + 1 == encoder->bits_to_encode))
        ++encoder->bits_encoded;        /* don't encode last space */

    return IrdaStatusOk;
}

IrdaStatus irda_common_encode_pdwm(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level) {
    furi_assert(encoder);
    furi_assert(duration);
    furi_assert(level);

    const IrdaTimings* timings = &encoder->protocol->timings;
    uint8_t index = encoder->bits_encoded / 8;
    uint8_t shift = encoder->bits_encoded % 8;   // LSB first
    bool logic_value = !!(encoder->data[index] & (0x01 << shift));
    bool pwm = timings->bit1_space == timings->bit0_space;

    if (encoder->timings_encoded % 2) {         /* start encoding from space */
        *duration = logic_value ? timings->bit1_mark : timings->bit0_mark;
        *level = true;
        if (pwm)
            ++encoder->bits_encoded;
    } else {
        *duration = logic_value ? timings->bit1_space : timings->bit0_space;
        *level = false;
        if (!pwm)
            ++encoder->bits_encoded;
    }

    return IrdaStatusOk;
}

IrdaStatus irda_common_encode(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level) {
    furi_assert(encoder);
    furi_assert(duration);
    furi_assert(level);
    IrdaStatus status = IrdaStatusOk;
    const IrdaTimings* timings = &encoder->protocol->timings;

    switch (encoder->state) {
    case IrdaCommonEncoderStateSilence:
        *duration = encoder->protocol->timings.silence_time;
        *level = false;
        status = IrdaStatusOk;
        encoder->state = IrdaCommonEncoderStatePreamble;
        ++encoder->timings_encoded;
        encoder->timings_sum = 0;
        break;
    case IrdaCommonEncoderStatePreamble:
        if (timings->preamble_mark) {
            if (encoder->timings_encoded == 1) {
                *duration = timings->preamble_mark;
                *level = true;
            } else {
                *duration = timings->preamble_space;
                *level = false;
                encoder->state = IrdaCommonEncoderStateEncode;
            }
            ++encoder->timings_encoded;
            encoder->timings_sum += *duration;
            break;
        } else {
            encoder->state = IrdaCommonEncoderStateEncode;
        }
        /* FALLTHROUGH */
    case IrdaCommonEncoderStateEncode:
        status = irda_common_encode_bits(encoder, duration, level);
        if (status == IrdaStatusDone) {
            if (encoder->protocol->encode_repeat) {
                encoder->state = IrdaCommonEncoderStateEncodeRepeat;
            } else {
                encoder->timings_encoded = 0;
                encoder->timings_sum = 0;
                encoder->bits_encoded = 0;
                encoder->switch_detect = 0;
                encoder->state = IrdaCommonEncoderStateSilence;
            }
        }
        break;
    case IrdaCommonEncoderStateEncodeRepeat:
        status = encoder->protocol->encode_repeat(encoder, duration, level);
        break;
    }
    return status;
}

void* irda_common_encoder_alloc(const IrdaCommonProtocolSpec* protocol) {
    furi_assert(protocol);
    if (protocol->decode == irda_common_decode_pdwm) {
        furi_assert((protocol->timings.bit1_mark == protocol->timings.bit0_mark) ^ (protocol->timings.bit1_space == protocol->timings.bit0_space));
    }

    /* protocol->databit_len[0] has to contain biggest value of bits that can be decoded */
    for (int i = 1; i < COUNT_OF(protocol->databit_len); ++i) {
        furi_assert(protocol->databit_len[i] <= protocol->databit_len[0]);
    }

    uint32_t alloc_size = sizeof(IrdaCommonDecoder)
                          + protocol->databit_len[0] / 8
                          + !!(protocol->databit_len[0] % 8);
    IrdaCommonEncoder* encoder = furi_alloc(alloc_size);
    memset(encoder, 0, alloc_size);
    encoder->protocol = protocol;

    return encoder;
}

void irda_common_encoder_free(IrdaCommonEncoder* encoder) {
    furi_assert(encoder);
    free(encoder);
}

void irda_common_encoder_reset(IrdaCommonEncoder* encoder) {
    furi_assert(encoder);
    encoder->timings_encoded = 0;
    encoder->timings_sum = 0;
    encoder->bits_encoded = 0;
    encoder->state = IrdaCommonEncoderStateSilence;
    encoder->switch_detect = 0;

    uint8_t max_databit_len = 0;

    for (int i = 0; i < COUNT_OF(encoder->protocol->databit_len); ++i) {
        max_databit_len = MAX(max_databit_len, encoder->protocol->databit_len[i]);
    }

    uint8_t bytes_to_clear = max_databit_len / 8
        + !!(max_databit_len % 8);
    memset(encoder->data, 0, bytes_to_clear);
}

