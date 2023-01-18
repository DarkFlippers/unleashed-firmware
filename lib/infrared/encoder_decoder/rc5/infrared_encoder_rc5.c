#include "infrared_protocol_rc5_i.h"

#include <stdlib.h>
#include <core/check.h>

typedef struct InfraredEncoderRC5 {
    InfraredCommonEncoder* common_encoder;
    bool toggle_bit;
} InfraredEncoderRC5;

void infrared_encoder_rc5_reset(void* encoder_ptr, const InfraredMessage* message) {
    furi_assert(encoder_ptr);

    InfraredEncoderRC5* encoder = encoder_ptr;
    InfraredCommonEncoder* common_encoder = encoder->common_encoder;
    infrared_common_encoder_reset(common_encoder);

    uint32_t* data = (void*)common_encoder->data;
    /* RC5 */
    *data |= 0x01; // start bit
    if(message->protocol == InfraredProtocolRC5) {
        *data |= 0x02; // start bit
    }
    *data |= encoder->toggle_bit ? 0x04 : 0;
    *data |= (reverse(message->address) >> 3) << 3; /* address 5 bit */
    *data |= (reverse(message->command) >> 2) << 8; /* command 6 bit */

    common_encoder->data[0] = ~common_encoder->data[0];
    common_encoder->data[1] = ~common_encoder->data[1];

    common_encoder->bits_to_encode = common_encoder->protocol->databit_len[0];
    encoder->toggle_bit ^= 1;
}

InfraredStatus infrared_encoder_rc5_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    InfraredEncoderRC5* encoder = encoder_ptr;
    return infrared_common_encode(encoder->common_encoder, duration, level);
}

void* infrared_encoder_rc5_alloc(void) {
    InfraredEncoderRC5* encoder = malloc(sizeof(InfraredEncoderRC5));
    encoder->common_encoder = infrared_common_encoder_alloc(&infrared_protocol_rc5);
    encoder->toggle_bit = false;
    return encoder;
}

void infrared_encoder_rc5_free(void* encoder_ptr) {
    furi_assert(encoder_ptr);

    InfraredEncoderRC5* encoder = encoder_ptr;
    free(encoder->common_encoder);
    free(encoder);
}
