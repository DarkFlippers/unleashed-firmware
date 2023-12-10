#include "infrared_protocol_sirc_i.h"
#include <core/check.h>

void infrared_encoder_sirc_reset(void* encoder_ptr, const InfraredMessage* message) {
    furi_assert(encoder_ptr);
    furi_assert(message);

    InfraredCommonEncoder* encoder = encoder_ptr;
    infrared_common_encoder_reset(encoder);

    uint32_t* data = (void*)encoder->data;

    if(message->protocol == InfraredProtocolSIRC) {
        *data = (message->command & 0x7F);
        *data |= (message->address & 0x1F) << 7;
        encoder->bits_to_encode = 12;
    } else if(message->protocol == InfraredProtocolSIRC15) {
        *data = (message->command & 0x7F);
        *data |= (message->address & 0xFF) << 7;
        encoder->bits_to_encode = 15;
    } else if(message->protocol == InfraredProtocolSIRC20) {
        *data = (message->command & 0x7F);
        *data |= (message->address & 0x1FFF) << 7;
        encoder->bits_to_encode = 20;
    } else {
        furi_crash();
    }
}

InfraredStatus infrared_encoder_sirc_encode_repeat(
    InfraredCommonEncoder* encoder,
    uint32_t* duration,
    bool* level) {
    furi_assert(encoder);

    furi_assert(encoder->timings_encoded == (1u + 2 + encoder->bits_to_encode * 2 - 1));

    furi_assert(encoder->timings_sum < INFRARED_SIRC_REPEAT_PERIOD);
    *duration = INFRARED_SIRC_REPEAT_PERIOD - encoder->timings_sum;
    *level = false;

    encoder->timings_sum = 0;
    encoder->timings_encoded = 1;
    encoder->bits_encoded = 0;
    encoder->state = InfraredCommonEncoderStatePreamble;

    return InfraredStatusOk;
}

void* infrared_encoder_sirc_alloc(void) {
    return infrared_common_encoder_alloc(&infrared_protocol_sirc);
}

void infrared_encoder_sirc_free(void* encoder_ptr) {
    infrared_common_encoder_free(encoder_ptr);
}

InfraredStatus infrared_encoder_sirc_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    InfraredCommonEncoder* encoder = encoder_ptr;

    InfraredStatus status = infrared_common_encode(encoder, duration, level);
    if((status == InfraredStatusOk) && (encoder->bits_encoded == encoder->bits_to_encode)) {
        furi_assert(!*level);
        status = InfraredStatusDone;
        encoder->state = InfraredCommonEncoderStateEncodeRepeat;
    }
    return status;
}
