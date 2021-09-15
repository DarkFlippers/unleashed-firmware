#include "furi/check.h"
#include "irda.h"
#include "common/irda_common_i.h"
#include <stdint.h>
#include "../irda_i.h"
#include "irda_protocol_defs_i.h"
#include <furi.h>

static const uint32_t repeat_timings[] = {
    IRDA_NEC_REPEAT_PERIOD - IRDA_NEC_REPEAT_MARK - IRDA_NEC_REPEAT_SPACE - IRDA_NEC_BIT1_MARK,
    IRDA_NEC_REPEAT_MARK,
    IRDA_NEC_REPEAT_SPACE,
    IRDA_NEC_BIT1_MARK,
};

void irda_encoder_nec_reset(void* encoder_ptr, const IrdaMessage* message) {
    furi_assert(encoder_ptr);
    furi_assert(message);

    IrdaCommonEncoder* encoder = encoder_ptr;
    irda_common_encoder_reset(encoder);

    uint32_t* data1 = (void*) encoder->data;
    uint32_t* data2 = data1 + 1;
    if (message->protocol == IrdaProtocolNEC) {
        uint8_t address = message->address;
        uint8_t address_inverse = ~address;
        uint8_t command = message->command;
        uint8_t command_inverse = ~command;
        *data1 = address;
        *data1 |= address_inverse << 8;
        *data1 |= command << 16;
        *data1 |= command_inverse << 24;
        encoder->bits_to_encode = 32;
    } else if (message->protocol == IrdaProtocolNECext) {
        *data1 = (uint16_t) message->address;
        *data1 |= (message->command & 0xFFFF) << 16;
        encoder->bits_to_encode = 32;
    } else if (message->protocol == IrdaProtocolNEC42) {
        /* 13 address + 13 inverse address + 8 command + 8 inv command */
        *data1 = message->address & 0x1FFFUL;
        *data1 |= (~message->address & 0x1FFFUL) << 13;
        *data1 |= ((message->command & 0x3FUL) << 26);
        *data2 = (message->command & 0xC0UL) >> 6;
        *data2 |= (~message->command & 0xFFUL) << 2;
        encoder->bits_to_encode = 42;
    } else if (message->protocol == IrdaProtocolNEC42ext) {
        *data1 = message->address & 0x3FFFFFF;
        *data1 |= ((message->command & 0x3F) << 26);
        *data2 = (message->command & 0xFFC0) >> 6;
        encoder->bits_to_encode = 42;
    } else {
        furi_assert(0);
    }
}

IrdaStatus irda_encoder_nec_encode_repeat(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level) {
    furi_assert(encoder);

    /* space + 2 timings preambule + payload + stop bit */
    uint32_t timings_encoded_up_to_repeat = 1 + 2 + encoder->bits_to_encode * 2 + 1;
    uint32_t repeat_cnt = encoder->timings_encoded - timings_encoded_up_to_repeat;

    furi_assert(encoder->timings_encoded >= timings_encoded_up_to_repeat);

    if (repeat_cnt > 0) {
        *duration = repeat_timings[repeat_cnt % COUNT_OF(repeat_timings)];
    } else {
        *duration = IRDA_NEC_REPEAT_PERIOD - encoder->timings_sum;
    }

    *level = repeat_cnt % 2;
    ++encoder->timings_encoded;
    bool done = (!((repeat_cnt + 1) % COUNT_OF(repeat_timings)));

    return done ? IrdaStatusDone : IrdaStatusOk;
}

void* irda_encoder_nec_alloc(void) {
    return irda_common_encoder_alloc(&protocol_nec);
}

void irda_encoder_nec_free(void* encoder_ptr) {
    irda_common_encoder_free(encoder_ptr);
}

IrdaStatus irda_encoder_nec_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    return irda_common_encode(encoder_ptr, duration, level);
}

