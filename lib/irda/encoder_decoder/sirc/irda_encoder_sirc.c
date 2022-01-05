#include "furi/check.h"
#include "irda.h"
#include "common/irda_common_i.h"
#include <stdint.h>
#include "../irda_i.h"
#include "irda_protocol_defs_i.h"
#include <furi.h>

void irda_encoder_sirc_reset(void* encoder_ptr, const IrdaMessage* message) {
    furi_assert(encoder_ptr);
    furi_assert(message);

    IrdaCommonEncoder* encoder = encoder_ptr;
    irda_common_encoder_reset(encoder);

    uint32_t* data = (void*)encoder->data;

    if(message->protocol == IrdaProtocolSIRC) {
        *data = (message->command & 0x7F);
        *data |= (message->address & 0x1F) << 7;
        encoder->bits_to_encode = 12;
    } else if(message->protocol == IrdaProtocolSIRC15) {
        *data = (message->command & 0x7F);
        *data |= (message->address & 0xFF) << 7;
        encoder->bits_to_encode = 15;
    } else if(message->protocol == IrdaProtocolSIRC20) {
        *data = (message->command & 0x7F);
        *data |= (message->address & 0x1FFF) << 7;
        encoder->bits_to_encode = 20;
    } else {
        furi_assert(0);
    }
}

IrdaStatus
    irda_encoder_sirc_encode_repeat(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level) {
    furi_assert(encoder);

    furi_assert(encoder->timings_encoded == (1 + 2 + encoder->bits_to_encode * 2 - 1));

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
    return irda_common_encoder_alloc(&protocol_sirc);
}

void irda_encoder_sirc_free(void* encoder_ptr) {
    irda_common_encoder_free(encoder_ptr);
}

IrdaStatus irda_encoder_sirc_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    IrdaCommonEncoder* encoder = encoder_ptr;

    IrdaStatus status = irda_common_encode(encoder, duration, level);
    if((status == IrdaStatusOk) && (encoder->bits_encoded == encoder->bits_to_encode)) {
        furi_assert(!*level);
        status = IrdaStatusDone;
        encoder->state = IrdaCommonEncoderStateEncodeRepeat;
    }
    return status;
}
