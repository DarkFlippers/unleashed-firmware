#include "infrared_protocol_rca_i.h"

#include <core/check.h>

void infrared_encoder_rca_reset(void* encoder_ptr, const InfraredMessage* message) {
    furi_assert(encoder_ptr);
    furi_assert(message);

    InfraredCommonEncoder* encoder = encoder_ptr;
    infrared_common_encoder_reset(encoder);

    uint32_t* data = (void*)encoder->data;

    uint8_t address = message->address;
    uint8_t address_inverse = ~address;
    uint8_t command = message->command;
    uint8_t command_inverse = ~command;

    *data = address & 0xF;
    *data |= command << 4;
    *data |= (address_inverse & 0xF) << 12;
    *data |= command_inverse << 16;

    encoder->bits_to_encode = encoder->protocol->databit_len[0];
}

void* infrared_encoder_rca_alloc(void) {
    return infrared_common_encoder_alloc(&infrared_protocol_rca);
}

void infrared_encoder_rca_free(void* encoder_ptr) {
    infrared_common_encoder_free(encoder_ptr);
}

InfraredStatus infrared_encoder_rca_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    return infrared_common_encode(encoder_ptr, duration, level);
}
