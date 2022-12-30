#include "weather_station_app_i.h"

#define TAG "WeatherStation"
#include <flipper_format/flipper_format_i.h>

void ws_preset_init(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint8_t* preset_data,
    size_t preset_data_size) {
    furi_assert(context);
    WeatherStationApp* app = context;
    furi_string_set(app->txrx->preset->name, preset_name);
    app->txrx->preset->frequency = frequency;
    app->txrx->preset->data = preset_data;
    app->txrx->preset->data_size = preset_data_size;
}

bool ws_set_preset(WeatherStationApp* app, const char* preset) {
    if(!strcmp(preset, "FuriHalSubGhzPresetOok270Async")) {
        furi_string_set(app->txrx->preset->name, "AM270");
    } else if(!strcmp(preset, "FuriHalSubGhzPresetOok650Async")) {
        furi_string_set(app->txrx->preset->name, "AM650");
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev238Async")) {
        furi_string_set(app->txrx->preset->name, "FM238");
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev476Async")) {
        furi_string_set(app->txrx->preset->name, "FM476");
    } else if(!strcmp(preset, "FuriHalSubGhzPresetCustom")) {
        furi_string_set(app->txrx->preset->name, "CUSTOM");
    } else {
        FURI_LOG_E(TAG, "Unknown preset");
        return false;
    }
    return true;
}

void ws_get_frequency_modulation(
    WeatherStationApp* app,
    FuriString* frequency,
    FuriString* modulation) {
    furi_assert(app);
    if(frequency != NULL) {
        furi_string_printf(
            frequency,
            "%03ld.%02ld",
            app->txrx->preset->frequency / 1000000 % 1000,
            app->txrx->preset->frequency / 10000 % 100);
    }
    if(modulation != NULL) {
        furi_string_printf(modulation, "%.2s", furi_string_get_cstr(app->txrx->preset->name));
    }
}

void ws_begin(WeatherStationApp* app, uint8_t* preset_data) {
    furi_assert(app);
    UNUSED(preset_data);
    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_custom_preset(preset_data);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
    app->txrx->txrx_state = WSTxRxStateIDLE;
}

uint32_t ws_rx(WeatherStationApp* app, uint32_t frequency) {
    furi_assert(app);
    if(!furi_hal_subghz_is_frequency_valid(frequency)) {
        furi_crash("WeatherStation: Incorrect RX frequency.");
    }
    furi_assert(
        app->txrx->txrx_state != WSTxRxStateRx && app->txrx->txrx_state != WSTxRxStateSleep);

    furi_hal_subghz_idle();
    uint32_t value = furi_hal_subghz_set_frequency_and_path(frequency);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
    furi_hal_subghz_flush_rx();
    furi_hal_subghz_rx();

    furi_hal_subghz_start_async_rx(subghz_worker_rx_callback, app->txrx->worker);
    subghz_worker_start(app->txrx->worker);
    app->txrx->txrx_state = WSTxRxStateRx;
    return value;
}

void ws_idle(WeatherStationApp* app) {
    furi_assert(app);
    furi_assert(app->txrx->txrx_state != WSTxRxStateSleep);
    furi_hal_subghz_idle();
    app->txrx->txrx_state = WSTxRxStateIDLE;
}

void ws_rx_end(WeatherStationApp* app) {
    furi_assert(app);
    furi_assert(app->txrx->txrx_state == WSTxRxStateRx);
    if(subghz_worker_is_running(app->txrx->worker)) {
        subghz_worker_stop(app->txrx->worker);
        furi_hal_subghz_stop_async_rx();
    }
    furi_hal_subghz_idle();
    app->txrx->txrx_state = WSTxRxStateIDLE;
}

void ws_sleep(WeatherStationApp* app) {
    furi_assert(app);
    furi_hal_subghz_sleep();
    app->txrx->txrx_state = WSTxRxStateSleep;
}

void ws_hopper_update(WeatherStationApp* app) {
    furi_assert(app);

    switch(app->txrx->hopper_state) {
    case WSHopperStateOFF:
    case WSHopperStatePause:
        return;
    case WSHopperStateRSSITimeOut:
        if(app->txrx->hopper_timeout != 0) {
            app->txrx->hopper_timeout--;
            return;
        }
        break;
    default:
        break;
    }
    float rssi = -127.0f;
    if(app->txrx->hopper_state != WSHopperStateRSSITimeOut) {
        // See RSSI Calculation timings in CC1101 17.3 RSSI
        rssi = furi_hal_subghz_get_rssi();

        // Stay if RSSI is high enough
        if(rssi > -90.0f) {
            app->txrx->hopper_timeout = 10;
            app->txrx->hopper_state = WSHopperStateRSSITimeOut;
            return;
        }
    } else {
        app->txrx->hopper_state = WSHopperStateRunnig;
    }
    // Select next frequency
    if(app->txrx->hopper_idx_frequency <
       subghz_setting_get_hopper_frequency_count(app->setting) - 1) {
        app->txrx->hopper_idx_frequency++;
    } else {
        app->txrx->hopper_idx_frequency = 0;
    }

    if(app->txrx->txrx_state == WSTxRxStateRx) {
        ws_rx_end(app);
    };
    if(app->txrx->txrx_state == WSTxRxStateIDLE) {
        subghz_receiver_reset(app->txrx->receiver);
        app->txrx->preset->frequency =
            subghz_setting_get_hopper_frequency(app->setting, app->txrx->hopper_idx_frequency);
        ws_rx(app, app->txrx->preset->frequency);
    }
}
