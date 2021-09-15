#include "furi/check.h"
#include "common/irda_common_i.h"
#include <stdint.h>
#include "../irda_i.h"
#include "irda_protocol_defs_i.h"
#include <furi.h>

static const uint32_t repeat_timings[] = {
    IRDA_SAMSUNG_REPEAT_PAUSE2,
    IRDA_SAMSUNG_REPEAT_MARK,
    IRDA_SAMSUNG_REPEAT_SPACE,
    IRDA_SAMSUNG_BIT1_MARK,
    IRDA_SAMSUNG_BIT1_SPACE,
    IRDA_SAMSUNG_BIT1_MARK,
};

void irda_encoder_samsung32_reset(void* encoder_ptr, const IrdaMessage* message) {
    furi_assert(encoder_ptr);

    IrdaCommonEncoder* encoder = encoder_ptr;
    irda_common_encoder_reset(encoder);

    uint8_t address = message->address;
    uint8_t command = message->command;
    uint8_t command_inverse = ~command;

    uint32_t* data = (void*) encoder->data;
    *data |= address;
    *data |= address << 8;
    *data |= command << 16;
    *data |= command_inverse << 24;

    encoder->bits_to_encode = encoder->protocol->databit_len[0];
}

IrdaStatus irda_encoder_samsung32_encode_repeat(IrdaCommonEncoder* encoder, uint32_t* duration, bool* level) {
    furi_assert(encoder);

    /* space + 2 timings preambule + payload + stop bit */
    uint32_t timings_encoded_up_to_repeat = 1 + 2 + encoder->bits_encoded * 2 + 1;
    uint32_t repeat_cnt = encoder->timings_encoded - timings_encoded_up_to_repeat;

    furi_assert(encoder->timings_encoded >= timings_encoded_up_to_repeat);

    if (repeat_cnt > 0)
        *duration = repeat_timings[repeat_cnt % COUNT_OF(repeat_timings)];
    else
        *duration = IRDA_SAMSUNG_REPEAT_PAUSE1;

    *level = repeat_cnt % 2;
    ++encoder->timings_encoded;
    bool done = (!((repeat_cnt + 1) % COUNT_OF(repeat_timings)));

    return done ? IrdaStatusDone : IrdaStatusOk;
}

void* irda_encoder_samsung32_alloc(void) {
    return irda_common_encoder_alloc(&protocol_samsung32);
}

void irda_encoder_samsung32_free(void* encoder_ptr) {
    irda_common_encoder_free(encoder_ptr);
}

IrdaStatus irda_encoder_samsung32_encode(void* encoder_ptr, uint32_t* duration, bool* level) {
    return irda_common_encode(encoder_ptr, duration, level);
}


