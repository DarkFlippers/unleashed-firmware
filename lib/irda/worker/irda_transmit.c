#include "irda.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <furi.h>
#include <furi-hal-irda.h>
#include <furi-hal-delay.h>

static uint32_t irda_tx_number_of_transmissions = 0;
static uint32_t irda_tx_raw_timings_index = 0;
static uint32_t irda_tx_raw_timings_number = 0;
static uint32_t irda_tx_raw_start_from_mark = 0;
static bool irda_tx_raw_add_silence = false;

FuriHalIrdaTxGetDataState irda_get_raw_data_callback (void* context, uint32_t* duration, bool* level) {
    furi_assert(duration);
    furi_assert(level);
    furi_assert(context);

    FuriHalIrdaTxGetDataState state = FuriHalIrdaTxGetDataStateOk;
    const uint32_t* timings = context;

    if (irda_tx_raw_add_silence && (irda_tx_raw_timings_index == 0)) {
        irda_tx_raw_add_silence = false;
        *level = false;
        *duration = IRDA_RAW_TX_TIMING_DELAY_US;
    } else {
        *level = irda_tx_raw_start_from_mark ^ (irda_tx_raw_timings_index % 2);
        *duration = timings[irda_tx_raw_timings_index++];
    }

    if (irda_tx_raw_timings_number == irda_tx_raw_timings_index) {
        state = FuriHalIrdaTxGetDataStateLastDone;
    }

    return state;
}

void irda_send_raw_ext(const uint32_t timings[], uint32_t timings_cnt, bool start_from_mark, uint32_t frequency, float duty_cycle) {
    furi_assert(timings);

    irda_tx_raw_start_from_mark = start_from_mark;
    irda_tx_raw_timings_index = 0;
    irda_tx_raw_timings_number = timings_cnt;
    irda_tx_raw_add_silence = start_from_mark;
    furi_hal_irda_async_tx_set_data_isr_callback(irda_get_raw_data_callback, (void*) timings);
    furi_hal_irda_async_tx_start(frequency, duty_cycle);
    furi_hal_irda_async_tx_wait_termination();

    furi_assert(!furi_hal_irda_is_busy());
}

void irda_send_raw(const uint32_t timings[], uint32_t timings_cnt, bool start_from_mark) {
    irda_send_raw_ext(timings, timings_cnt, start_from_mark, IRDA_COMMON_CARRIER_FREQUENCY, IRDA_COMMON_DUTY_CYCLE);
}

FuriHalIrdaTxGetDataState irda_get_data_callback (void* context, uint32_t* duration, bool* level) {
    FuriHalIrdaTxGetDataState state = FuriHalIrdaTxGetDataStateLastDone;
    IrdaEncoderHandler* handler = context;
    IrdaStatus status = IrdaStatusError;

    if (irda_tx_number_of_transmissions > 0) {
        status = irda_encode(handler, duration, level);
    }

    if (status == IrdaStatusError) {
        state = FuriHalIrdaTxGetDataStateLastDone;
        *duration = 0;
        *level = 0;
    } else if (status == IrdaStatusOk) {
        state = FuriHalIrdaTxGetDataStateOk;
    } else if (status == IrdaStatusDone) {
        state = FuriHalIrdaTxGetDataStateDone;
        if (--irda_tx_number_of_transmissions == 0) {
            state = FuriHalIrdaTxGetDataStateLastDone;
        }
    } else {
        furi_check(0);
    }

    return state;
}

void irda_send(const IrdaMessage* message, int times) {
    furi_assert(message);
    furi_assert(times);
    furi_assert(irda_is_protocol_valid(message->protocol));

    IrdaEncoderHandler* handler = irda_alloc_encoder();
    irda_reset_encoder(handler, message);
    irda_tx_number_of_transmissions = times;

    uint32_t frequency = irda_get_protocol_frequency(message->protocol);
    float duty_cycle = irda_get_protocol_duty_cycle(message->protocol);

    furi_hal_irda_async_tx_set_data_isr_callback(irda_get_data_callback, handler);
    furi_hal_irda_async_tx_start(frequency, duty_cycle);
    furi_hal_irda_async_tx_wait_termination();

    irda_free_encoder(handler);

    furi_assert(!furi_hal_irda_is_busy());
}

