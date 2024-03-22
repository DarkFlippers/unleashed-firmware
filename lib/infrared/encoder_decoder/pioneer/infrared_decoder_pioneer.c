#include "infrared_protocol_pioneer_i.h"
#include <core/check.h>

InfraredMessage* infrared_decoder_pioneer_check_ready(void* ctx) {
    return infrared_common_decoder_check_ready(ctx);
}

bool infrared_decoder_pioneer_interpret(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    uint32_t* data = (void*)&decoder->data[0];
    uint8_t address = 0;
    uint8_t command = 0;
    InfraredProtocol protocol = InfraredProtocolUnknown;

    if(decoder->databit_cnt == decoder->protocol->databit_len[0] ||
       decoder->databit_cnt == decoder->protocol->databit_len[1]) {
        address = *data & 0xFF;
        uint8_t real_address_checksum = ~address;
        uint8_t address_checksum = (*data >> 8) & 0xFF;
        command = (*data >> 16) & 0xFF;
        uint8_t real_command_checksum = ~command;
        uint8_t command_checksum = (*data >> 24) & 0xFF;
        if(address_checksum != real_address_checksum) {
            return false;
        }
        if(command_checksum != real_command_checksum) {
            return false;
        }
        protocol = InfraredProtocolPioneer;
    } else {
        return false;
    }

    decoder->message.protocol = protocol;
    decoder->message.address = address;
    decoder->message.command = command;
    decoder->message.repeat = false;

    return true;
}

void* infrared_decoder_pioneer_alloc(void) {
    return infrared_common_decoder_alloc(&infrared_protocol_pioneer);
}

InfraredMessage* infrared_decoder_pioneer_decode(void* decoder, bool level, uint32_t duration) {
    return infrared_common_decode(decoder, level, duration);
}

void infrared_decoder_pioneer_free(void* decoder) {
    infrared_common_decoder_free(decoder);
}

void infrared_decoder_pioneer_reset(void* decoder) {
    infrared_common_decoder_reset(decoder);
}
