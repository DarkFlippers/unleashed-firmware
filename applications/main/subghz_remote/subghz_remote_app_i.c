#include "subghz_remote_app_i.h"
#include <lib/toolbox/path.h>
#include <flipper_format/flipper_format_i.h>

#include "helpers/txrx/subghz_txrx.h"

// #include <lib/subghz/protocols/keeloq.h>
// #include <lib/subghz/protocols/star_line.h>

#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/blocks/custom_btn.h>

#define TAG "SubGhzRemote"

static const char* map_file_labels[SubRemSubKeyNameMaxCount][2] = {
    [SubRemSubKeyNameUp] = {"UP", "ULABEL"},
    [SubRemSubKeyNameDown] = {"DOWN", "DLABEL"},
    [SubRemSubKeyNameLeft] = {"LEFT", "LLABEL"},
    [SubRemSubKeyNameRight] = {"RIGHT", "RLABEL"},
    [SubRemSubKeyNameOk] = {"OK", "OKLABEL"},
};

void subrem_map_preset_reset(SubRemMapPreset* map_preset) {
    furi_assert(map_preset);

    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        subrem_sub_file_preset_reset(map_preset->subs_preset[i]);
    }
}

static SubRemLoadMapState subrem_map_preset_check(
    SubRemMapPreset* map_preset,
    SubGhzTxRx* txrx,
    FlipperFormat* fff_data_file) {
    furi_assert(map_preset);
    furi_assert(txrx);

    bool all_loaded = true;
    SubRemLoadMapState ret = SubRemLoadMapStateErrorBrokenFile;

    SubRemLoadSubState sub_loadig_state;
    SubRemSubFilePreset* sub_preset;

    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        sub_preset = map_preset->subs_preset[i];

        sub_loadig_state = SubRemLoadSubStateErrorNoFile;

        if(furi_string_empty(sub_preset->file_path)) {
            // FURI_LOG_I(TAG, "Empty file path");
        } else if(!flipper_format_file_open_existing(
                      fff_data_file, furi_string_get_cstr(sub_preset->file_path))) {
            sub_preset->load_state = SubRemLoadSubStateErrorNoFile;
            FURI_LOG_W(TAG, "Error open file %s", furi_string_get_cstr(sub_preset->file_path));
        } else {
            sub_loadig_state = subrem_sub_preset_load(sub_preset, txrx, fff_data_file);
        }

        if(sub_loadig_state != SubRemLoadSubStateOK) {
            all_loaded = false;
        } else {
            ret = SubRemLoadMapStateNotAllOK;
        }

        if(ret != SubRemLoadMapStateErrorBrokenFile && all_loaded) {
            ret = SubRemLoadMapStateOK;
        }

        flipper_format_file_close(fff_data_file);
    }

    return ret;
}

static bool subrem_map_preset_load(SubRemMapPreset* map_preset, FlipperFormat* fff_data_file) {
    furi_assert(map_preset);
    bool ret = false;
    SubRemSubFilePreset* sub_preset;
    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        sub_preset = map_preset->subs_preset[i];
        if(!flipper_format_read_string(
               fff_data_file, map_file_labels[i][0], sub_preset->file_path)) {
#if FURI_DEBUG
            FURI_LOG_W(TAG, "No file patch for %s", map_file_labels[i][0]);
#endif
            sub_preset->type = SubGhzProtocolTypeUnknown;
        } else if(!path_contains_only_ascii(furi_string_get_cstr(sub_preset->file_path))) {
            FURI_LOG_E(TAG, "Incorrect characters in [%s] file path", map_file_labels[i][0]);
            sub_preset->type = SubGhzProtocolTypeUnknown;
        } else if(!flipper_format_rewind(fff_data_file)) {
            // Rewind error
        } else if(!flipper_format_read_string(
                      fff_data_file, map_file_labels[i][1], sub_preset->label)) {
#if FURI_DEBUG
            FURI_LOG_W(TAG, "No Label for %s", map_file_labels[i][0]);
#endif
            ret = true;
        } else {
            ret = true;
        }
        if(ret) {
            // Preload seccesful
            FURI_LOG_I(
                TAG,
                "%-5s: %s %s",
                map_file_labels[i][0],
                furi_string_get_cstr(sub_preset->label),
                furi_string_get_cstr(sub_preset->file_path));
            sub_preset->load_state = SubRemLoadSubStatePreloaded;
        }

        flipper_format_rewind(fff_data_file);
    }
    return ret;
}

SubRemLoadMapState subrem_map_file_load(SubGhzRemoteApp* app, const char* file_path) {
    furi_assert(app);
    furi_assert(file_path);
#if FURI_DEBUG
    FURI_LOG_I(TAG, "Load Map File Start");
#endif
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    SubRemLoadMapState ret = SubRemLoadMapStateErrorOpenError;
#if FURI_DEBUG
    FURI_LOG_I(TAG, "Open Map File..");
#endif
    subrem_map_preset_reset(app->map_preset);

    if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
        FURI_LOG_E(TAG, "Could not open MAP file %s", file_path);
        ret = SubRemLoadMapStateErrorOpenError;
    } else {
        if(!subrem_map_preset_load(app->map_preset, fff_data_file)) {
            FURI_LOG_E(TAG, "Could no Sub file path in MAP file");
            // ret = // error for popup
        } else if(!flipper_format_file_close(fff_data_file)) {
            ret = SubRemLoadMapStateErrorOpenError;
        } else {
            ret = subrem_map_preset_check(app->map_preset, app->txrx, fff_data_file);
        }
    }

    if(ret == SubRemLoadMapStateOK) {
        FURI_LOG_I(TAG, "Load Map File Seccesful");
    } else if(ret == SubRemLoadMapStateNotAllOK) {
        FURI_LOG_I(TAG, "Load Map File Seccesful [Not all files]");
    } else {
        FURI_LOG_E(TAG, "Broken Map File");
    }

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);

    furi_record_close(RECORD_STORAGE);
    return ret;
}

bool subrem_save_protocol_to_file(FlipperFormat* flipper_format, const char* sub_file_name) {
    furi_assert(flipper_format);
    furi_assert(sub_file_name);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* flipper_format_stream = flipper_format_get_raw_stream(flipper_format);

    bool saved = false;
    uint32_t repeat = 200;
    FuriString* file_dir = furi_string_alloc();

    path_extract_dirname(sub_file_name, file_dir);
    do {
        // removing additional fields
        flipper_format_delete_key(flipper_format, "Repeat");
        // flipper_format_delete_key(flipper_format, "Manufacture");

        if(!storage_simply_remove(storage, sub_file_name)) {
            break;
        }

        //ToDo check Write
        stream_seek(flipper_format_stream, 0, StreamOffsetFromStart);
        stream_save_to_file(flipper_format_stream, storage, sub_file_name, FSOM_CREATE_ALWAYS);

        if(!flipper_format_insert_or_update_uint32(flipper_format, "Repeat", &repeat, 1)) {
            FURI_LOG_E(TAG, "Unable Repeat");
            break;
        }

        saved = true;
    } while(0);

    furi_string_free(file_dir);
    furi_record_close(RECORD_STORAGE);
    return saved;
}

void subrem_save_active_sub(void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;

    SubRemSubFilePreset* sub_preset = app->map_preset->subs_preset[app->chusen_sub];
    subrem_save_protocol_to_file(
        sub_preset->fff_data, furi_string_get_cstr(sub_preset->file_path));
}

bool subrem_tx_start_sub(SubGhzRemoteApp* app, SubRemSubFilePreset* sub_preset) {
    furi_assert(app);
    furi_assert(sub_preset);
    bool ret = false;

    subrem_tx_stop_sub(app, true);

    if(sub_preset->type == SubGhzProtocolTypeUnknown) {
        ret = false;
    } else {
        FURI_LOG_I(TAG, "Send %s", furi_string_get_cstr(sub_preset->label));

        subghz_txrx_load_decoder_by_name_protocol(
            app->txrx, furi_string_get_cstr(sub_preset->protocaol_name));

        subghz_txrx_set_preset(
            app->txrx,
            furi_string_get_cstr(sub_preset->freq_preset.name),
            sub_preset->freq_preset.frequency,
            NULL,
            0);

        subghz_custom_btns_reset();

        if(subghz_txrx_tx_start(app->txrx, sub_preset->fff_data) == SubGhzTxRxStartTxStateOk) {
            ret = true;
        }
    }

    return ret;
}

bool subrem_tx_stop_sub(SubGhzRemoteApp* app, bool forced) {
    furi_assert(app);
    SubRemSubFilePreset* sub_preset = app->map_preset->subs_preset[app->chusen_sub];

    if(forced || (sub_preset->type != SubGhzProtocolTypeRAW)) {
        subghz_txrx_stop(app->txrx);

        if(sub_preset->type == SubGhzProtocolTypeDynamic) {
            subghz_txrx_reset_dynamic_and_custom_btns(app->txrx);
        }
        subghz_custom_btns_reset();

        return true;
    }

    return false;
}

SubRemLoadMapState subrem_load_from_file(SubGhzRemoteApp* app) {
    furi_assert(app);

    FuriString* file_path = furi_string_alloc();
    SubRemLoadMapState ret = SubRemLoadMapStateBack;

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, SUBREM_APP_EXTENSION, &I_subrem_10px);
    browser_options.base_path = SUBREM_APP_FOLDER;

    // Input events and views are managed by file_select
    if(!dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options)) {
    } else {
        ret = subrem_map_file_load(app, furi_string_get_cstr(app->file_path));
    }

    furi_string_free(file_path);

    return ret;
}

bool subrem_save_map_to_file(SubGhzRemoteApp* app) {
    furi_assert(app);

    const char* file_name = furi_string_get_cstr(app->file_path);
    bool saved = false;
    FlipperFormat* fff_data = flipper_format_string_alloc();

    SubRemSubFilePreset* sub_preset;

    flipper_format_write_header_cstr(
        fff_data, SUBREM_APP_APP_FILE_TYPE, SUBREM_APP_APP_FILE_VERSION);
    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        sub_preset = app->map_preset->subs_preset[i];
        if(!furi_string_empty(sub_preset->file_path)) {
            flipper_format_write_string(fff_data, map_file_labels[i][0], sub_preset->file_path);
        }
    }
    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        sub_preset = app->map_preset->subs_preset[i];
        if(!furi_string_empty(sub_preset->file_path)) {
            flipper_format_write_string(fff_data, map_file_labels[i][1], sub_preset->label);
        }
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* flipper_format_stream = flipper_format_get_raw_stream(fff_data);

    do {
        if(!storage_simply_remove(storage, file_name)) {
            break;
        }
        //ToDo check Write
        stream_seek(flipper_format_stream, 0, StreamOffsetFromStart);
        stream_save_to_file(flipper_format_stream, storage, file_name, FSOM_CREATE_ALWAYS);

        saved = true;
    } while(0);

    furi_record_close(RECORD_STORAGE);
    flipper_format_free(fff_data);

    return saved;
}