#include "furi/check.h"
#include "irda.h"
#include "common/irda_common_i.h"
#include <stdint.h>
#include "../irda_i.h"
#include "irda_protocol_defs_i.h"
#include <furi.h>


typedef struct {
    IrdaCommonEncoder* common_encoder;
    uint8_t databits;
} IrdaSircEncoder;


void irda_encoder_sirc_reset(void* encoder_ptr, const IrdaMessage* message) {
    furi_assert(encoder_ptr);
    furi_assert(message);

    IrdaCommonEncoder* encoder = encoder_ptr;
    IrdaSircEncoder* encoder_sirc = encoder->context;
    irda_common_encoder_reset(encoder);

    uint32_t* data = (void*) encoder->data;

    if (message->protocol == IrdaProtocolSIRC) {
        encoder_sirc->databits = 12;
        *data = (message->command & 0x7F);
        *data |= (message->address & 0x1F) << 7;
    } else if (message->protocol == IrdaProtocolSIRC15) {
        encoder_sirc->databits = 15;
        *data = (message->command & 0x7F);
        *data |= (message->address & 0xFF) << 7;
    } else if (message->protocol == IrdaProtocolSIRC20) {
        encoder_sirc->databits = 20;
        *data = (message->command & 0x7F);
        *data |= (message->address & 0x1FFF) << 7;
    } else {
        furi_assert(0);
    }
}

IrdaStatus irda_encoder_sirc_encode_repeat(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level) {
    furi_assert(encoder);

    IrdaSircEncoder* encoder_sirc = encoder->context;

    uint32_t timings_in_message = 1 + 2 + encoder_sirc->databits * 2;
    furi_assert(encoder->timings_encoded == timings_in_message);

    furi_assert(encoder->timings_sum < IRDA_SIRC_REPEAT_PERIOD);
    *duration = IRDA_SIRC_REPEAT_PERIOD - encoder->timings_sum;
    *level = false;

    encoder->timings_sum = 0;
    encoder->timings_encoded = 1;
    encoder->bits_encoded = 0;
    encoder->state = IrdaCommonEncoderStatePreamble;

    return IrdaStatusOk;
}

void* irda_encoder_sirc_alloc(void) {
    IrdaCommonEncoder* encoder_common = irda_common_encoder_alloc(&protocol_sirc);
    IrdaSircEncoder* encoder_sirc = furi_alloc(sizeof(IrdaSircEncoder));
    encoder_sirc->common_encoder = encoder_common;
    encoder_common->context = encoder_sirc;
    return encoder_common;
}

void irda_encoder_sirc_free(void* encoder_ptr) {
    IrdaCommonEncoder* encoder = encoder_ptr;
    free(encoder->context);
    irda_common_encoder_free(encoder);
}

IrdaStatus irda_encoder_sirc_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    IrdaCommonEncoder* encoder_common = encoder_ptr;
    IrdaSircEncoder* encoder_sirc = encoder_common->context;

    IrdaStatus status = irda_common_encode(encoder_ptr, duration, level);
    if ((status == IrdaStatusOk) && (encoder_common->bits_encoded == encoder_sirc->databits)) {
        furi_assert(!*level);
        status = IrdaStatusDone;
        encoder_common->state = IrdaCommonEncoderStateEncodeRepeat;
    }
    return status;
}

