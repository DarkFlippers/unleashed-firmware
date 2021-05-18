#include <stdint.h>
#include <stdbool.h>
#include <api-hal-irda.h>
#include <api-hal-delay.h>
#include "irda_i.h"


void irda_encode_mark(const IrdaEncoderTimings *timings, uint32_t duration) {
    api_hal_irda_pwm_set(timings->duty_cycle, timings->carrier_frequency);
    delay_us(duration);
}

void irda_encode_space(const IrdaEncoderTimings *timings, uint32_t duration) {
    (void) timings;
    api_hal_irda_pwm_stop();
    delay_us(duration);
}

void irda_encode_bit(const IrdaEncoderTimings *timings, bool bit) {
    if (bit) {
        irda_encode_mark(timings, timings->bit1_mark);
        irda_encode_space(timings, timings->bit1_space);
    } else {
        irda_encode_mark(timings, timings->bit0_mark);
        irda_encode_space(timings, timings->bit0_space);
    }
}

void irda_encode_byte(const IrdaEncoderTimings *timings, uint8_t data) {
    for(uint8_t i = 0; i < 8; i++) {
        irda_encode_bit(timings, !!(data & (1 << i)));
    }
}

