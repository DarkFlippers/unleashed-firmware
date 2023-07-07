/* Copyright (C) 2022-2023 Salvatore Sanfilippo -- All Rights Reserved
 * See the LICENSE file for information about the license. */

#include "app.h"
#include "custom_presets.h"

#include <flipper_format/flipper_format_i.h>
#include <furi_hal_rtc.h>
#include <furi_hal_spi.h>
#include <furi_hal_interrupt.h>

void raw_sampling_timer_start(ProtoViewApp* app);
void raw_sampling_timer_stop(ProtoViewApp* app);

ProtoViewModulation ProtoViewModulations[] = {
    {"OOK 650Khz", "FuriHalSubGhzPresetOok650Async", FuriHalSubGhzPresetOok650Async, NULL, 30},
    {"OOK 270Khz", "FuriHalSubGhzPresetOok270Async", FuriHalSubGhzPresetOok270Async, NULL, 30},
    {"2FSK 2.38Khz",
     "FuriHalSubGhzPreset2FSKDev238Async",
     FuriHalSubGhzPreset2FSKDev238Async,
     NULL,
     30},
    {"2FSK 47.6Khz",
     "FuriHalSubGhzPreset2FSKDev476Async",
     FuriHalSubGhzPreset2FSKDev476Async,
     NULL,
     30},
    {"TPMS 1 (FSK)", NULL, 0, (uint8_t*)protoview_subghz_tpms1_fsk_async_regs, 30},
    {"TPMS 2 (OOK)", NULL, 0, (uint8_t*)protoview_subghz_tpms2_ook_async_regs, 30},
    {"TPMS 3 (GFSK)", NULL, 0, (uint8_t*)protoview_subghz_tpms3_gfsk_async_regs, 30},
    {"OOK 40kBaud", NULL, 0, (uint8_t*)protoview_subghz_40k_ook_async_regs, 15},
    {"FSK 40kBaud", NULL, 0, (uint8_t*)protoview_subghz_40k_fsk_async_regs, 15},
    {NULL, NULL, 0, NULL, 0} /* End of list sentinel. */
};

/* Called after the application initialization in order to setup the
 * subghz system and put it into idle state. */
void radio_begin(ProtoViewApp* app) {
    furi_assert(app);
    furi_hal_subghz_reset();
    furi_hal_subghz_idle();

    /* Power circuits are noisy. Suppressing the charge while we use
     * ProtoView will improve the RF performances. */
    furi_hal_power_suppress_charge_enter();

    /* The CC1101 preset can be either one of the standard presets, if
     * the modulation "custom" field is NULL, or a custom preset we
     * defined in custom_presets.h. */
    if(ProtoViewModulations[app->modulation].custom == NULL) {
        furi_hal_subghz_load_preset(ProtoViewModulations[app->modulation].preset);
    } else {
        furi_hal_subghz_load_custom_preset(ProtoViewModulations[app->modulation].custom);
    }
    furi_hal_gpio_init(furi_hal_subghz.cc1101_g0_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);
    app->txrx->txrx_state = TxRxStateIDLE;
}

/* ================================= Reception ============================== */

/* We avoid the subghz provided abstractions and put the data in our
 * simple abstraction: the RawSamples circular buffer. */
void protoview_rx_callback(bool level, uint32_t duration, void* context) {
    UNUSED(context);
    /* Add data to the circular buffer. */
    raw_samples_add(RawSamples, level, duration);
    // FURI_LOG_E(TAG, "FEED: %d %d", (int)level, (int)duration);
    return;
}

/* Setup the CC1101 to start receiving using a background worker. */
uint32_t radio_rx(ProtoViewApp* app) {
    furi_assert(app);
    if(!furi_hal_subghz_is_frequency_valid(app->frequency)) {
        furi_crash(TAG " Incorrect RX frequency.");
    }

    if(app->txrx->txrx_state == TxRxStateRx) return app->frequency;

    furi_hal_subghz_idle(); /* Put it into idle state in case it is sleeping. */
    uint32_t value = furi_hal_subghz_set_frequency_and_path(app->frequency);
    FURI_LOG_E(TAG, "Switched to frequency: %lu", value);
    furi_hal_gpio_init(furi_hal_subghz.cc1101_g0_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);
    furi_hal_subghz_flush_rx();
    furi_hal_subghz_rx();
    if(!app->txrx->debug_timer_sampling) {
        furi_hal_subghz_start_async_rx(protoview_rx_callback, NULL);
    } else {
        raw_sampling_worker_start(app);
    }
    app->txrx->txrx_state = TxRxStateRx;
    return value;
}

/* Stop receiving (if active) and put the radio on idle state. */
void radio_rx_end(ProtoViewApp* app) {
    furi_assert(app);

    if(app->txrx->txrx_state == TxRxStateRx) {
        if(!app->txrx->debug_timer_sampling) {
            furi_hal_subghz_stop_async_rx();
        } else {
            raw_sampling_worker_stop(app);
        }
    }
    furi_hal_subghz_idle();
    app->txrx->txrx_state = TxRxStateIDLE;
}

/* Put radio on sleep. */
void radio_sleep(ProtoViewApp* app) {
    furi_assert(app);
    if(app->txrx->txrx_state == TxRxStateRx) {
        /* Stop the asynchronous receiving system before putting the
         * chip into sleep. */
        radio_rx_end(app);
    }
    furi_hal_subghz_sleep();
    app->txrx->txrx_state = TxRxStateSleep;
    furi_hal_power_suppress_charge_exit();
}

/* =============================== Transmission ============================= */

/* This function suspends the current RX state, switches to TX mode,
 * transmits the signal provided by the callback data_feeder, and later
 * restores the RX state if there was one. */
void radio_tx_signal(ProtoViewApp* app, FuriHalSubGhzAsyncTxCallback data_feeder, void* ctx) {
    TxRxState oldstate = app->txrx->txrx_state;

    if(oldstate == TxRxStateRx) radio_rx_end(app);
    radio_begin(app);

    furi_hal_subghz_idle();
    uint32_t value = furi_hal_subghz_set_frequency_and_path(app->frequency);
    FURI_LOG_E(TAG, "Switched to frequency: %lu", value);
    furi_hal_gpio_write(furi_hal_subghz.cc1101_g0_pin, false);
    furi_hal_gpio_init(
        furi_hal_subghz.cc1101_g0_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);

    furi_hal_subghz_start_async_tx(data_feeder, ctx);
    while(!furi_hal_subghz_is_async_tx_complete()) furi_delay_ms(10);
    furi_hal_subghz_stop_async_tx();
    furi_hal_subghz_idle();

    radio_begin(app);
    if(oldstate == TxRxStateRx) radio_rx(app);
}

/* ============================= Raw sampling mode =============================
 * This is a special mode that uses a high frequency timer to sample the
 * CC1101 pin directly. It's useful for debugging purposes when we want
 * to get the raw data from the chip and completely bypass the subghz
 * Flipper system.
 * ===========================================================================*/

void protoview_timer_isr(void* ctx) {
    ProtoViewApp* app = ctx;

    bool level = furi_hal_gpio_read(furi_hal_subghz.cc1101_g0_pin);
    if(app->txrx->last_g0_value != level) {
        uint32_t now = DWT->CYCCNT;
        uint32_t dur = now - app->txrx->last_g0_change_time;
        dur /= furi_hal_cortex_instructions_per_microsecond();
        if(dur > 15000) dur = 15000;
        raw_samples_add(RawSamples, app->txrx->last_g0_value, dur);
        app->txrx->last_g0_value = level;
        app->txrx->last_g0_change_time = now;
    }
    LL_TIM_ClearFlag_UPDATE(TIM2);
}

void raw_sampling_worker_start(ProtoViewApp* app) {
    UNUSED(app);

    furi_hal_bus_enable(FuriHalBusTIM2);

    LL_TIM_InitTypeDef tim_init = {
        .Prescaler = 63, /* CPU frequency is ~64Mhz. */
        .CounterMode = LL_TIM_COUNTERMODE_UP,
        .Autoreload = 5, /* Sample every 5 us */
    };

    LL_TIM_Init(TIM2, &tim_init);
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_SetCounter(TIM2, 0);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, protoview_timer_isr, app);
    LL_TIM_EnableIT_UPDATE(TIM2);
    LL_TIM_EnableCounter(TIM2);
    FURI_LOG_E(TAG, "Timer enabled");
}

void raw_sampling_worker_stop(ProtoViewApp* app) {
    UNUSED(app);
    FURI_CRITICAL_ENTER();
    LL_TIM_DisableCounter(TIM2);
    LL_TIM_DisableIT_UPDATE(TIM2);
    furi_hal_interrupt_set_isr(FuriHalInterruptIdTIM2, NULL, NULL);
    furi_hal_bus_disable(FuriHalBusTIM2);
    FURI_CRITICAL_EXIT();
}
