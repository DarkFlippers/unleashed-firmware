#include "common/irda_common_i.h"
#include "irda.h"
#include "irda_protocol_defs_i.h"
#include <stdbool.h>
#include <stdint.h>
#include <furi.h>
#include "../irda_i.h"


IrdaMessage* irda_decoder_sirc_check_ready(void* ctx) {
    IrdaMessage* message = NULL;
    IrdaCommonDecoder* decoder = ctx;

    if (irda_decoder_sirc_interpret(decoder)) {
        message = &decoder->message;
        decoder->timings_cnt = 0;
        decoder->databit_cnt = 0;
    }

    return message;
}

bool irda_decoder_sirc_interpret(IrdaCommonDecoder* decoder) {
    furi_assert(decoder);

    uint32_t* data = (void*) &decoder->data[0];
    uint16_t address = 0;
    uint8_t command = 0;
    IrdaProtocol protocol = IrdaProtocolUnknown;

    if (decoder->databit_cnt == 12) {
        address = (*data >> 7)  & 0x1F;
        command = *data & 0x7F;
        protocol = IrdaProtocolSIRC;
    } else if (decoder->databit_cnt == 15) {
        address = (*data >> 7)  & 0xFF;
        command = *data & 0x7F;
        protocol = IrdaProtocolSIRC15;
    } else if (decoder->databit_cnt == 20) {
        address = (*data >> 7)  & 0x1FFF;
        command = *data & 0x7F;
        protocol = IrdaProtocolSIRC20;
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

void* irda_decoder_sirc_alloc(void) {
    return irda_common_decoder_alloc(&protocol_sirc);
}

IrdaMessage* irda_decoder_sirc_decode(void* context, bool level, uint32_t duration) {
    IrdaCommonDecoder* decoder = context;
    IrdaMessage* message = NULL;

    if ((decoder->databit_cnt == 12) || (decoder->databit_cnt == 15)) {
        if (!level && (duration >= IRDA_SIRC_MIN_SILENCE)) {
            if (irda_decoder_sirc_interpret(decoder)) {
                message = &decoder->message;
                decoder->timings_cnt = 0;
                decoder->databit_cnt = 0;
            }
        }
    }

    if (!message) {
        message = irda_common_decode(decoder, level, duration);
        if (message) {  /* 20 bit */
            decoder->timings_cnt = 0;
            decoder->databit_cnt = 0;
        }
    }

    return message;
}

void irda_decoder_sirc_free(void* decoder) {
    irda_common_decoder_free(decoder);
}

void irda_decoder_sirc_reset(void* decoder) {
    irda_common_decoder_reset(decoder);
}

