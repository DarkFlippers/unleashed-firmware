#include "subghz_remote_app_i.h"
#include <lib/toolbox/path.h>
#include <flipper_format/flipper_format_i.h>

#include <lib/subghz/protocols/raw.h>
#include <lib/subghz/protocols/protocol_items.h>

#define TAG "SubGhzRemote"

static const char* map_file_labels[SUBREM_MAX_SUB_KEY_COUNT][2] = {
    {"UP", "ULABEL"},
    {"DOWN", "DLABEL"},
    {"LEFT", "LLABEL"},
    {"RIGHT", "RLABEL"},
    {"OK", "OKLABEL"},
};

bool subrem_set_preset_data(SubGhzSetting* setting, FreqPreset* freq_preset, const char* preset) {
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

    sub_preset->type = SubRemSubKeyTypeNoData;

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

void subrem_subs_file_preset_reset(SubRemSubFilePreset* sub_preset) {
    furi_assert(sub_preset);

    furi_string_set_str(sub_preset->label, "N/A");
    furi_string_reset(sub_preset->protocaol_name);
    furi_string_reset(sub_preset->file_path);

    Stream* fff_data_stream = flipper_format_get_raw_stream(sub_preset->fff_data);
    stream_clean(fff_data_stream);

    sub_preset->type = SubRemSubKeyTypeNoData;
}

void subrem_sub_file_presets_reset(SubGhzRemoteApp* app) {
    furi_assert(app);

    for(uint8_t i = 0; i < SUBREM_MAX_SUB_KEY_COUNT; i++) {
        subrem_subs_file_preset_reset(app->subs_preset[i]);
    }
}

static bool subrem_sub_file_presets_load(SubGhzRemoteApp* app, FlipperFormat* fff_data_file) {
    furi_assert(app);
    bool ret = false;

    for(uint8_t i = 0; i < SUBREM_MAX_SUB_KEY_COUNT; i++) {
        if(!flipper_format_read_string(
               fff_data_file, map_file_labels[i][0], app->subs_preset[i]->file_path)) {
            FURI_LOG_W(TAG, "No file patch for %s", map_file_labels[i][0]);
            app->subs_preset[i]->type = SubRemSubKeyTypeNoData;
            //continue;
        } else if(!flipper_format_rewind(fff_data_file)) {
            // Rewind error
            //continue;
        } else if(!flipper_format_read_string(
                      fff_data_file, map_file_labels[i][1], app->subs_preset[i]->label)) {
            FURI_LOG_W(TAG, "No Label for %s", map_file_labels[i][0]);
            furi_string_set_str(app->subs_preset[i]->label, "N/A");
        } else {
            FURI_LOG_I(
                TAG,
                // "Loaded %s key \r\nLabel %s file %s",
                "Loaded %s key: %s %s",
                map_file_labels[i][0],
                furi_string_get_cstr(app->subs_preset[i]->label),
                furi_string_get_cstr(app->subs_preset[i]->file_path));
            app->subs_preset[i]->type = SubRemSubKeyTypeHaveFileName; // TODO:
            ret = true;
        }
        flipper_format_rewind(fff_data_file);
    }
    return ret;
}

bool subghz_tx_start_sub(
    SubGhzRemoteApp* app,
    SubRemSubFilePreset* sub_preset,
    SubGhzProtocolEncoderRAWCallbackEnd callback) {
    furi_assert(app);
    furi_assert(sub_preset);
    bool ret = false;

    FURI_LOG_I(TAG, "Send %s", furi_string_get_cstr(sub_preset->label));

    do {
        flipper_format_rewind(sub_preset->fff_data); // FIXME:

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

            if(sub_preset->type == SubRemSubKeyTypeRawKey) {
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
    app->tx_running = ret; // TODO:

    return ret;
}

void subghz_tx_stop_sub(SubGhzRemoteApp* app) {
    furi_assert(app);

    //Stop TX
    furi_hal_subghz_stop_async_tx();

    subghz_transmitter_stop(app->transmitter);
    subghz_transmitter_free(app->transmitter);
    furi_hal_subghz_idle();

    // SubRemSubFilePreset* sub_preset = app->subs_preset[app->chusen_sub];

    // TODO: need saving logic
    app->tx_running = false;
}

static bool subrem_sub_presets_check(SubGhzRemoteApp* app, FlipperFormat* fff_data_file) {
    furi_assert(app);
    FuriString* temp_str = furi_string_alloc();
    uint32_t temp_data32;
    bool ret = false;
    SubRemSubFilePreset* sub_preset;

    for(uint8_t i = 0; i < SUBREM_MAX_SUB_KEY_COUNT; i++) {
        sub_preset = app->subs_preset[i];
        if(sub_preset->type == SubRemSubKeyTypeNoData) {
            continue;
        }
        do {
            if(!flipper_format_file_open_existing(
                   fff_data_file, furi_string_get_cstr(sub_preset->file_path))) {
                FURI_LOG_E(
                    TAG,
                    "Error open file %s",
                    furi_string_get_cstr(sub_preset->file_path)); // TODO: warning
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
                FURI_LOG_W(TAG, "Cannot read frequency. Defaulting to 433.92 MHz");
                sub_preset->freq_preset.frequency = subghz_setting_get_default_frequency(
                    app->setting); // TODO: Get default from settings
            } else if(!furi_hal_subghz_is_tx_allowed(temp_data32)) {
                FURI_LOG_E(TAG, "This frequency can only be used for RX");
                break;
            } else {
                sub_preset->freq_preset.frequency = temp_data32;
            }

            //Load preset
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
            if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
                FURI_LOG_E(TAG, "Missing Protocol");
                break;
            }

            FlipperFormat* fff_data = sub_preset->fff_data;
            if(!strcmp(furi_string_get_cstr(temp_str), "RAW")) {
                //if RAW
                subghz_protocol_raw_gen_fff_data(
                    fff_data, furi_string_get_cstr(sub_preset->file_path));
                sub_preset->type = SubRemSubKeyTypeRawKey;
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
            } else if(protocol->flag & SubGhzProtocolFlag_Send) { // FIXME:

                if(protocol->type == SubGhzProtocolTypeStatic) {
                    sub_preset->type = SubRemSubKeyTypeStaticKey;
                } else if(protocol->type == SubGhzProtocolTypeDynamic) {
                    sub_preset->type = SubRemSubKeyTypeDynamicKey;
                } else if(protocol->type == SubGhzProtocolTypeRAW) {
                    sub_preset->type = SubRemSubKeyTypeRawKey;
                    // } else if(protocol->type == SubGhzProtocolTypeBinRAW) { // TODO: BINRAW
                } else {
                    FURI_LOG_E(TAG, "Unsuported Protocol");
                    break;
                }

                furi_string_set(sub_preset->protocaol_name, temp_str);
            } // TODO: check dynamic and protocol found

            ret |= true;

            if(ret) {
                FURI_LOG_I(TAG, "Protocol Loaded");
            }
        } while(false);

        flipper_format_file_close(fff_data_file);
    }
    furi_string_free(temp_str);

    //ret = false; // TODO:
    return ret;
}

bool subrem_map_file_load(SubGhzRemoteApp* app, const char* file_path) {
    furi_assert(app);
    furi_assert(file_path);
    // TODO: drop furi log
    FURI_LOG_I(TAG, "Load Map File Start"); // drop
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);
    bool ret = false;
    FURI_LOG_I(TAG, "Open Map File.."); // drop

    subrem_sub_file_presets_reset(app);

    if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
        FURI_LOG_E(TAG, "Could not open MAP file %s", file_path);
    } else {
        if(!subrem_sub_file_presets_load(app, fff_data_file)) {
            // TODO: error popup or return error type
            FURI_LOG_E(TAG, "Could no Sub file path in  MAP file");
        } else if(
            (flipper_format_file_close(fff_data_file)) &&
            (subrem_sub_presets_check(app, fff_data_file))) {
            FURI_LOG_I(TAG, "Load Map File Seccesful");
            ret = true;
        }
    }

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);

    furi_record_close(RECORD_STORAGE);

    //ret = false; // TODO:
    return ret;
}

bool subrem_load_from_file(SubGhzRemoteApp* app) {
    furi_assert(app);

    FuriString* file_path = furi_string_alloc();

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, SUBREM_APP_EXTENSION, &I_sub1_10px);
    browser_options.base_path = SUBREM_APP_FOLDER;

    // Input events and views are managed by file_select
    bool res =
        dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options);

    if(res) {
        res = subrem_map_file_load(app, furi_string_get_cstr(app->file_path));
        // FIXME res = subghz_key_load(app, furi_string_get_cstr(app->file_path), true);
    }

    furi_string_free(file_path);

    return res;
}
