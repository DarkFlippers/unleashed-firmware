#include "subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi-hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification-messages.h>
#include "file-worker.h"
#include "../notification/notification.h"
#include "views/subghz_receiver.h"

void subghz_begin(FuriHalSubGhzPreset preset) {
    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_preset(preset);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
}

uint32_t subghz_rx(void* context, uint32_t frequency) {
    furi_assert(context);
    SubGhzWorker* worker = context;

    furi_hal_subghz_idle();
    uint32_t value = furi_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
    furi_hal_subghz_flush_rx();
    furi_hal_subghz_rx();

    furi_hal_subghz_start_async_rx(subghz_worker_rx_callback, worker);
    subghz_worker_start(worker);
    return value;
}

uint32_t subghz_tx(uint32_t frequency) {
    furi_hal_subghz_idle();
    uint32_t value = furi_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_cc1101_g0, true);
    furi_hal_subghz_tx();
    return value;
}

void subghz_idle(void) {
    furi_hal_subghz_idle();
}

void subghz_rx_end(void* context) {
    furi_assert(context);
    SubGhzWorker* worker = context;

    if(subghz_worker_is_running(worker)) {
        subghz_worker_stop(worker);
        furi_hal_subghz_stop_async_rx();
    }
}

void subghz_sleep(void) {
    furi_hal_subghz_sleep();
}

void subghz_frequency_preset_to_str(void* context, string_t output) {
    furi_assert(context);
    SubGhz* subghz = context;
    string_cat_printf(
        output,
        "Frequency: %d\n"
        "Preset: %d\n",
        (int)subghz->frequency,
        (int)subghz->preset);
}

void subghz_transmitter_tx_start(void* context) {
    SubGhz* subghz = context;
    subghz->encoder = subghz_protocol_encoder_common_alloc();
    subghz->encoder->repeat = 200; //max repeat with the button held down
    //get upload
    if(subghz->protocol_result->get_upload_protocol) {
        if(subghz->protocol_result->get_upload_protocol(subghz->protocol_result, subghz->encoder)) {
            if(subghz->preset) {
                subghz_begin(subghz->preset);
            } else {
                subghz_begin(FuriHalSubGhzPresetOok650Async);
            }
            if(subghz->frequency) {
                subghz_tx(subghz->frequency);
            } else {
                subghz_tx(433920000);
            }

            //Start TX
            furi_hal_subghz_start_async_tx(subghz_protocol_encoder_common_yield, subghz->encoder);
        }
    }
}

void subghz_transmitter_tx_stop(void* context) {
    SubGhz* subghz = context;
    //Stop TX
    furi_hal_subghz_stop_async_tx();
    subghz_protocol_encoder_common_free(subghz->encoder);
    //if protocol dynamic then we save the last upload
    if(subghz->protocol_result->type_protocol == TYPE_PROTOCOL_DYNAMIC) {
        subghz_save_protocol_to_file(subghz, subghz->text_store);
    }
    notification_message(subghz->notifications, &sequence_reset_red);
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
    int res = 0;
    int data = 0;

    do {
        if(!file_worker_open(file_worker, string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }

        // Read and parse frequency from 1st line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        res = sscanf(string_get_cstr(temp_str), "Frequency: %d\n", &data);
        if(res != 1) {
            break;
        }
        subghz->frequency = (uint32_t)data;

        // Read and parse preset from 2st line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        res = sscanf(string_get_cstr(temp_str), "Preset: %d\n", &data);
        if(res != 1) {
            break;
        }
        subghz->preset = (FuriHalSubGhzPreset)data;

        // Read and parse name protocol from 2st line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        // strlen("Protocol: ") = 10
        string_right(temp_str, 10);
        subghz->protocol_result =
            subghz_protocol_get_by_name(subghz->protocol, string_get_cstr(temp_str));
        if(subghz->protocol_result == NULL) {
            break;
        }
        if(!subghz->protocol_result->to_load_protocol_from_file(
               file_worker, subghz->protocol_result)) {
            break;
        }
        loaded = true;
    } while(0);

    if(!loaded) {
        file_worker_show_error(file_worker, "Cannot parse\nfile");
    }
    string_clear(temp_str);
    string_clear(path);
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return loaded;
}

bool subghz_save_protocol_to_file(void* context, const char* dev_name) {
    SubGhz* subghz = context;
    furi_assert(subghz->protocol_result);
    FileWorker* file_worker = file_worker_alloc(false);
    string_t dev_file_name;
    string_init(dev_file_name);
    string_t temp_str;
    string_init(temp_str);
    bool saved = false;

    do {
        // Create subghz folder directory if necessary
        if(!file_worker_mkdir(file_worker, SUBGHZ_APP_FOLDER)) {
            break;
        }
        // Create saved directory if necessary
        if(!file_worker_mkdir(file_worker, SUBGHZ_APP_PATH_FOLDER)) {
            break;
        }
        // First remove subghz device file if it was saved
        string_printf(
            dev_file_name, "%s/%s%s", SUBGHZ_APP_PATH_FOLDER, dev_name, SUBGHZ_APP_EXTENSION);
        if(!file_worker_remove(file_worker, string_get_cstr(dev_file_name))) {
            break;
        }
        // Open file
        if(!file_worker_open(
               file_worker, string_get_cstr(dev_file_name), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            break;
        }
        //Get string frequency preset protocol
        subghz_frequency_preset_to_str(subghz, temp_str);
        if(!file_worker_write(file_worker, string_get_cstr(temp_str), string_size(temp_str))) {
            break;
        }
        //Get string save
        subghz->protocol_result->to_save_string(subghz->protocol_result, temp_str);
        // Prepare and write data to file
        if(!file_worker_write(file_worker, string_get_cstr(temp_str), string_size(temp_str))) {
            break;
        }
        saved = true;
    } while(0);

    string_clear(temp_str);
    string_clear(dev_file_name);
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return saved;
}

bool subghz_load_protocol_from_file(SubGhz* subghz) {
    furi_assert(subghz);

    FileWorker* file_worker = file_worker_alloc(false);
    string_t protocol_file_name;
    string_init(protocol_file_name);
    string_t temp_str;
    string_init(temp_str);
    int sscanf_res = 0;
    int data = 0;

    // Input events and views are managed by file_select
    bool res = file_worker_file_select(
        file_worker,
        SUBGHZ_APP_PATH_FOLDER,
        SUBGHZ_APP_EXTENSION,
        subghz->text_store,
        sizeof(subghz->text_store),
        NULL);

    if(res) {
        // Get key file path
        string_printf(
            protocol_file_name,
            "%s/%s%s",
            SUBGHZ_APP_PATH_FOLDER,
            subghz->text_store,
            SUBGHZ_APP_EXTENSION);
    } else {
        string_clear(temp_str);
        string_clear(protocol_file_name);

        file_worker_close(file_worker);
        file_worker_free(file_worker);
        return res;
    }
    res = false;
    do {
        if(!file_worker_open(
               file_worker, string_get_cstr(protocol_file_name), FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }
        // Read and parse frequency from 1st line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        sscanf_res = sscanf(string_get_cstr(temp_str), "Frequency: %d\n", &data);
        if(sscanf_res != 1) {
            break;
        }
        subghz->frequency = (uint32_t)data;

        // Read and parse preset from 2st line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        sscanf_res = sscanf(string_get_cstr(temp_str), "Preset: %d\n", &data);
        if(sscanf_res != 1) {
            break;
        }
        subghz->preset = (FuriHalSubGhzPreset)data;

        // Read and parse name protocol from 3st line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        // strlen("Protocol: ") = 10
        string_right(temp_str, 10);
        subghz->protocol_result =
            subghz_protocol_get_by_name(subghz->protocol, string_get_cstr(temp_str));
        if(subghz->protocol_result == NULL) {
            break;
        }
        if(!subghz->protocol_result->to_load_protocol_from_file(
               file_worker, subghz->protocol_result)) {
            break;
        }
        res = true;
    } while(0);

    if(!res) {
        file_worker_show_error(file_worker, "Cannot parse\nfile");
    }

    string_clear(temp_str);
    string_clear(protocol_file_name);

    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return res;
}

uint32_t subghz_random_serial(void) {
    static bool rand_generator_inited = false;

    if(!rand_generator_inited) {
        srand(DWT->CYCCNT);
        rand_generator_inited = true;
    }
    return (uint32_t)rand();
}
