#include "irda.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <furi.h>
#include <api-hal-irda.h>
#include <api-hal-delay.h>

#define IRDA_SET_TX_COMMON(d, l)        irda_set_tx((d), (l), IRDA_COMMON_DUTY_CYCLE, IRDA_COMMON_CARRIER_FREQUENCY)

static void irda_set_tx(uint32_t duration, bool level, float duty_cycle, float frequency) {
    if (level) {
        api_hal_irda_pwm_set(duty_cycle, frequency);
        delay_us(duration);
    } else {
        api_hal_irda_pwm_stop();
        delay_us(duration);
    }
}

void irda_send_raw_ext(const uint32_t timings[], uint32_t timings_cnt, bool start_from_mark, float duty_cycle, float frequency) {
    __disable_irq();
    for (uint32_t i = 0; i < timings_cnt; ++i) {
        irda_set_tx(timings[i], (i % 2) ^ start_from_mark, duty_cycle, frequency);
    }
    IRDA_SET_TX_COMMON(0, false);
    __enable_irq();
}

void irda_send_raw(const uint32_t timings[], uint32_t timings_cnt, bool start_from_mark) {
    __disable_irq();
    for (uint32_t i = 0; i < timings_cnt; ++i) {
        IRDA_SET_TX_COMMON(timings[i], (i % 2) ^ start_from_mark);
    }
    IRDA_SET_TX_COMMON(0, false);
    __enable_irq();
}

void irda_send(const IrdaMessage* message, int times) {
    furi_assert(message);
    furi_assert(irda_is_protocol_valid(message->protocol));

    IrdaStatus status;
    uint32_t duration = 0;
    bool level = false;
    IrdaEncoderHandler* handler = irda_alloc_encoder();
    irda_reset_encoder(handler, message);

    /* Hotfix: first timings is space timing, so make delay instead of locking
     * whole system for that long. Replace when async timing lib will be ready.
     * This timing doesn't have to be precise.
     */
    status = irda_encode(handler, &duration, &level);
    furi_assert(status != IrdaStatusError);
    furi_assert(level == false);
    delay_us(duration);

    __disable_irq();

    while (times) {
        status = irda_encode(handler, &duration, &level);
        if (status != IrdaStatusError) {
            IRDA_SET_TX_COMMON(duration, level);
        } else {
            furi_assert(0);
            break;
        }
        if (status == IrdaStatusDone)
            --times;
    }

    IRDA_SET_TX_COMMON(0, false);
    __enable_irq();

    irda_free_encoder(handler);
}

