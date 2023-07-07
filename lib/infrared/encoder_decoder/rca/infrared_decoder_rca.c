#include "infrared_protocol_rca_i.h"
#include <core/check.h>

InfraredMessage* infrared_decoder_rca_check_ready(void* ctx) {
    return infrared_common_decoder_check_ready(ctx);
}

bool infrared_decoder_rca_interpret(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    uint32_t* data = (void*)&decoder->data;

    uint8_t address = (*data & 0xF);
    uint8_t command = (*data >> 4) & 0xFF;
    uint8_t address_inverse = (*data >> 12) & 0xF;
    uint8_t command_inverse = (*data >> 16) & 0xFF;
    uint8_t inverse_address_inverse = (uint8_t)~address_inverse & 0xF;
    uint8_t inverse_command_inverse = (uint8_t)~command_inverse;

    if((command == inverse_command_inverse) && (address == inverse_address_inverse)) {
        decoder->message.protocol = InfraredProtocolRCA;
        decoder->message.address = address;
        decoder->message.command = command;
        decoder->message.repeat = false;
        return true;
    }

    return false;
}

void* infrared_decoder_rca_alloc(void) {
    return infrared_common_decoder_alloc(&infrared_protocol_rca);
}

InfraredMessage* infrared_decoder_rca_decode(void* decoder, bool level, uint32_t duration) {
    return infrared_common_decode(decoder, level, duration);
}

void infrared_decoder_rca_free(void* decoder) {
    infrared_common_decoder_free(decoder);
}

void infrared_decoder_rca_reset(void* decoder) {
    infrared_common_decoder_reset(decoder);
}
