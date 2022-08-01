#include "subghz_i.h"

#include "assets_icons.h"
#include "subghz/types.h"
#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification_messages.h>
#include <flipper_format/flipper_format.h>
#include "../notification/notification.h"
#include "views/receiver.h"

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/subghz/protocols/raw.h>

#define TAG "SubGhz"

void subghz_preset_init(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint8_t* preset_data,
    size_t preset_data_size) {
    furi_assert(context);
    SubGhz* subghz = context;
    string_set(subghz->txrx->preset->name, preset_name);
    subghz->txrx->preset->frequency = frequency;
    subghz->txrx->preset->data = preset_data;
    subghz->txrx->preset->data_size = preset_data_size;
}

bool subghz_set_preset(SubGhz* subghz, const char* preset) {
    if(!strcmp(preset, "FuriHalSubGhzPresetOok270Async")) {
        string_set(subghz->txrx->preset->name, "AM270");
    } else if(!strcmp(preset, "FuriHalSubGhzPresetOok650Async")) {
        string_set(subghz->txrx->preset->name, "AM650");
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev238Async")) {
        string_set(subghz->txrx->preset->name, "FM238");
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev476Async")) {
        string_set(subghz->txrx->preset->name, "FM476");
    } else if(!strcmp(preset, "FuriHalSubGhzPresetCustom")) {
        string_set(subghz->txrx->preset->name, "CUSTOM");
    } else {
        FURI_LOG_E(TAG, "Unknown preset");
        return false;
    }
    return true;
}

void subghz_get_frequency_modulation(SubGhz* subghz, string_t frequency, string_t modulation) {
    furi_assert(subghz);
    if(frequency != NULL) {
        string_printf(
            frequency,
            "%03ld.%02ld",
            subghz->txrx->preset->frequency / 1000000 % 1000,
            subghz->txrx->preset->frequency / 10000 % 100);
    }
    if(modulation != NULL) {
        string_printf(modulation, "%0.2s", string_get_cstr(subghz->txrx->preset->name));
    }
}

void subghz_begin(SubGhz* subghz, uint8_t* preset_data) {
    furi_assert(subghz);
    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_custom_preset(preset_data);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
    subghz->txrx->txrx_state = SubGhzTxRxStateIDLE;
}

uint32_t subghz_rx(SubGhz* subghz, uint32_t frequency) {
    furi_assert(subghz);
    if(!furi_hal_subghz_is_frequency_valid(frequency)) {
        furi_crash("SubGhz: Incorrect RX frequency.");
    }
    furi_assert(
        subghz->txrx->txrx_state != SubGhzTxRxStateRx &&
        subghz->txrx->txrx_state != SubGhzTxRxStateSleep);

    furi_hal_subghz_idle();
    uint32_t value = furi_hal_subghz_set_frequency_and_path(frequency);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeInput, GpioPullNo, GpioSpeedLow);
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
        furi_crash("SubGhz: Incorrect TX frequency.");
    }
    furi_assert(subghz->txrx->txrx_state != SubGhzTxRxStateSleep);
    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency_and_path(frequency);
    furi_hal_gpio_init(&gpio_cc1101_g0, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(&gpio_cc1101_g0, true);
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

bool subghz_tx_start(SubGhz* subghz, FlipperFormat* flipper_format) {
    furi_assert(subghz);

    bool ret = false;
    string_t temp_str;
    string_init(temp_str);
    uint32_t repeat = 200;
    do {
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        if(!flipper_format_read_string(flipper_format, "Protocol", temp_str)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            break;
        }
        //ToDo FIX
        if(!flipper_format_insert_or_update_uint32(flipper_format, "Repeat", &repeat, 1)) {
            FURI_LOG_E(TAG, "Unable Repeat");
            break;
        }

        subghz->txrx->transmitter =
            subghz_transmitter_alloc_init(subghz->txrx->environment, string_get_cstr(temp_str));

        if(subghz->txrx->transmitter) {
            if(subghz_transmitter_deserialize(subghz->txrx->transmitter, flipper_format)) {
                if(strcmp(string_get_cstr(subghz->txrx->preset->name), "")) {
                    subghz_begin(
                        subghz,
                        subghz_setting_get_preset_data_by_name(
                            subghz->setting, string_get_cstr(subghz->txrx->preset->name)));
                } else {
                    FURI_LOG_E(
                        TAG,
                        "Unknown name preset \" %s \"",
                        string_get_cstr(subghz->txrx->preset->name));
                    subghz_begin(
                        subghz, subghz_setting_get_preset_data_by_name(subghz->setting, "AM650"));
                }
                if(subghz->txrx->preset->frequency) {
                    ret = subghz_tx(subghz, subghz->txrx->preset->frequency);
                } else {
                    ret = subghz_tx(subghz, 433920000);
                }
                if(ret) {
                    //Start TX
                    furi_hal_subghz_start_async_tx(
                        subghz_transmitter_yield, subghz->txrx->transmitter);
                }
            }
        }
        if(!ret) {
            subghz_transmitter_free(subghz->txrx->transmitter);
            if(subghz->txrx->txrx_state != SubGhzTxRxStateSleep) {
                subghz_idle(subghz);
            }
        }

    } while(false);
    string_clear(temp_str);
    return ret;
}

void subghz_tx_stop(SubGhz* subghz) {
    furi_assert(subghz);
    furi_assert(subghz->txrx->txrx_state == SubGhzTxRxStateTx);
    //Stop TX
    furi_hal_subghz_stop_async_tx();
    subghz_transmitter_stop(subghz->txrx->transmitter);
    subghz_transmitter_free(subghz->txrx->transmitter);

    //if protocol dynamic then we save the last upload
    if((subghz->txrx->decoder_result->protocol->type == SubGhzProtocolTypeDynamic) &&
       (subghz_path_is_file(subghz->file_path))) {
        subghz_save_protocol_to_file(
            subghz, subghz->txrx->fff_data, string_get_cstr(subghz->file_path));
    }
    subghz_idle(subghz);
    notification_message(subghz->notifications, &sequence_reset_red);
}

void subghz_dialog_message_show_only_rx(SubGhz* subghz) {
    DialogsApp* dialogs = subghz->dialogs;
    DialogMessage* message = dialog_message_alloc();

    dialog_message_set_header(message, "Transmission is blocked", 63, 3, AlignCenter, AlignTop);

    dialog_message_set_text(
        message,
        "This frequency\nis restricted to\nreceiving only\nin your region.",
        3,
        17,
        AlignLeft,
        AlignTop);

    dialog_message_set_icon(message, &I_DolphinFirstStart8_56x51, 72, 14);

    dialog_message_show(dialogs, message);
    dialog_message_free(message);
}

bool subghz_key_load(SubGhz* subghz, const char* file_path, bool show_dialog) {
    furi_assert(subghz);
    furi_assert(file_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    Stream* fff_data_stream = flipper_format_get_raw_stream(subghz->txrx->fff_data);

    SubGhzLoadKeyState load_key_state = SubGhzLoadKeyStateParseErr;
    string_t temp_str;
    string_init(temp_str);
    uint32_t temp_data32;

    do {
        stream_clean(fff_data_stream);
        if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
            FURI_LOG_E(TAG, "Error open file %s", file_path);
            break;
        }

        if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
            FURI_LOG_E(TAG, "Missing or incorrect header");
            break;
        }

        if(((!strcmp(string_get_cstr(temp_str), SUBGHZ_KEY_FILE_TYPE)) ||
            (!strcmp(string_get_cstr(temp_str), SUBGHZ_RAW_FILE_TYPE))) &&
           temp_data32 == SUBGHZ_KEY_FILE_VERSION) {
        } else {
            FURI_LOG_E(TAG, "Type or version mismatch");
            break;
        }

        if(!flipper_format_read_uint32(fff_data_file, "Frequency", &temp_data32, 1)) {
            FURI_LOG_E(TAG, "Missing Frequency");
            break;
        }

        if(!furi_hal_subghz_is_frequency_valid(temp_data32)) {
            FURI_LOG_E(TAG, "Frequency not supported");
            break;
        }

        if(!furi_hal_subghz_is_tx_allowed(temp_data32)) {
            FURI_LOG_E(TAG, "This frequency can only be used for RX in your region");
            load_key_state = SubGhzLoadKeyStateOnlyRx;
            break;
        }
        subghz->txrx->preset->frequency = temp_data32;

        if(!flipper_format_read_string(fff_data_file, "Preset", temp_str)) {
            FURI_LOG_E(TAG, "Missing Preset");
            break;
        }

        if(!subghz_set_preset(subghz, string_get_cstr(temp_str))) {
            break;
        }

        if(!strcmp(string_get_cstr(temp_str), "FuriHalSubGhzPresetCustom")) {
            //Todo add Custom_preset_module
            //delete peset if it already exists
            subghz_setting_delete_custom_preset(
                subghz->setting, string_get_cstr(subghz->txrx->preset->name));
            //load custom preset from file
            if(!subghz_setting_load_custom_preset(
                   subghz->setting, string_get_cstr(subghz->txrx->preset->name), fff_data_file)) {
                FURI_LOG_E(TAG, "Missing Custom preset");
                break;
            }
        }
        size_t preset_index = subghz_setting_get_inx_preset_by_name(
            subghz->setting, string_get_cstr(subghz->txrx->preset->name));
        subghz_preset_init(
            subghz,
            string_get_cstr(subghz->txrx->preset->name),
            subghz->txrx->preset->frequency,
            subghz_setting_get_preset_data(subghz->setting, preset_index),
            subghz_setting_get_preset_data_size(subghz->setting, preset_index));

        if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            break;
        }
        if(!strcmp(string_get_cstr(temp_str), "RAW")) {
            //if RAW
            subghz_protocol_raw_gen_fff_data(subghz->txrx->fff_data, file_path);
        } else {
            stream_copy_full(
                flipper_format_get_raw_stream(fff_data_file),
                flipper_format_get_raw_stream(subghz->txrx->fff_data));
        }

        subghz->txrx->decoder_result = subghz_receiver_search_decoder_base_by_name(
            subghz->txrx->receiver, string_get_cstr(temp_str));
        if(subghz->txrx->decoder_result) {
            if(!subghz_protocol_decoder_base_deserialize(
                   subghz->txrx->decoder_result, subghz->txrx->fff_data)) {
                break;
            }
        } else {
            FURI_LOG_E(TAG, "Protocol not found");
            break;
        }

        load_key_state = SubGhzLoadKeyStateOK;
    } while(0);

    string_clear(temp_str);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);

    switch(load_key_state) {
    case SubGhzLoadKeyStateParseErr:
        if(show_dialog) {
            dialog_message_show_storage_error(subghz->dialogs, "Cannot parse\nfile");
        }
        return false;

    case SubGhzLoadKeyStateOnlyRx:
        if(show_dialog) {
            subghz_dialog_message_show_only_rx(subghz);
        }
        return false;

    case SubGhzLoadKeyStateOK:
        return true;

    default:
        furi_crash("SubGhz: Unknown load_key_state.");
        return false;
    }
}

bool subghz_get_next_name_file(SubGhz* subghz, uint8_t max_len) {
    furi_assert(subghz);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    string_t temp_str;
    string_t file_name;
    string_t file_path;

    string_init(temp_str);
    string_init(file_name);
    string_init(file_path);

    bool res = false;

    if(subghz_path_is_file(subghz->file_path)) {
        //get the name of the next free file
        path_extract_filename(subghz->file_path, file_name, true);
        path_extract_dirname(string_get_cstr(subghz->file_path), file_path);

        storage_get_next_filename(
            storage,
            string_get_cstr(file_path),
            string_get_cstr(file_name),
            SUBGHZ_APP_EXTENSION,
            file_name,
            max_len);

        string_printf(
            temp_str,
            "%s/%s%s",
            string_get_cstr(file_path),
            string_get_cstr(file_name),
            SUBGHZ_APP_EXTENSION);
        string_set(subghz->file_path, temp_str);
        res = true;
    }

    string_clear(temp_str);
    string_clear(file_path);
    string_clear(file_name);
    furi_record_close(RECORD_STORAGE);

    return res;
}

bool subghz_save_protocol_to_file(
    SubGhz* subghz,
    FlipperFormat* flipper_format,
    const char* dev_file_name) {
    furi_assert(subghz);
    furi_assert(flipper_format);
    furi_assert(dev_file_name);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* flipper_format_stream = flipper_format_get_raw_stream(flipper_format);

    bool saved = false;
    string_t file_dir;
    string_init(file_dir);

    path_extract_dirname(dev_file_name, file_dir);
    do {
        //removing additional fields
        flipper_format_delete_key(flipper_format, "Repeat");
        flipper_format_delete_key(flipper_format, "Manufacture");

        // Create subghz folder directory if necessary
        if(!storage_simply_mkdir(storage, string_get_cstr(file_dir))) {
            dialog_message_show_storage_error(subghz->dialogs, "Cannot create\nfolder");
            break;
        }

        if(!storage_simply_remove(storage, dev_file_name)) {
            break;
        }
        //ToDo check Write
        stream_seek(flipper_format_stream, 0, StreamOffsetFromStart);
        stream_save_to_file(flipper_format_stream, storage, dev_file_name, FSOM_CREATE_ALWAYS);

        saved = true;
    } while(0);
    string_clear(file_dir);
    furi_record_close(RECORD_STORAGE);
    return saved;
}

bool subghz_load_protocol_from_file(SubGhz* subghz) {
    furi_assert(subghz);

    string_t file_path;
    string_init(file_path);

    // Input events and views are managed by file_select
    bool res = dialog_file_browser_show(
        subghz->dialogs,
        subghz->file_path,
        subghz->file_path,
        SUBGHZ_APP_EXTENSION,
        true,
        &I_sub1_10px,
        true);

    if(res) {
        res = subghz_key_load(subghz, string_get_cstr(subghz->file_path), true);
    }

    string_clear(file_path);

    return res;
}

bool subghz_rename_file(SubGhz* subghz) {
    furi_assert(subghz);
    bool ret = true;

    Storage* storage = furi_record_open(RECORD_STORAGE);

    if(string_cmp(subghz->file_path_tmp, subghz->file_path)) {
        FS_Error fs_result = storage_common_rename(
            storage, string_get_cstr(subghz->file_path_tmp), string_get_cstr(subghz->file_path));

        if(fs_result != FSE_OK) {
            dialog_message_show_storage_error(subghz->dialogs, "Cannot rename\n file/directory");
            ret = false;
        }
    }
    furi_record_close(RECORD_STORAGE);

    return ret;
}

bool subghz_delete_file(SubGhz* subghz) {
    furi_assert(subghz);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = storage_simply_remove(storage, string_get_cstr(subghz->file_path_tmp));
    furi_record_close(RECORD_STORAGE);

    subghz_file_name_clear(subghz);

    return result;
}

void subghz_file_name_clear(SubGhz* subghz) {
    furi_assert(subghz);
    string_set_str(subghz->file_path, SUBGHZ_APP_FOLDER);
    string_reset(subghz->file_path_tmp);
}

bool subghz_path_is_file(string_t path) {
    return string_end_with_str_p(path, SUBGHZ_APP_EXTENSION);
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
    if(subghz->txrx->hopper_idx_frequency <
       subghz_setting_get_hopper_frequency_count(subghz->setting) - 1) {
        subghz->txrx->hopper_idx_frequency++;
    } else {
        subghz->txrx->hopper_idx_frequency = 0;
    }

    if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
        subghz_rx_end(subghz);
    };
    if(subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) {
        subghz_receiver_reset(subghz->txrx->receiver);
        subghz->txrx->preset->frequency = subghz_setting_get_hopper_frequency(
            subghz->setting, subghz->txrx->hopper_idx_frequency);
        subghz_rx(subghz, subghz->txrx->preset->frequency);
    }
}
