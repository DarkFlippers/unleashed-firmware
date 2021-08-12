#include "subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi-hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification-messages.h>
#include "file-worker.h"

void subghz_begin(FuriHalSubGhzPreset preset) {
    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_preset(preset);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
}

void subghz_rx(uint32_t frequency) {
    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
    furi_hal_subghz_flush_rx();
    furi_hal_subghz_rx();
}

void subghz_tx(uint32_t frequency) {
    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_cc1101_g0, true);
    furi_hal_subghz_tx();
}

void subghz_idle(void) {
    furi_hal_subghz_idle();
}

void subghz_end(void) {
    furi_hal_subghz_sleep();
}

bool subghz_key_load(SubGhz* subghz, const char* file_path) {
    furi_assert(subghz);
    furi_assert(file_path);

    FileWorker* file_worker = file_worker_alloc(false);
    // Load device data
    bool loaded = false;
    string_t path;
    string_init_set_str(path, file_path);
    string_t temp_str;
    string_init(temp_str);

    do {
        if(!file_worker_open(file_worker, string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }
        // Read and parse name protocol from 1st line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        // strlen("Protocol: ") = 10
        string_right(temp_str, 10);
        subghz->protocol_result =
            subghz_protocol_get_by_name(subghz->protocol, string_get_cstr(temp_str));
        if(subghz->protocol_result == NULL) {
            file_worker_show_error(file_worker, "Cannot parse\nfile");
            break;
        }
        if(!subghz->protocol_result->to_load_protocol(file_worker, subghz->protocol_result)) {
            file_worker_show_error(file_worker, "Cannot parse\nfile");
            break;
        }
        loaded = true;
    } while(0);

    string_clear(temp_str);
    string_clear(path);
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return loaded;
}
