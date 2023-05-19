#include "subghz_remote_app_i.h"
#include <lib/toolbox/path.h>
#include <flipper_format/flipper_format_i.h>

#include <lib/subghz/protocols/protocol_items.h>

// #include <lib/subghz/protocols/keeloq.h>
// #include <lib/subghz/protocols/star_line.h>

#include <lib/subghz/blocks/custom_btn.h>

#define TAG "SubGhzRemote"

static const char* map_file_labels[SubRemSubKeyNameMaxCount][2] = {
    [SubRemSubKeyNameUp] = {"UP", "ULABEL"},
    [SubRemSubKeyNameDown] = {"DOWN", "DLABEL"},
    [SubRemSubKeyNameLeft] = {"LEFT", "LLABEL"},
    [SubRemSubKeyNameRight] = {"RIGHT", "RLABEL"},
    [SubRemSubKeyNameOk] = {"OK", "OKLABEL"},
};

static bool
    subrem_set_preset_data(SubGhzSetting* setting, FreqPreset* freq_preset, const char* preset) {
    const char* preset_name = "";
    if(!strcmp(preset, "FuriHalSubGhzPresetOok270Async")) {
        preset_name = "AM270";
    } else if(!strcmp(preset, "FuriHalSubGhzPresetOok650Async")) {
        preset_name = "AM650";
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev238Async")) {
        preset_name = "FM238";
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev476Async")) {
        preset_name = "FM476";
    } else if(!strcmp(preset, "FuriHalSubGhzPresetCustom")) {
        // preset_name = "CUSTOM";
        return false;
    } else {
        FURI_LOG_E(TAG, "Unknown preset");
        return false;
    }
    size_t preset_index = subghz_setting_get_inx_preset_by_name(setting, preset_name);
    freq_preset->data = subghz_setting_get_preset_data(setting, preset_index);
    return true;
}

SubRemSubFilePreset* subrem_sub_file_preset_alloc() {
    SubRemSubFilePreset* sub_preset = malloc(sizeof(SubRemSubFilePreset));

    sub_preset->fff_data = flipper_format_string_alloc();
    sub_preset->file_path = furi_string_alloc();
    sub_preset->protocaol_name = furi_string_alloc();
    sub_preset->label = furi_string_alloc_set_str("N/A");

    sub_preset->type = SubGhzProtocolTypeUnknown;
    sub_preset->load_state = SubRemLoadSubStateNotSet;

    return sub_preset;
}

void subrem_sub_file_preset_free(SubRemSubFilePreset* sub_preset) {
    furi_assert(sub_preset);

    furi_string_free(sub_preset->label);
    furi_string_free(sub_preset->protocaol_name);
    furi_string_free(sub_preset->file_path);
    flipper_format_free(sub_preset->fff_data);

    free(sub_preset);
}

static void subrem_sub_file_preset_reset(SubRemSubFilePreset* sub_preset) {
    furi_assert(sub_preset);

    furi_string_set_str(sub_preset->label, "N/A");
    furi_string_reset(sub_preset->protocaol_name);
    furi_string_reset(sub_preset->file_path);

    Stream* fff_data_stream = flipper_format_get_raw_stream(sub_preset->fff_data);
    stream_clean(fff_data_stream);

    sub_preset->type = SubGhzProtocolTypeUnknown;
    sub_preset->load_state = SubRemLoadSubStateNotSet;
}

void subrem_map_preset_reset(SubGhzRemoteApp* app) {
    furi_assert(app);

    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        subrem_sub_file_preset_reset(app->subs_preset[i]);
    }
}

static bool subrem_map_preset_load(SubGhzRemoteApp* app, FlipperFormat* fff_data_file) {
    furi_assert(app);
    bool ret = false;
    SubRemSubFilePreset* sub_preset;
    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        sub_preset = app->subs_preset[i];
        if(!flipper_format_read_string(
               fff_data_file, map_file_labels[i][0], sub_preset->file_path)) {
#if FURI_DEBUG
            FURI_LOG_W(TAG, "No file patch for %s", map_file_labels[i][0]);
#endif
            sub_preset->type = SubGhzProtocolTypeUnknown;
        } else if(!flipper_format_rewind(fff_data_file)) {
            // Rewind error
        } else if(!flipper_format_read_string(
                      fff_data_file, map_file_labels[i][1], sub_preset->label)) {
#if FURI_DEBUG
            FURI_LOG_W(TAG, "No Label for %s", map_file_labels[i][0]);
#endif
            path_extract_filename(sub_preset->file_path, sub_preset->label, true);
        } else {
            // Preload seccesful
            FURI_LOG_I(
                TAG,
                "%-5s: %s %s",
                map_file_labels[i][0],
                furi_string_get_cstr(sub_preset->label),
                furi_string_get_cstr(sub_preset->file_path));
            ret = true;
            sub_preset->load_state = SubRemLoadSubStatePreloaded;
        }
        flipper_format_rewind(fff_data_file);
    }
    return ret;
}

bool subrem_save_protocol_to_file(FlipperFormat* flipper_format, const char* dev_file_name) {
    furi_assert(flipper_format);
    furi_assert(dev_file_name);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* flipper_format_stream = flipper_format_get_raw_stream(flipper_format);

    bool saved = false;
    uint32_t repeat = 200;
    FuriString* file_dir = furi_string_alloc();

    path_extract_dirname(dev_file_name, file_dir);
    do {
        // removing additional fields
        flipper_format_delete_key(flipper_format, "Repeat");
        // flipper_format_delete_key(flipper_format, "Manufacture");

        if(!storage_simply_remove(storage, dev_file_name)) {
            break;
        }

        //ToDo check Write
        stream_seek(flipper_format_stream, 0, StreamOffsetFromStart);
        stream_save_to_file(flipper_format_stream, storage, dev_file_name, FSOM_CREATE_ALWAYS);

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

bool subrem_tx_start_sub(
    SubGhzRemoteApp* app,
    SubRemSubFilePreset* sub_preset,
    SubGhzProtocolEncoderRAWCallbackEnd callback) {
    furi_assert(app);
    furi_assert(sub_preset);
    bool ret = false;

    subrem_tx_stop_sub(app, true);

    if(sub_preset->type == SubGhzProtocolTypeUnknown) {
        return false;
    }

    FURI_LOG_I(TAG, "Send %s", furi_string_get_cstr(sub_preset->label));

    subghz_custom_btn_set(SUBGHZ_CUSTOM_BTN_OK);
    keeloq_reset_original_btn();
    subghz_custom_btns_reset();

    do {
        flipper_format_rewind(sub_preset->fff_data); //

        app->transmitter = subghz_transmitter_alloc_init(
            app->environment, furi_string_get_cstr(sub_preset->protocaol_name));

        if(app->transmitter) {
            if(subghz_transmitter_deserialize(app->transmitter, sub_preset->fff_data) !=
               SubGhzProtocolStatusOk) {
                FURI_LOG_E(TAG, "Deserialize error!");
                break;
            }
            furi_hal_subghz_reset();
            furi_hal_subghz_idle();
            furi_hal_subghz_load_custom_preset(sub_preset->freq_preset.data);
            furi_hal_gpio_init(
                furi_hal_subghz.cc1101_g0_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);

            furi_hal_subghz_idle();

            furi_hal_subghz_set_frequency_and_path(sub_preset->freq_preset.frequency);
            furi_hal_gpio_write(furi_hal_subghz.cc1101_g0_pin, false);
            furi_hal_gpio_init(
                furi_hal_subghz.cc1101_g0_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);

            if(!furi_hal_subghz_tx()) {
                FURI_LOG_E(TAG, "Sending not allowed");
                break;
            }

            if(sub_preset->type == SubGhzProtocolTypeRAW) {
                subghz_protocol_raw_file_encoder_worker_set_callback_end(
                    (SubGhzProtocolEncoderRAW*)subghz_transmitter_get_protocol_instance(
                        app->transmitter),
                    callback,
                    app);
            }

            furi_hal_subghz_start_async_tx(subghz_transmitter_yield, app->transmitter);

            ret = true;
        }
    } while(false);

    app->tx_running = ret;

    return ret;
}

static void subghz_tx_stop(SubGhzRemoteApp* app) {
    furi_assert(app);

    //Stop TX
    furi_hal_subghz_stop_async_tx();

    subghz_transmitter_stop(app->transmitter);
    subghz_transmitter_free(app->transmitter);
    furi_hal_subghz_idle();
}

bool subrem_tx_stop_sub(SubGhzRemoteApp* app, bool forced) {
    furi_assert(app);
    SubRemSubFilePreset* sub_preset = app->subs_preset[app->chusen_sub];

    if(forced || (sub_preset->type != SubGhzProtocolTypeRAW)) {
        // SubRemSubKeyTypeRawKey)) {
        if(app->tx_running) {
            subghz_tx_stop(app);

            if(sub_preset->type == SubGhzProtocolTypeDynamic) {
                subrem_save_protocol_to_file(
                    sub_preset->fff_data, furi_string_get_cstr(sub_preset->file_path));

                keeloq_reset_mfname();
                keeloq_reset_kl_type();
                keeloq_reset_original_btn();
                subghz_custom_btns_reset();
                star_line_reset_mfname();
                star_line_reset_kl_type();
            }

            app->tx_running = false;
            return true;
        }
    }

    return false;
}

static SubRemLoadMapState
    subrem_map_preset_check(SubGhzRemoteApp* app, FlipperFormat* fff_data_file) {
    furi_assert(app);
    FuriString* temp_str = furi_string_alloc();
    uint32_t temp_data32;
    bool all_loaded = true;
    SubRemLoadMapState ret = SubRemLoadMapStateErrorBrokenFile;
    SubRemLoadSubState sub_preset_loaded;
    SubRemSubFilePreset* sub_preset;
    uint32_t repeat = 200;
    for(uint8_t i = 0; i < SubRemSubKeyNameMaxCount; i++) {
        sub_preset = app->subs_preset[i];
        if(furi_string_empty(sub_preset->file_path)) {
            // FURI_LOG_I(TAG, "Empty file path");
            continue;
        }

        sub_preset_loaded = SubRemLoadSubStateErrorNoFile;

        repeat = 200;
        do {
            if(!flipper_format_file_open_existing(
                   fff_data_file, furi_string_get_cstr(sub_preset->file_path))) {
                FURI_LOG_W(TAG, "Error open file %s", furi_string_get_cstr(sub_preset->file_path));
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
            sub_preset_loaded = SubRemLoadSubStateErrorFreq;
            if(!flipper_format_read_uint32(fff_data_file, "Frequency", &temp_data32, 1)) {
                FURI_LOG_W(TAG, "Cannot read frequency. Set default frequency");
                sub_preset->freq_preset.frequency =
                    subghz_setting_get_default_frequency(app->setting);
            } else if(!furi_hal_subghz_is_tx_allowed(temp_data32)) {
                FURI_LOG_E(TAG, "This frequency can only be used for RX");
                break;
            } else {
                sub_preset->freq_preset.frequency = temp_data32;
            }

            //Load preset
            sub_preset_loaded = SubRemLoadSubStateErrorMod;
            if(!flipper_format_read_string(fff_data_file, "Preset", temp_str)) {
                FURI_LOG_E(TAG, "Missing Preset");
                break;
            }

            if(!subrem_set_preset_data(
                   app->setting, &sub_preset->freq_preset, furi_string_get_cstr(temp_str))) {
                FURI_LOG_E(TAG, "Cannot load preset.");
                break;
            }

            //Load protocol
            sub_preset_loaded = SubRemLoadSubStateErrorProtocol;
            if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
                FURI_LOG_E(TAG, "Missing Protocol");
                break;
            }

            FlipperFormat* fff_data = sub_preset->fff_data;
            if(!strcmp(furi_string_get_cstr(temp_str), "RAW")) {
                //if RAW
                subghz_protocol_raw_gen_fff_data(
                    fff_data, furi_string_get_cstr(sub_preset->file_path));
            } else {
                stream_copy_full(
                    flipper_format_get_raw_stream(fff_data_file),
                    flipper_format_get_raw_stream(fff_data));
            }

            const SubGhzProtocolRegistry* protocol_registry_items =
                subghz_environment_get_protocol_registry(app->environment);

            const SubGhzProtocol* protocol = subghz_protocol_registry_get_by_name(
                protocol_registry_items, furi_string_get_cstr(temp_str));

            if(!protocol) {
                FURI_LOG_E(TAG, "Protocol not found");
                break;
            } else if(protocol->flag & SubGhzProtocolFlag_Send) {
                if((protocol->type == SubGhzProtocolTypeStatic) ||
                   (protocol->type == SubGhzProtocolTypeDynamic) ||
                   // (protocol->type == SubGhzProtocolTypeBinRAW) || // TODO: BINRAW
                   (protocol->type == SubGhzProtocolTypeRAW)) {
                    sub_preset->type = protocol->type;
                } else {
                    FURI_LOG_E(TAG, "Unsuported Protocol");
                    break;
                }

                furi_string_set(sub_preset->protocaol_name, temp_str);
            } else {
                FURI_LOG_E(TAG, "Protocol does not support transmission");
            }

            if(!flipper_format_insert_or_update_uint32(fff_data, "Repeat", &repeat, 1)) {
                FURI_LOG_E(TAG, "Unable Repeat");
                break;
            }

            sub_preset_loaded = SubRemLoadSubStateOK;
            ret = SubRemLoadMapStateNotAllOK;

#if FURI_DEBUG
            FURI_LOG_I(TAG, "%-16s - protocol Loaded", furi_string_get_cstr(sub_preset->label));
#endif
        } while(false);

        // TODO:
        // Load file state logic
        // Label depending on the state
        // Move to remote scene

        if(sub_preset_loaded != SubRemLoadSubStateOK) {
            furi_string_set_str(sub_preset->label, "N/A");
            all_loaded = false;
        }

        if(ret != SubRemLoadMapStateErrorBrokenFile && all_loaded) {
            ret = SubRemLoadMapStateOK;
        }

        flipper_format_file_close(fff_data_file);
    }
    furi_string_free(temp_str);

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
    subrem_map_preset_reset(app);

    if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
        FURI_LOG_E(TAG, "Could not open MAP file %s", file_path);
        ret = SubRemLoadMapStateErrorOpenError;
    } else {
        if(!subrem_map_preset_load(app, fff_data_file)) {
            FURI_LOG_E(TAG, "Could no Sub file path in MAP file");
            // ret = // error for popup
        } else if(!flipper_format_file_close(fff_data_file)) {
            ret = SubRemLoadMapStateErrorOpenError;
        } else {
            ret = subrem_map_preset_check(app, fff_data_file);
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

SubRemLoadMapState subrem_load_from_file(SubGhzRemoteApp* app) {
    furi_assert(app);

    FuriString* file_path = furi_string_alloc();
    SubRemLoadMapState ret = SubRemLoadMapStateBack;

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, SUBREM_APP_EXTENSION, &I_sub1_10px);
    browser_options.base_path = SUBREM_APP_FOLDER;

    // Input events and views are managed by file_select
    if(!dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options)) {
    } else {
        ret = subrem_map_file_load(app, furi_string_get_cstr(app->file_path));
    }

    furi_string_free(file_path);

    return ret;
}
