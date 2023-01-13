#include "infrared_protocol_rc6_i.h"

#include <stdlib.h>
#include <core/check.h>

typedef struct InfraredEncoderRC6 {
    InfraredCommonEncoder* common_encoder;
    bool toggle_bit;
} InfraredEncoderRC6;

void infrared_encoder_rc6_reset(void* encoder_ptr, const InfraredMessage* message) {
    furi_assert(encoder_ptr);

    InfraredEncoderRC6* encoder = encoder_ptr;
    InfraredCommonEncoder* common_encoder = encoder->common_encoder;
    infrared_common_encoder_reset(common_encoder);

    uint32_t* data = (void*)common_encoder->data;
    *data |= 0x01; // start bit
    (void)*data; // 3 bits for mode == 0
    *data |= encoder->toggle_bit ? 0x10 : 0;
    *data |= reverse(message->address) << 5;
    *data |= reverse(message->command) << 13;

    common_encoder->bits_to_encode = common_encoder->protocol->databit_len[0];
    encoder->toggle_bit ^= 1;
}

InfraredStatus infrared_encoder_rc6_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    InfraredEncoderRC6* encoder = encoder_ptr;
    return infrared_common_encode(encoder->common_encoder, duration, level);
}

void* infrared_encoder_rc6_alloc(void) {
    InfraredEncoderRC6* encoder = malloc(sizeof(InfraredEncoderRC6));
    encoder->common_encoder = infrared_common_encoder_alloc(&infrared_protocol_rc6);
    encoder->toggle_bit = false;
    return encoder;
}

void infrared_encoder_rc6_free(void* encoder_ptr) {
    furi_assert(encoder_ptr);

    InfraredEncoderRC6* encoder = encoder_ptr;
    free(encoder->common_encoder);
    free(encoder);
}

InfraredStatus infrared_encoder_rc6_encode_manchester(
    InfraredCommonEncoder* common_encoder,
    uint32_t* duration,
    bool* polarity) {
    InfraredStatus status = InfraredStatusError;

    bool toggle_bit = (common_encoder->bits_encoded == 4);
    status = infrared_common_encode_manchester(common_encoder, duration, polarity);
    if(toggle_bit) *duration *= 2;
    return status;
}
