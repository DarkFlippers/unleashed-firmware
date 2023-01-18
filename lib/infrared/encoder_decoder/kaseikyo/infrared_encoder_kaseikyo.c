#include "infrared_protocol_kaseikyo_i.h"
#include <core/check.h>

void infrared_encoder_kaseikyo_reset(void* encoder_ptr, const InfraredMessage* message) {
    furi_assert(encoder_ptr);

    InfraredCommonEncoder* encoder = encoder_ptr;
    infrared_common_encoder_reset(encoder);

    uint32_t address = message->address;
    uint16_t command = message->command;

    uint8_t id = (address >> 24) & 3;
    uint16_t vendor_id = (address >> 8) & 0xffff;
    uint8_t genre1 = (address >> 4) & 0xf;
    uint8_t genre2 = address & 0xf;

    encoder->data[0] = (uint8_t)(vendor_id & 0xff);
    encoder->data[1] = (uint8_t)(vendor_id >> 8);
    uint8_t vendor_parity = encoder->data[0] ^ encoder->data[1];
    vendor_parity = (vendor_parity & 0xf) ^ (vendor_parity >> 4);
    encoder->data[2] = (vendor_parity & 0xf) | (genre1 << 4);
    encoder->data[3] = (genre2 & 0xf) | ((uint8_t)(command & 0xf) << 4);
    encoder->data[4] = (id << 6) | (uint8_t)(command >> 4);
    encoder->data[5] = encoder->data[2] ^ encoder->data[3] ^ encoder->data[4];

    encoder->bits_to_encode = encoder->protocol->databit_len[0];
}

void* infrared_encoder_kaseikyo_alloc(void) {
    return infrared_common_encoder_alloc(&infrared_protocol_kaseikyo);
}

void infrared_encoder_kaseikyo_free(void* encoder_ptr) {
    infrared_common_encoder_free(encoder_ptr);
}

InfraredStatus
    infrared_encoder_kaseikyo_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    return infrared_common_encode(encoder_ptr, duration, level);
}
