#include "infrared_protocol_nec_i.h"

#include <core/core_defines.h>
#include <core/check.h>

static const uint32_t repeat_timings[] = {
    INFRARED_NEC_REPEAT_PERIOD - INFRARED_NEC_REPEAT_MARK - INFRARED_NEC_REPEAT_SPACE -
        INFRARED_NEC_BIT1_MARK,
    INFRARED_NEC_REPEAT_MARK,
    INFRARED_NEC_REPEAT_SPACE,
    INFRARED_NEC_BIT1_MARK,
};

void infrared_encoder_nec_reset(void* encoder_ptr, const InfraredMessage* message) {
    furi_assert(encoder_ptr);
    furi_assert(message);

    InfraredCommonEncoder* encoder = encoder_ptr;
    infrared_common_encoder_reset(encoder);

    uint32_t* data1 = (void*)encoder->data;
    uint32_t* data2 = data1 + 1;
    if(message->protocol == InfraredProtocolNEC) {
        uint8_t address = message->address;
        uint8_t address_inverse = ~address;
        uint8_t command = message->command;
        uint8_t command_inverse = ~command;
        *data1 = address;
        *data1 |= address_inverse << 8;
        *data1 |= command << 16;
        *data1 |= command_inverse << 24;
        encoder->bits_to_encode = 32;
    } else if(message->protocol == InfraredProtocolNECext) {
        *data1 = (uint16_t)message->address;
        *data1 |= (message->command & 0xFFFF) << 16;
        encoder->bits_to_encode = 32;
    } else if(message->protocol == InfraredProtocolNEC42) {
        /* 13 address + 13 inverse address + 8 command + 8 inv command */
        *data1 = message->address & 0x1FFFUL;
        *data1 |= (~message->address & 0x1FFFUL) << 13;
        *data1 |= ((message->command & 0x3FUL) << 26);
        *data2 = (message->command & 0xC0UL) >> 6;
        *data2 |= (~message->command & 0xFFUL) << 2;
        encoder->bits_to_encode = 42;
    } else if(message->protocol == InfraredProtocolNEC42ext) {
        *data1 = message->address & 0x3FFFFFF;
        *data1 |= ((message->command & 0x3F) << 26);
        *data2 = (message->command & 0xFFC0) >> 6;
        encoder->bits_to_encode = 42;
    } else {
        furi_crash();
    }
}

InfraredStatus infrared_encoder_nec_encode_repeat(
    InfraredCommonEncoder* encoder,
    uint32_t* duration,
    bool* level) {
    furi_assert(encoder);

    /* space + 2 timings preambule + payload + stop bit */
    uint32_t timings_encoded_up_to_repeat = 1 + 2 + encoder->bits_to_encode * 2 + 1;
    uint32_t repeat_cnt = encoder->timings_encoded - timings_encoded_up_to_repeat;

    furi_assert(encoder->timings_encoded >= timings_encoded_up_to_repeat);

    if(repeat_cnt > 0) {
        *duration = repeat_timings[repeat_cnt % COUNT_OF(repeat_timings)];
    } else {
        *duration = INFRARED_NEC_REPEAT_PERIOD - encoder->timings_sum;
    }

    *level = repeat_cnt % 2;
    ++encoder->timings_encoded;
    bool done = (!((repeat_cnt + 1) % COUNT_OF(repeat_timings)));

    return done ? InfraredStatusDone : InfraredStatusOk;
}

void* infrared_encoder_nec_alloc(void) {
    return infrared_common_encoder_alloc(&infrared_protocol_nec);
}

void infrared_encoder_nec_free(void* encoder_ptr) {
    infrared_common_encoder_free(encoder_ptr);
}

InfraredStatus infrared_encoder_nec_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    return infrared_common_encode(encoder_ptr, duration, level);
}
