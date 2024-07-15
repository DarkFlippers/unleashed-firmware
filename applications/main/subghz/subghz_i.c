#include "subghz_i.h"

#include "assets_icons.h"
#include "subghz/types.h"
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/elements.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <flipper_format/flipper_format.h>

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/subghz/protocols/raw.h>

#define TAG "SubGhz"

void subghz_set_default_preset(SubGhz* subghz) {
    furi_assert(subghz);
    subghz_txrx_set_preset(
        subghz->txrx,
        "AM650",
        subghz_setting_get_default_frequency(subghz_txrx_get_setting(subghz->txrx)),
        NULL,
        0);
}

void subghz_blink_start(SubGhz* subghz) {
    furi_assert(subghz);
    notification_message(subghz->notifications, &sequence_blink_stop);
    notification_message(subghz->notifications, &sequence_blink_start_magenta);
}

void subghz_blink_stop(SubGhz* subghz) {
    furi_assert(subghz);
    notification_message(subghz->notifications, &sequence_blink_stop);
}

bool subghz_tx_start(SubGhz* subghz, FlipperFormat* flipper_format) {
    switch(subghz_txrx_tx_start(subghz->txrx, flipper_format)) {
    case SubGhzTxRxStartTxStateErrorParserOthers:
        dialog_message_show_storage_error(
            subghz->dialogs, "Error in protocol\nparameters\ndescription");
        break;
    case SubGhzTxRxStartTxStateErrorOnlyRx:
        subghz_dialog_message_show_only_rx(subghz);
        break;

    default:
        return true;
        break;
    }
    return false;
}

void subghz_dialog_message_show_only_rx(SubGhz* subghz) {
    DialogsApp* dialogs = subghz->dialogs;
    DialogMessage* message = dialog_message_alloc();

    const char* header_text = "Transmission is Blocked!";
    const char* message_text = "Transmission on\nthis frequency is\nrestricted in your\nregion";
    if(!furi_hal_region_is_provisioned()) {
        header_text = "Firmware update needed";
        message_text = "Please update\nfirmware before\nusing this feature\nflipp.dev/upd";
    }

    dialog_message_set_header(message, header_text, 63, 0, AlignCenter, AlignTop);
    dialog_message_set_text(message, message_text, 1, 13, AlignLeft, AlignTop);

    dialog_message_set_icon(message, &I_WarningDolphinFlip_45x42, 83, 22);

    dialog_message_show(dialogs, message);
    dialog_message_free(message);
}

bool subghz_key_load(SubGhz* subghz, const char* file_path, bool show_dialog) {
    furi_assert(subghz);
    furi_assert(file_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    Stream* fff_data_stream =
        flipper_format_get_raw_stream(subghz_txrx_get_fff_data(subghz->txrx));

    SubGhzLoadKeyState load_key_state = SubGhzLoadKeyStateParseErr;
    FuriString* temp_str = furi_string_alloc();
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

        if(((!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_KEY_FILE_TYPE)) ||
            (!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_RAW_FILE_TYPE))) &&
           temp_data32 == SUBGHZ_KEY_FILE_VERSION) {
        } else {
            FURI_LOG_E(TAG, "Type or version mismatch");
            break;
        }

        //Load frequency
        if(!flipper_format_read_uint32(fff_data_file, "Frequency", &temp_data32, 1)) {
            FURI_LOG_E(TAG, "Missing Frequency");
            break;
        }

        if(!subghz_txrx_radio_device_is_frequecy_valid(subghz->txrx, temp_data32)) {
            FURI_LOG_E(TAG, "Frequency not supported");
            break;
        }

        //Load preset
        if(!flipper_format_read_string(fff_data_file, "Preset", temp_str)) {
            FURI_LOG_E(TAG, "Missing Preset");
            break;
        }

        furi_string_set_str(
            temp_str, subghz_txrx_get_preset_name(subghz->txrx, furi_string_get_cstr(temp_str)));
        if(!strcmp(furi_string_get_cstr(temp_str), "")) {
            break;
        }
        SubGhzSetting* setting = subghz_txrx_get_setting(subghz->txrx);

        if(!strcmp(furi_string_get_cstr(temp_str), "CUSTOM")) {
            //TODO FL-3551: add Custom_preset_module
            //delete preset if it already exists
            subghz_setting_delete_custom_preset(setting, furi_string_get_cstr(temp_str));
            //load custom preset from file
            if(!subghz_setting_load_custom_preset(
                   setting, furi_string_get_cstr(temp_str), fff_data_file)) {
                FURI_LOG_E(TAG, "Missing Custom preset");
                break;
            }
        }
        size_t preset_index =
            subghz_setting_get_inx_preset_by_name(setting, furi_string_get_cstr(temp_str));
        subghz_txrx_set_preset(
            subghz->txrx,
            furi_string_get_cstr(temp_str),
            temp_data32,
            subghz_setting_get_preset_data(setting, preset_index),
            subghz_setting_get_preset_data_size(setting, preset_index));

        //Load protocol
        if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            break;
        }

        FlipperFormat* fff_data = subghz_txrx_get_fff_data(subghz->txrx);
        if(!strcmp(furi_string_get_cstr(temp_str), "RAW")) {
            //if RAW
            subghz->load_type_file = SubGhzLoadTypeFileRaw;
            subghz_protocol_raw_gen_fff_data(
                fff_data, file_path, subghz_txrx_radio_device_get_name(subghz->txrx));
        } else {
            subghz->load_type_file = SubGhzLoadTypeFileKey;
            stream_copy_full(
                flipper_format_get_raw_stream(fff_data_file),
                flipper_format_get_raw_stream(fff_data));
        }

        if(subghz_txrx_load_decoder_by_name_protocol(
               subghz->txrx, furi_string_get_cstr(temp_str))) {
            SubGhzProtocolStatus status = subghz_protocol_decoder_base_deserialize(
                subghz_txrx_get_decoder(subghz->txrx), fff_data);
            if(status != SubGhzProtocolStatusOk) {
                load_key_state = SubGhzLoadKeyStateProtocolDescriptionErr;
                break;
            }
        } else {
            FURI_LOG_E(TAG, "Protocol not found");
            break;
        }

        load_key_state = SubGhzLoadKeyStateOK;
    } while(0);

    furi_string_free(temp_str);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);

    switch(load_key_state) {
    case SubGhzLoadKeyStateParseErr:
        if(show_dialog) {
            dialog_message_show_storage_error(subghz->dialogs, "Cannot parse\nfile");
        }
        return false;
    case SubGhzLoadKeyStateProtocolDescriptionErr:
        if(show_dialog) {
            dialog_message_show_storage_error(
                subghz->dialogs, "Error in protocol\nparameters\ndescription");
        }
        return false;

    case SubGhzLoadKeyStateOK:
        return true;

    default:
        furi_crash("SubGhz: Unknown load_key_state.");
        return false;
    }
}

SubGhzLoadTypeFile subghz_get_load_type_file(SubGhz* subghz) {
    furi_assert(subghz);
    return subghz->load_type_file;
}

bool subghz_get_next_name_file(SubGhz* subghz, uint8_t max_len) {
    furi_assert(subghz);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* temp_str = furi_string_alloc();
    FuriString* file_name = furi_string_alloc();
    FuriString* file_path = furi_string_alloc();

    bool res = false;

    if(subghz_path_is_file(subghz->file_path)) {
        //get the name of the next free file
        path_extract_filename(subghz->file_path, file_name, true);
        path_extract_dirname(furi_string_get_cstr(subghz->file_path), file_path);

        storage_get_next_filename(
            storage,
            furi_string_get_cstr(file_path),
            furi_string_get_cstr(file_name),
            SUBGHZ_APP_FILENAME_EXTENSION,
            file_name,
            max_len);

        furi_string_printf(
            temp_str,
            "%s/%s%s",
            furi_string_get_cstr(file_path),
            furi_string_get_cstr(file_name),
            SUBGHZ_APP_FILENAME_EXTENSION);
        furi_string_set(subghz->file_path, temp_str);
        res = true;
    }

    furi_string_free(temp_str);
    furi_string_free(file_path);
    furi_string_free(file_name);
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
    FuriString* file_dir = furi_string_alloc();

    path_extract_dirname(dev_file_name, file_dir);
    do {
        //removing additional fields
        flipper_format_delete_key(flipper_format, "Repeat");
        flipper_format_delete_key(flipper_format, "Manufacture");

        // Create subghz folder directory if necessary
        if(!storage_simply_mkdir(storage, furi_string_get_cstr(file_dir))) {
            dialog_message_show_storage_error(subghz->dialogs, "Cannot create\nfolder");
            break;
        }

        if(!storage_simply_remove(storage, dev_file_name)) {
            break;
        }
        stream_seek(flipper_format_stream, 0, StreamOffsetFromStart);
        stream_save_to_file(flipper_format_stream, storage, dev_file_name, FSOM_CREATE_ALWAYS);

        if(storage_common_stat(storage, dev_file_name, NULL) != FSE_OK) {
            break;
        }

        saved = true;
    } while(0);
    furi_string_free(file_dir);
    furi_record_close(RECORD_STORAGE);
    return saved;
}

void subghz_save_to_file(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    if(subghz_path_is_file(subghz->file_path)) {
        subghz_save_protocol_to_file(
            subghz,
            subghz_txrx_get_fff_data(subghz->txrx),
            furi_string_get_cstr(subghz->file_path));
    }
}

bool subghz_load_protocol_from_file(SubGhz* subghz) {
    furi_assert(subghz);

    FuriString* file_path = furi_string_alloc();

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, SUBGHZ_APP_FILENAME_EXTENSION, &I_sub1_10px);
    browser_options.base_path = SUBGHZ_APP_FOLDER;

    // Input events and views are managed by file_select
    bool res = dialog_file_browser_show(
        subghz->dialogs, subghz->file_path, subghz->file_path, &browser_options);

    if(res) {
        res = subghz_key_load(subghz, furi_string_get_cstr(subghz->file_path), true);
    }

    furi_string_free(file_path);

    return res;
}

bool subghz_rename_file(SubGhz* subghz) {
    furi_assert(subghz);
    bool ret = true;

    Storage* storage = furi_record_open(RECORD_STORAGE);

    if(furi_string_cmp(subghz->file_path_tmp, subghz->file_path)) {
        FS_Error fs_result = storage_common_rename(
            storage,
            furi_string_get_cstr(subghz->file_path_tmp),
            furi_string_get_cstr(subghz->file_path));

        if(fs_result != FSE_OK) {
            dialog_message_show_storage_error(subghz->dialogs, "Cannot rename\n file/directory");
            ret = false;
        }
    }
    furi_record_close(RECORD_STORAGE);

    return ret;
}

bool subghz_file_available(SubGhz* subghz) {
    furi_assert(subghz);
    bool ret = true;
    Storage* storage = furi_record_open(RECORD_STORAGE);

    FS_Error fs_result =
        storage_common_stat(storage, furi_string_get_cstr(subghz->file_path), NULL);

    if(fs_result != FSE_OK) {
        dialog_message_show_storage_error(subghz->dialogs, "File not available\n file/directory");
        ret = false;
    }

    furi_record_close(RECORD_STORAGE);
    return ret;
}

bool subghz_delete_file(SubGhz* subghz) {
    furi_assert(subghz);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool result = storage_simply_remove(storage, furi_string_get_cstr(subghz->file_path_tmp));
    furi_record_close(RECORD_STORAGE);

    subghz_file_name_clear(subghz);

    return result;
}

void subghz_file_name_clear(SubGhz* subghz) {
    furi_assert(subghz);
    furi_string_set(subghz->file_path, SUBGHZ_APP_FOLDER);
    furi_string_reset(subghz->file_path_tmp);
}

bool subghz_path_is_file(FuriString* path) {
    return furi_string_end_with(path, SUBGHZ_APP_FILENAME_EXTENSION);
}

void subghz_lock(SubGhz* subghz) {
    furi_assert(subghz);
    subghz->lock = SubGhzLockOn;
}

void subghz_unlock(SubGhz* subghz) {
    furi_assert(subghz);
    subghz->lock = SubGhzLockOff;
}

bool subghz_is_locked(SubGhz* subghz) {
    furi_assert(subghz);
    return subghz->lock == SubGhzLockOn;
}

void subghz_rx_key_state_set(SubGhz* subghz, SubGhzRxKeyState state) {
    furi_assert(subghz);
    subghz->rx_key_state = state;
}

SubGhzRxKeyState subghz_rx_key_state_get(SubGhz* subghz) {
    furi_assert(subghz);
    return subghz->rx_key_state;
}
