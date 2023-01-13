#include "infrared_protocol_kaseikyo_i.h"
#include <core/check.h>

InfraredMessage* infrared_decoder_kaseikyo_check_ready(void* ctx) {
    return infrared_common_decoder_check_ready(ctx);
}

bool infrared_decoder_kaseikyo_interpret(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;
    uint16_t vendor_id = ((uint16_t)(decoder->data[1]) << 8) | (uint16_t)decoder->data[0];
    uint8_t vendor_parity = decoder->data[2] & 0x0f;
    uint8_t genre1 = decoder->data[2] >> 4;
    uint8_t genre2 = decoder->data[3] & 0x0f;
    uint16_t data = (uint16_t)(decoder->data[3] >> 4) | ((uint16_t)(decoder->data[4] & 0x3f) << 4);
    uint8_t id = decoder->data[4] >> 6;
    uint8_t parity = decoder->data[5];

    uint8_t vendor_parity_check = decoder->data[0] ^ decoder->data[1];
    vendor_parity_check = (vendor_parity_check & 0xf) ^ (vendor_parity_check >> 4);
    uint8_t parity_check = decoder->data[2] ^ decoder->data[3] ^ decoder->data[4];

    if(vendor_parity == vendor_parity_check && parity == parity_check) {
        decoder->message.command = (uint32_t)data;
        decoder->message.address = ((uint32_t)id << 24) | ((uint32_t)vendor_id << 8) |
                                   ((uint32_t)genre1 << 4) | (uint32_t)genre2;
        decoder->message.protocol = InfraredProtocolKaseikyo;
        decoder->message.repeat = false;
        result = true;
    }

    return result;
}

void* infrared_decoder_kaseikyo_alloc(void) {
    return infrared_common_decoder_alloc(&infrared_protocol_kaseikyo);
}

InfraredMessage* infrared_decoder_kaseikyo_decode(void* decoder, bool level, uint32_t duration) {
    return infrared_common_decode(decoder, level, duration);
}

void infrared_decoder_kaseikyo_free(void* decoder) {
    infrared_common_decoder_free(decoder);
}

void infrared_decoder_kaseikyo_reset(void* decoder) {
    infrared_common_decoder_reset(decoder);
}
