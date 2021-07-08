#include "furi/check.h"
#include "irda.h"
#include "irda_common_i.h"
#include <stdbool.h>
#include <furi.h>
#include "irda_i.h"

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
    bool inverse = encoder->protocol->manchester_inverse_level;
    bool even_timing = !(encoder->timings_encoded % 2);

    *level = even_timing ^ logic_value ^ inverse;
    *duration = timings->bit1_mark;
    if (even_timing)        /* start encoding from space */
        ++encoder->bits_encoded;
    ++encoder->timings_encoded;

    bool finish = (encoder->bits_encoded == encoder->protocol->databit_len);
    finish |= (encoder->bits_encoded == (encoder->protocol->databit_len-1)) && *level && !even_timing;
    return finish ? IrdaStatusDone : IrdaStatusOk;
}

IrdaStatus irda_common_encode_pdwm(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level) {
    furi_assert(encoder);
    furi_assert(duration);
    furi_assert(level);

    const IrdaTimings* timings = &encoder->protocol->timings;
    uint8_t index = encoder->bits_encoded / 8;
    uint8_t shift = encoder->bits_encoded % 8;   // LSB first
    bool logic_value = !!(encoder->data[index] & (0x01 << shift));

    // stop bit
    if (encoder->bits_encoded == encoder->protocol->databit_len) {
        *duration = timings->bit1_mark;
        *level = true;
        ++encoder->timings_encoded;
        return IrdaStatusDone;
    }

    if (encoder->timings_encoded % 2) {         /* start encoding from space */
        *duration = logic_value ? timings->bit1_mark : timings->bit0_mark;
        *level = true;
    } else {
        *duration = logic_value ? timings->bit1_space : timings->bit0_space;
        *level = false;
        ++encoder->bits_encoded;
    }

    ++encoder->timings_encoded;
    return IrdaStatusOk;
}

IrdaStatus irda_common_encode(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level) {
    furi_assert(encoder);
    furi_assert(duration);
    furi_assert(level);
    IrdaStatus status = IrdaStatusOk;
    const IrdaTimings* timings = &encoder->protocol->timings;

    switch (encoder->state) {
    case IrdaCommonEncoderStateSpace:
        *duration = encoder->protocol->timings.silence_time;
        *level = false;
        status = IrdaStatusOk;
        encoder->state = IrdaCommonEncoderStatePreamble;
        ++encoder->timings_encoded;
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
            break;
        } else {
            encoder->state = IrdaCommonEncoderStateEncode;
        }
        /* FALLTHROUGH */
    case IrdaCommonEncoderStateEncode:
        status = encoder->protocol->encode(encoder, duration, level);
        if (status == IrdaStatusDone) {
            if (encoder->protocol->encode_repeat) {
                encoder->state = IrdaCommonEncoderStateEncodeRepeat;
            } else {
                encoder->timings_encoded = 0;
                encoder->bits_encoded = 0;
                encoder->switch_detect = 0;
                encoder->state = IrdaCommonEncoderStateSpace;
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

    uint32_t alloc_size = sizeof(IrdaCommonEncoder)
                          + protocol->databit_len / 8
                          + !!(protocol->databit_len % 8);
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
    encoder->bits_encoded = 0;
    encoder->state = IrdaCommonEncoderStateSpace;
    encoder->switch_detect = 0;

    uint8_t bytes_to_clear = encoder->protocol->databit_len / 8
        + !!(encoder->protocol->databit_len % 8);
    memset(encoder->data, 0, bytes_to_clear);
}

void irda_common_encoder_set_context(void* decoder, void* context) {
    IrdaCommonEncoder* common_encoder = decoder;
    common_encoder->context = context;
}

