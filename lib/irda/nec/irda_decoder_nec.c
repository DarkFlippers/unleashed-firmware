#include <stdbool.h>
#include <stdint.h>
#include <furi.h>
#include "../irda_i.h"


static bool interpret_nec(IrdaCommonDecoder* decoder);
static bool interpret_necext(IrdaCommonDecoder* decoder);
static DecodeStatus decode_repeat_nec(IrdaCommonDecoder* decoder);


static const IrdaCommonProtocolSpec protocol_nec = {
    {
        IRDA_NEC_PREAMBULE_MARK,
        IRDA_NEC_PREAMBULE_SPACE,
        IRDA_NEC_BIT1_MARK,
        IRDA_NEC_BIT1_SPACE,
        IRDA_NEC_BIT0_MARK,
        IRDA_NEC_BIT0_SPACE,
        IRDA_NEC_PREAMBLE_TOLERANCE,
        IRDA_NEC_BIT_TOLERANCE,
    },
    32,
    irda_common_decode_pdwm,
    interpret_nec,
    decode_repeat_nec,
};

static const IrdaCommonProtocolSpec protocol_necext = {
    {
        IRDA_NEC_PREAMBULE_MARK,
        IRDA_NEC_PREAMBULE_SPACE,
        IRDA_NEC_BIT1_MARK,
        IRDA_NEC_BIT1_SPACE,
        IRDA_NEC_BIT0_MARK,
        IRDA_NEC_BIT0_SPACE,
        IRDA_NEC_PREAMBLE_TOLERANCE,
        IRDA_NEC_BIT_TOLERANCE,
    },
    32,
    irda_common_decode_pdwm,
    interpret_necext,
    decode_repeat_nec,
};

static bool interpret_nec(IrdaCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;
    uint8_t address = decoder->data[0];
    uint8_t address_inverse = decoder->data[1];
    uint8_t command = decoder->data[2];
    uint8_t command_inverse = decoder->data[3];

    if ((command == (uint8_t) ~command_inverse) && (address == (uint8_t) ~address_inverse)) {
        decoder->message.command = command;
        decoder->message.address = address;
        decoder->message.repeat = false;
        result = true;
    }

    return result;
}

// Some NEC's extensions allow 16 bit address
static bool interpret_necext(IrdaCommonDecoder* decoder) {
    furi_assert(decoder);

    bool result = false;
    uint8_t command = decoder->data[2];
    uint8_t command_inverse = decoder->data[3];

    if(command == (uint8_t)~command_inverse) {
        decoder->message.command = command;
        decoder->message.address = decoder->data[0] | (decoder->data[1] << 8);
        decoder->message.repeat = false;
        result = true;
    }

    return result;
}

// timings start from Space (delay between message and repeat)
static DecodeStatus decode_repeat_nec(IrdaCommonDecoder* decoder) {
    furi_assert(decoder);

    float preamble_tolerance = decoder->protocol->timings.preamble_tolerance;
    uint32_t bit_tolerance = decoder->protocol->timings.bit_tolerance;
    DecodeStatus status = DecodeStatusError;

    if(decoder->timings_cnt < 4) return DecodeStatusOk;

    if((decoder->timings[0] > IRDA_NEC_REPEAT_PAUSE_MIN) &&
       (decoder->timings[0] < IRDA_NEC_REPEAT_PAUSE_MAX) &&
       MATCH_PREAMBLE_TIMING(decoder->timings[1], IRDA_NEC_REPEAT_MARK, preamble_tolerance) &&
       MATCH_PREAMBLE_TIMING(decoder->timings[2], IRDA_NEC_REPEAT_SPACE, preamble_tolerance) &&
       MATCH_BIT_TIMING(decoder->timings[3], decoder->protocol->timings.bit1_mark, bit_tolerance)) {
        status = DecodeStatusReady;
        decoder->timings_cnt = 0;
    } else {
        status = DecodeStatusError;
    }

    return status;
}

void* irda_decoder_nec_alloc(void) {
    return irda_common_decoder_alloc(&protocol_nec);
}

void* irda_decoder_necext_alloc(void) {
    return irda_common_decoder_alloc(&protocol_necext);
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

