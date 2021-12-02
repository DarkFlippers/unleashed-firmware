#include "subghz_i.h"

#include <math.h>
#include <furi.h>
#include <furi-hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification-messages.h>
#include <lib/flipper_file/flipper_file.h>
#include "../notification/notification.h"
#include "views/subghz_receiver.h"

bool subghz_set_pteset(SubGhz* subghz, const char* preset) {
    if(!strcmp(preset, "FuriHalSubGhzPresetOok270Async")) {
        subghz->txrx->preset = FuriHalSubGhzPresetOok270Async;
    } else if(!strcmp(preset, "FuriHalSubGhzPresetOok650Async")) {
        subghz->txrx->preset = FuriHalSubGhzPresetOok650Async;
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev238Async")) {
        subghz->txrx->preset = FuriHalSubGhzPreset2FSKDev238Async;
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev476Async")) {
        subghz->txrx->preset = FuriHalSubGhzPreset2FSKDev476Async;
    } else {
        FURI_LOG_E(SUBGHZ_PARSER_TAG, "Unknown preset");
        return false;
    }
    return true;
}

bool subghz_get_preset_name(SubGhz* subghz, string_t preset) {
    const char* preset_name;
    switch(subghz->txrx->preset) {
    case FuriHalSubGhzPresetOok270Async:
        preset_name = "FuriHalSubGhzPresetOok270Async";
        break;
    case FuriHalSubGhzPresetOok650Async:
        preset_name = "FuriHalSubGhzPresetOok650Async";
        break;
    case FuriHalSubGhzPreset2FSKDev238Async:
        preset_name = "FuriHalSubGhzPreset2FSKDev238Async";
        break;
    case FuriHalSubGhzPreset2FSKDev476Async:
        preset_name = "FuriHalSubGhzPreset2FSKDev476Async";
        break;
        FURI_LOG_E(SUBGHZ_PARSER_TAG, "Unknown preset");
    default:
        return false;
        break;
    }
    string_set(preset, preset_name);
    return true;
}

void subghz_get_frequency_modulation(SubGhz* subghz, string_t frequency, string_t modulation) {
    furi_assert(subghz);
    if(frequency != NULL) {
        string_printf(
            frequency,
            "%03ld.%02ld",
            subghz->txrx->frequency / 1000000 % 1000,
            subghz->txrx->frequency / 10000 % 100);
    }

    if(modulation != NULL) {
        if(subghz->txrx->preset == FuriHalSubGhzPresetOok650Async ||
           subghz->txrx->preset == FuriHalSubGhzPresetOok270Async) {
            string_set(modulation, "AM");
        } else if(
            subghz->txrx->preset == FuriHalSubGhzPreset2FSKDev238Async ||
            subghz->txrx->preset == FuriHalSubGhzPreset2FSKDev476Async) {
            string_set(modulation, "FM");
        } else {
            furi_crash(NULL);
        }
    }
}

void subghz_begin(SubGhz* subghz, FuriHalSubGhzPreset preset) {
    furi_assert(subghz);
    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_preset(preset);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
    subghz->txrx->txrx_state = SubGhzTxRxStateIDLE;
}

uint32_t subghz_rx(SubGhz* subghz, uint32_t frequency) {
    furi_assert(subghz);
    if(!furi_hal_subghz_is_frequency_valid(frequency)) {
        furi_crash(NULL);
    }
    furi_assert(
        subghz->txrx->txrx_state != SubGhzTxRxStateRx &&
        subghz->txrx->txrx_state != SubGhzTxRxStateSleep);

    furi_hal_subghz_idle();
    uint32_t value = furi_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
    furi_hal_subghz_flush_rx();
    furi_hal_subghz_rx();

    furi_hal_subghz_start_async_rx(subghz_worker_rx_callback, subghz->txrx->worker);
    subghz_worker_start(subghz->txrx->worker);
    subghz->txrx->txrx_state = SubGhzTxRxStateRx;
    return value;
}

static bool subghz_tx(SubGhz* subghz, uint32_t frequency) {
    furi_assert(subghz);
    if(!furi_hal_subghz_is_frequency_valid(frequency)) {
        furi_crash(NULL);
    }
    furi_assert(subghz->txrx->txrx_state != SubGhzTxRxStateSleep);
    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency_and_path(frequency);
    hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(&gpio_cc1101_g0, true);
    bool ret = furi_hal_subghz_tx();
    subghz->txrx->txrx_state = SubGhzTxRxStateTx;
    return ret;
}

void subghz_idle(SubGhz* subghz) {
    furi_assert(subghz);
    furi_assert(subghz->txrx->txrx_state != SubGhzTxRxStateSleep);
    furi_hal_subghz_idle();
    subghz->txrx->txrx_state = SubGhzTxRxStateIDLE;
}

void subghz_rx_end(SubGhz* subghz) {
    furi_assert(subghz);
    furi_assert(subghz->txrx->txrx_state == SubGhzTxRxStateRx);
    if(subghz_worker_is_running(subghz->txrx->worker)) {
        subghz_worker_stop(subghz->txrx->worker);
        furi_hal_subghz_stop_async_rx();
    }
    furi_hal_subghz_idle();
    subghz->txrx->txrx_state = SubGhzTxRxStateIDLE;
}

void subghz_sleep(SubGhz* subghz) {
    furi_assert(subghz);
    furi_hal_subghz_sleep();
    subghz->txrx->txrx_state = SubGhzTxRxStateSleep;
}

bool subghz_tx_start(SubGhz* subghz) {
    furi_assert(subghz);

    bool ret = false;
    subghz->txrx->encoder = subghz_protocol_encoder_common_alloc();
    subghz->txrx->encoder->repeat = 200; //max repeat with the button held down
    //get upload
    if(subghz->txrx->protocol_result->get_upload_protocol) {
        if(subghz->txrx->protocol_result->get_upload_protocol(
               subghz->txrx->protocol_result, subghz->txrx->encoder)) {
            if(subghz->txrx->preset) {
                subghz_begin(subghz, subghz->txrx->preset);
            } else {
                subghz_begin(subghz, FuriHalSubGhzPresetOok270Async);
            }
            if(subghz->txrx->frequency) {
                ret = subghz_tx(subghz, subghz->txrx->frequency);
            } else {
                ret = subghz_tx(subghz, 433920000);
            }

            if(ret) {
                //Start TX
                furi_hal_subghz_start_async_tx(
                    subghz_protocol_encoder_common_yield, subghz->txrx->encoder);
            }
        }
    }
    if(!ret) {
        subghz_protocol_encoder_common_free(subghz->txrx->encoder);
        subghz_idle(subghz);
    }
    return ret;
}

void subghz_tx_stop(SubGhz* subghz) {
    furi_assert(subghz);
    furi_assert(subghz->txrx->txrx_state == SubGhzTxRxStateTx);
    //Stop TX
    furi_hal_subghz_stop_async_tx();
    subghz_protocol_encoder_common_free(subghz->txrx->encoder);
    subghz_idle(subghz);
    //if protocol dynamic then we save the last upload
    if((subghz->txrx->protocol_result->type_protocol == SubGhzProtocolCommonTypeDynamic) &&
       (strcmp(subghz->file_name, ""))) {
        subghz_save_protocol_to_file(subghz, subghz->file_name);
    }
    notification_message(subghz->notifications, &sequence_reset_red);
}

bool subghz_key_load(SubGhz* subghz, const char* file_path) {
    furi_assert(subghz);
    furi_assert(file_path);

    Storage* storage = furi_record_open("storage");
    FlipperFile* flipper_file = flipper_file_alloc(storage);

    // Load device data
    bool loaded = false;
    string_t path;
    string_init_set_str(path, file_path);
    string_t temp_str;
    string_init(temp_str);
    uint32_t version;

    do {
        if(!flipper_file_open_existing(flipper_file, string_get_cstr(path))) {
            FURI_LOG_E(
                SUBGHZ_PARSER_TAG, "Unable to open file for read: %s", string_get_cstr(path));
            break;
        }
        if(!flipper_file_read_header(flipper_file, temp_str, &version)) {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "Missing or incorrect header");
            break;
        }

        if(((!strcmp(string_get_cstr(temp_str), SUBGHZ_KEY_FILE_TYPE)) ||
            (!strcmp(string_get_cstr(temp_str), SUBGHZ_RAW_FILE_TYPE))) &&
           version == SUBGHZ_KEY_FILE_VERSION) {
        } else {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "Type or version mismatch");
            break;
        }

        if(!flipper_file_read_uint32(
               flipper_file, "Frequency", (uint32_t*)&subghz->txrx->frequency, 1)) {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "Missing Frequency");
            break;
        }

        if(!flipper_file_read_string(flipper_file, "Preset", temp_str)) {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "Missing Preset");
            break;
        }
        if(!subghz_set_pteset(subghz, string_get_cstr(temp_str))) {
            break;
        }

        if(!flipper_file_read_string(flipper_file, "Protocol", temp_str)) {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "Missing Protocol");
            break;
        }

        subghz->txrx->protocol_result =
            subghz_parser_get_by_name(subghz->txrx->parser, string_get_cstr(temp_str));
        if(subghz->txrx->protocol_result == NULL) {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "This type of protocol was not found");
            break;
        }
        if(!subghz->txrx->protocol_result->to_load_protocol_from_file(
               flipper_file, subghz->txrx->protocol_result, string_get_cstr(path))) {
            break;
        }
        loaded = true;
    } while(0);

    if(!loaded) {
        dialog_message_show_storage_error(subghz->dialogs, "Cannot parse\nfile");
    }
    string_clear(temp_str);
    string_clear(path);

    flipper_file_close(flipper_file);
    flipper_file_free(flipper_file);

    furi_record_close("storage");

    return loaded;
}

bool subghz_get_next_name_file(SubGhz* subghz) {
    furi_assert(subghz);

    Storage* storage = furi_record_open("storage");
    string_t temp_str;
    string_init(temp_str);
    bool res = false;

    if(strcmp(subghz->file_name, "")) {
        //get the name of the next free file
        storage_get_next_filename(
            storage, SUBGHZ_RAW_PATH_FOLDER, subghz->file_name, SUBGHZ_APP_EXTENSION, temp_str);

        memcpy(subghz->file_name, string_get_cstr(temp_str), strlen(string_get_cstr(temp_str)));
        res = true;
    }

    string_clear(temp_str);
    furi_record_close("storage");

    return res;
}

bool subghz_save_protocol_to_file(SubGhz* subghz, const char* dev_name) {
    furi_assert(subghz);
    furi_assert(subghz->txrx->protocol_result);

    Storage* storage = furi_record_open("storage");
    FlipperFile* flipper_file = flipper_file_alloc(storage);
    string_t dev_file_name;
    string_init(dev_file_name);
    string_t temp_str;
    string_init(temp_str);
    bool saved = false;

    do {
        // Checking that this type of people can be saved
        if(subghz->txrx->protocol_result->to_save_file == NULL) {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "No saving of this type of keys");
            break;
        }
        // Create subghz folder directory if necessary
        if(!storage_simply_mkdir(storage, SUBGHZ_APP_FOLDER)) {
            dialog_message_show_storage_error(subghz->dialogs, "Cannot create\nfolder");
            break;
        }
        // Create saved directory if necessary
        if(!storage_simply_mkdir(storage, SUBGHZ_APP_PATH_FOLDER)) {
            dialog_message_show_storage_error(subghz->dialogs, "Cannot create\nfolder");
            break;
        }

        // First remove subghz device file if it was saved
        string_printf(
            dev_file_name, "%s/%s%s", SUBGHZ_APP_PATH_FOLDER, dev_name, SUBGHZ_APP_EXTENSION);

        if(!storage_simply_remove(storage, string_get_cstr(dev_file_name))) {
            break;
        }

        // Open file
        if(!flipper_file_open_always(flipper_file, string_get_cstr(dev_file_name))) {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "Unable to open file for write: %s", dev_file_name);
            break;
        }

        if(!flipper_file_write_header_cstr(
               flipper_file, SUBGHZ_KEY_FILE_TYPE, SUBGHZ_KEY_FILE_VERSION)) {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "Unable to add header");
            break;
        }

        if(!flipper_file_write_uint32(flipper_file, "Frequency", &subghz->txrx->frequency, 1)) {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "Unable to add Frequency");
            break;
        }

        if(!subghz_get_preset_name(subghz, temp_str)) {
            break;
        }
        if(!flipper_file_write_string_cstr(flipper_file, "Preset", string_get_cstr(temp_str))) {
            FURI_LOG_E(SUBGHZ_PARSER_TAG, "Unable to add Preset");
            break;
        }

        if(!subghz->txrx->protocol_result->to_save_file(
               subghz->txrx->protocol_result, flipper_file)) {
            break;
        }

        saved = true;
    } while(0);

    string_clear(temp_str);
    string_clear(dev_file_name);

    flipper_file_close(flipper_file);
    flipper_file_free(flipper_file);

    furi_record_close("storage");

    return saved;
}

bool subghz_load_protocol_from_file(SubGhz* subghz) {
    furi_assert(subghz);

    string_t file_name;
    string_init(file_name);

    // Input events and views are managed by file_select
    bool res = dialog_file_select_show(
        subghz->dialogs,
        SUBGHZ_APP_PATH_FOLDER,
        SUBGHZ_APP_EXTENSION,
        subghz->file_name,
        sizeof(subghz->file_name),
        NULL);

    if(res) {
        string_printf(
            file_name, "%s/%s%s", SUBGHZ_APP_PATH_FOLDER, subghz->file_name, SUBGHZ_APP_EXTENSION);

        res = subghz_key_load(subghz, string_get_cstr(file_name));
    }

    string_clear(file_name);

    return res;
}

bool subghz_rename_file(SubGhz* subghz) {
    furi_assert(subghz);
    bool ret = true;
    string_t old_path;
    string_t new_path;

    Storage* storage = furi_record_open("storage");

    string_init_printf(
        old_path, "%s/%s%s", SUBGHZ_APP_PATH_FOLDER, subghz->file_name_tmp, SUBGHZ_APP_EXTENSION);

    string_init_printf(
        new_path, "%s/%s%s", SUBGHZ_APP_PATH_FOLDER, subghz->file_name, SUBGHZ_APP_EXTENSION);

    FS_Error fs_result =
        storage_common_rename(storage, string_get_cstr(old_path), string_get_cstr(new_path));

    if(fs_result != FSE_OK && fs_result != FSE_EXIST) {
        dialog_message_show_storage_error(subghz->dialogs, "Cannot rename\n file/directory");
        ret = false;
    }

    string_clear(old_path);
    string_clear(new_path);
    furi_record_close("storage");

    return ret;
}

bool subghz_delete_file(SubGhz* subghz) {
    furi_assert(subghz);

    Storage* storage = furi_record_open("storage");
    string_t file_path;
    string_init_printf(
        file_path, "%s/%s%s", SUBGHZ_APP_PATH_FOLDER, subghz->file_name_tmp, SUBGHZ_APP_EXTENSION);
    bool result = storage_simply_remove(storage, string_get_cstr(file_path));
    furi_record_close("storage");

    subghz_file_name_clear(subghz);

    return result;
}

void subghz_file_name_clear(SubGhz* subghz) {
    furi_assert(subghz);
    memset(subghz->file_name, 0, sizeof(subghz->file_name));
    memset(subghz->file_name_tmp, 0, sizeof(subghz->file_name_tmp));
}

uint32_t subghz_random_serial(void) {
    static bool rand_generator_inited = false;

    if(!rand_generator_inited) {
        srand(DWT->CYCCNT);
        rand_generator_inited = true;
    }
    return (uint32_t)rand();
}

void subghz_hopper_update(SubGhz* subghz) {
    furi_assert(subghz);

    switch(subghz->txrx->hopper_state) {
    case SubGhzHopperStateOFF:
        return;
        break;
    case SubGhzHopperStatePause:
        return;
        break;
    case SubGhzHopperStateRSSITimeOut:
        if(subghz->txrx->hopper_timeout != 0) {
            subghz->txrx->hopper_timeout--;
            return;
        }
        break;
    default:
        break;
    }
    float rssi = -127.0f;
    if(subghz->txrx->hopper_state != SubGhzHopperStateRSSITimeOut) {
        // See RSSI Calculation timings in CC1101 17.3 RSSI
        rssi = furi_hal_subghz_get_rssi();

        // Stay if RSSI is high enough
        if(rssi > -90.0f) {
            subghz->txrx->hopper_timeout = 10;
            subghz->txrx->hopper_state = SubGhzHopperStateRSSITimeOut;
            return;
        }
    } else {
        subghz->txrx->hopper_state = SubGhzHopperStateRunnig;
    }

    // Select next frequency
    if(subghz->txrx->hopper_idx_frequency < subghz_hopper_frequencies_count - 1) {
        subghz->txrx->hopper_idx_frequency++;
    } else {
        subghz->txrx->hopper_idx_frequency = 0;
    }

    if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
        subghz_rx_end(subghz);
    };
    if(subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) {
        subghz_parser_reset(subghz->txrx->parser);
        subghz->txrx->frequency = subghz_hopper_frequencies[subghz->txrx->hopper_idx_frequency];
        subghz_rx(subghz, subghz->txrx->frequency);
    }
}
