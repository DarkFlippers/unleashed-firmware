#include "infrared_protocol_sirc_i.h"
#include <core/check.h>

InfraredMessage* infrared_decoder_sirc_check_ready(void* ctx) {
    return infrared_common_decoder_check_ready(ctx);
}

bool infrared_decoder_sirc_interpret(InfraredCommonDecoder* decoder) {
    furi_assert(decoder);

    uint32_t* data = (void*)&decoder->data[0];
    uint16_t address = 0;
    uint8_t command = 0;
    InfraredProtocol protocol = InfraredProtocolUnknown;

    if(decoder->databit_cnt == 12) {
        address = (*data >> 7) & 0x1F;
        command = *data & 0x7F;
        protocol = InfraredProtocolSIRC;
    } else if(decoder->databit_cnt == 15) {
        address = (*data >> 7) & 0xFF;
        command = *data & 0x7F;
        protocol = InfraredProtocolSIRC15;
    } else if(decoder->databit_cnt == 20) {
        address = (*data >> 7) & 0x1FFF;
        command = *data & 0x7F;
        protocol = InfraredProtocolSIRC20;
    } else {
        return false;
    }

    decoder->message.protocol = protocol;
    decoder->message.address = address;
    decoder->message.command = command;
    /* SIRC doesn't specify repeat detection */
    decoder->message.repeat = false;

    return true;
}

void* infrared_decoder_sirc_alloc(void) {
    return infrared_common_decoder_alloc(&infrared_protocol_sirc);
}

InfraredMessage* infrared_decoder_sirc_decode(void* decoder, bool level, uint32_t duration) {
    return infrared_common_decode(decoder, level, duration);
}

void infrared_decoder_sirc_free(void* decoder) {
    infrared_common_decoder_free(decoder);
}

void infrared_decoder_sirc_reset(void* decoder) {
    infrared_common_decoder_reset(decoder);
}
