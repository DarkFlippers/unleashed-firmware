#include "common/irda_common_i.h"
#include "irda.h"
#include "irda_protocol_defs_i.h"
#include <stdbool.h>
#include <stdint.h>
#include <furi.h>
#include "../irda_i.h"


IrdaMessage* irda_decoder_nec_check_ready(void* ctx) {
    return irda_common_decoder_check_ready(ctx);
}

bool irda_decoder_nec_interpret(IrdaCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;

    if (decoder->databit_cnt == 32) {
        uint8_t address = decoder->data[0];
        uint8_t address_inverse = decoder->data[1];
        uint8_t command = decoder->data[2];
        uint8_t command_inverse = decoder->data[3];
        if ((command == (uint8_t) ~command_inverse) && (address == (uint8_t) ~address_inverse)) {
            decoder->message.protocol = IrdaProtocolNEC;
            decoder->message.address = address;
            decoder->message.command = command;
            decoder->message.repeat = false;
            result = true;
        } else {
            decoder->message.protocol = IrdaProtocolNECext;
            decoder->message.address = decoder->data[0] | (decoder->data[1] << 8);
            decoder->message.command = decoder->data[2] | (decoder->data[3] << 8);
            decoder->message.repeat = false;
            result = true;
        }
    } else if (decoder->databit_cnt == 42) {
        uint32_t* data1 = (void*) decoder->data;
        uint16_t* data2 = (void*) (data1 + 1);
        uint16_t address = *data1 & 0x1FFF;
        uint16_t address_inverse = (*data1 >> 13) & 0x1FFF;
        uint16_t command = ((*data1 >> 26) & 0x3F) | ((*data2 & 0x3) << 6);
        uint16_t command_inverse = (*data2 >> 2) & 0xFF;

        if ((address == (~address_inverse & 0x1FFF))
            && (command == (~command_inverse & 0xFF))) {
            decoder->message.protocol = IrdaProtocolNEC42;
            decoder->message.address = address;
            decoder->message.command = command;
            decoder->message.repeat = false;
            result = true;
        } else {
            decoder->message.protocol = IrdaProtocolNEC42ext;
            decoder->message.address = address | (address_inverse << 13);
            decoder->message.command = command | (command_inverse << 8);
            decoder->message.repeat = false;
            result = true;
        }
    }

    return result;
}

// timings start from Space (delay between message and repeat)
IrdaStatus irda_decoder_nec_decode_repeat(IrdaCommonDecoder* decoder) {
    furi_assert(decoder);

    float preamble_tolerance = decoder->protocol->timings.preamble_tolerance;
    uint32_t bit_tolerance = decoder->protocol->timings.bit_tolerance;
    IrdaStatus status = IrdaStatusError;

    if(decoder->timings_cnt < 4) return IrdaStatusOk;

    if((decoder->timings[0] > IRDA_NEC_REPEAT_PAUSE_MIN) &&
       (decoder->timings[0] < IRDA_NEC_REPEAT_PAUSE_MAX) &&
       MATCH_TIMING(decoder->timings[1], IRDA_NEC_REPEAT_MARK, preamble_tolerance) &&
       MATCH_TIMING(decoder->timings[2], IRDA_NEC_REPEAT_SPACE, preamble_tolerance) &&
       MATCH_TIMING(decoder->timings[3], decoder->protocol->timings.bit1_mark, bit_tolerance)) {
        status = IrdaStatusReady;
        decoder->timings_cnt = 0;
    } else {
        status = IrdaStatusError;
    }

    return status;
}

void* irda_decoder_nec_alloc(void) {
    return irda_common_decoder_alloc(&protocol_nec);
}

IrdaMessage* irda_decoder_nec_decode(void* decoder, bool level, uint32_t duration) {
    return irda_common_decode(decoder, level, duration);
}

void irda_decoder_nec_free(void* decoder) {
    irda_common_decoder_free(decoder);
}

void irda_decoder_nec_reset(void* decoder) {
    irda_common_decoder_reset(decoder);
}

