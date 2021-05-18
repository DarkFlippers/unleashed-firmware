#include <stdint.h>
#include "../irda_i.h"


static const IrdaEncoderTimings encoder_timings = {
    .bit1_mark = IRDA_NEC_BIT1_MARK,
    .bit1_space = IRDA_NEC_BIT1_SPACE,
    .bit0_mark =IRDA_NEC_BIT0_MARK,
    .bit0_space = IRDA_NEC_BIT0_SPACE,
    .duty_cycle = IRDA_NEC_DUTY_CYCLE,
    .carrier_frequency = IRDA_NEC_CARRIER_FREQUENCY,
};


static void irda_encode_nec_preamble(void) {
    irda_encode_mark(&encoder_timings, IRDA_NEC_PREAMBULE_MARK);
    irda_encode_space(&encoder_timings, IRDA_NEC_PREAMBULE_SPACE);
}

static void irda_encode_nec_repeat(void) {
    irda_encode_space(&encoder_timings, IRDA_NEC_REPEAT_PAUSE);
    irda_encode_mark(&encoder_timings, IRDA_NEC_REPEAT_MARK);
    irda_encode_space(&encoder_timings, IRDA_NEC_REPEAT_SPACE);
    irda_encode_bit(&encoder_timings, 1);
}

void irda_encoder_nec_encode(uint32_t addr, uint32_t cmd, bool repeat) {
    uint8_t address = addr & 0xFF;
    uint8_t command = cmd & 0xFF;
    uint8_t address_inverse = (uint8_t) ~address;
    uint8_t command_inverse = (uint8_t) ~command;

    if (!repeat) {
        irda_encode_nec_preamble();
        irda_encode_byte(&encoder_timings, address);
        irda_encode_byte(&encoder_timings, address_inverse);
        irda_encode_byte(&encoder_timings, command);
        irda_encode_byte(&encoder_timings, command_inverse);
        irda_encode_bit(&encoder_timings, 1);
    } else {
        irda_encode_nec_repeat();
    }
}

