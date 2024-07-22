#include "../../js_modules.h"
#include "radio_device_loader.h"

#include <lib/subghz/transmitter.h>
#include <lib/subghz/devices/devices.h>
#include <lib/subghz/protocols/protocol_items.h>

#include <flipper_format/flipper_format_i.h>

#define TAG "js_subghz"

typedef enum {
    JsSubghzRadioStateRX,
    JsSubghzRadioStateTX,
    JsSubghzRadioStateIDLE,
} JsSubghzRadioState;

typedef struct {
    const SubGhzDevice* radio_device;
    int frequency;
    bool is_external;
    JsSubghzRadioState state;
} JsSubghzInst;

static FuriHalSubGhzPreset js_subghz_get_preset_name(const char* preset_name) {
    FuriHalSubGhzPreset preset = FuriHalSubGhzPresetIDLE;
    if(!strcmp(preset_name, "FuriHalSubGhzPresetOok270Async")) {
        preset = FuriHalSubGhzPresetOok270Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPresetOok650Async")) {
        preset = FuriHalSubGhzPresetOok650Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPreset2FSKDev238Async")) {
        preset = FuriHalSubGhzPreset2FSKDev238Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPreset2FSKDev476Async")) {
        preset = FuriHalSubGhzPreset2FSKDev476Async;
    } else if(!strcmp(preset_name, "FuriHalSubGhzPresetCustom")) {
        preset = FuriHalSubGhzPresetCustom;
    } else {
        FURI_LOG_I(TAG, "unknown preset");
    }
    return preset;
}

static void js_subghz_set_rx(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(!js_subghz->radio_device) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not setup");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    if(js_subghz->state != JsSubghzRadioStateRX) {
        subghz_devices_set_rx(js_subghz->radio_device);
        js_subghz->state = JsSubghzRadioStateRX;
    }

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_subghz_set_idle(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(!js_subghz->radio_device) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not setup");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    if(js_subghz->state != JsSubghzRadioStateIDLE) {
        subghz_devices_idle(js_subghz->radio_device);
        js_subghz->state = JsSubghzRadioStateIDLE;
    }

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_subghz_get_rssi(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(!js_subghz->radio_device) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not setup");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    if(js_subghz->state != JsSubghzRadioStateRX) {
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    float rssi = subghz_devices_get_rssi(js_subghz->radio_device);
    mjs_return(mjs, mjs_mk_number(mjs, (double)rssi));
}

static void js_subghz_get_state(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(!js_subghz->radio_device) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not setup");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const char* state;
    switch(js_subghz->state) {
    case JsSubghzRadioStateRX:
        state = "RX";
        break;
    case JsSubghzRadioStateTX:
        state = "TX";
        break;
    case JsSubghzRadioStateIDLE:
        state = "IDLE";
        break;
    default:
        state = "";
        break;
    }

    mjs_return(mjs, mjs_mk_string(mjs, state, ~0, true));
}

static void js_subghz_is_external(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(!js_subghz->radio_device) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not setup");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    mjs_return(mjs, mjs_mk_boolean(mjs, js_subghz->is_external));
}

static void js_subghz_set_frequency(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(!js_subghz->radio_device) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not setup");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    if(js_subghz->state != JsSubghzRadioStateIDLE) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not in IDLE state");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    mjs_val_t frequency_arg = mjs_arg(mjs, 0);
    if(!mjs_is_number(frequency_arg)) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Frequency must be a number");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }
    int32_t frequency = mjs_get_int32(mjs, frequency_arg);

    if(!subghz_devices_is_frequency_valid(js_subghz->radio_device, frequency)) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Invalid frequency");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    js_subghz->frequency = subghz_devices_set_frequency(js_subghz->radio_device, frequency);

    mjs_return(mjs, mjs_mk_number(mjs, (double)js_subghz->frequency));
}

static void js_subghz_get_frequency(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(!js_subghz->radio_device) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not setup");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    mjs_return(mjs, mjs_mk_number(mjs, (double)js_subghz->frequency));
}

static void js_subghz_transmit_file(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(!js_subghz->radio_device) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not setup");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    mjs_val_t file = mjs_arg(mjs, 0);
    if(!mjs_is_string(file)) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "File must be a string");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    const char* file_path = mjs_get_string(mjs, &file, NULL);
    if(!file_path) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Failed to get file path");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    // Repeat works weirdly:
    // - "Repeat" in parsed protocol is like holding Send in Sub-GHz app
    //   This is necessary as most receivers require hearing signals multiple times
    // - "repeat" as variable and loop in this code applies to RAW files only
    //   parsed files handle repeat in protocol layer instead
    // We keep 0 as default, or literal value if specified by user
    // If user did not specify, -1 is detected below, and we use:
    // - 1 repeat for RAW
    // - 10 repeats for parsed, which is passed to protocol, and we loop once here
    uint32_t repeat = 0;
    mjs_val_t repeat_arg = mjs_arg(mjs, 1);
    if(mjs_is_number(repeat_arg)) {
        int32_t repeat_val = mjs_get_int32(mjs, repeat_arg);
        repeat = MAX(repeat_val, 0);
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_file = flipper_format_file_alloc(storage);
    FlipperFormat* fff_raw = NULL;

    if(!flipper_format_file_open_existing(fff_file, file_path)) {
        flipper_format_free(fff_file);
        furi_record_close(RECORD_STORAGE);
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Failed to open file");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    SubGhzEnvironment* environment = subghz_environment_alloc();
    if(!subghz_environment_load_keystore(environment, SUBGHZ_KEYSTORE_DIR_NAME)) {
        FURI_LOG_W(TAG, "Load_keystore keeloq_mfcodes - failed to load");
    }
    if(!subghz_environment_load_keystore(environment, SUBGHZ_KEYSTORE_DIR_USER_NAME)) {
        FURI_LOG_W(TAG, "Load_keystore keeloq_mfcodes_user - failed to load");
    }
    subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
        environment, SUBGHZ_ALUTECH_AT_4N_DIR_NAME);
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        environment, SUBGHZ_NICE_FLOR_S_DIR_NAME);
    subghz_environment_set_protocol_registry(environment, (void*)&subghz_protocol_registry);

    FuriString* temp_str = furi_string_alloc();
    SubGhzTransmitter* transmitter = NULL;
    uint32_t temp_data32 = 0;
    uint32_t frequency = 0;
    bool is_sent = false;

    do {
        if(!flipper_format_read_header(fff_file, temp_str, &temp_data32)) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Missing or incorrect header");
            break;
        }

        if(((!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_KEY_FILE_TYPE)) ||
            (!strcmp(furi_string_get_cstr(temp_str), SUBGHZ_RAW_FILE_TYPE))) &&
           temp_data32 == SUBGHZ_KEY_FILE_VERSION) {
        } else {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Type or version mismatch");
            break;
        }
        if(!flipper_format_read_uint32(fff_file, "Frequency", &frequency, 1)) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Missing Frequency");
            break;
        }

        if(!flipper_format_read_string(fff_file, "Preset", temp_str)) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Missing Preset");
            break;
        }

        FuriHalSubGhzPreset preset = js_subghz_get_preset_name(furi_string_get_cstr(temp_str));
        if(preset == FuriHalSubGhzPresetIDLE) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Unknown preset");
            break;
        }

        subghz_devices_reset(js_subghz->radio_device);
        subghz_devices_idle(js_subghz->radio_device);

        if(preset == FuriHalSubGhzPresetCustom) {
            uint8_t* custom_preset_data;
            if(!flipper_format_get_value_count(fff_file, "Custom_preset_data", &temp_data32) ||
               !temp_data32 || (temp_data32 % 2)) {
                mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Custom_preset_data size error");
                break;
            }
            custom_preset_data = malloc(temp_data32);
            if(!flipper_format_read_hex(
                   fff_file, "Custom_preset_data", custom_preset_data, temp_data32)) {
                mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Custom_preset_data read error");
                break;
            }
            subghz_devices_load_preset(js_subghz->radio_device, preset, custom_preset_data);
            free(custom_preset_data);
        } else {
            subghz_devices_load_preset(js_subghz->radio_device, preset, NULL);
        }

        js_subghz->frequency = subghz_devices_set_frequency(js_subghz->radio_device, frequency);

        if(!flipper_format_read_string(fff_file, "Protocol", temp_str)) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Missing protocol");
            break;
        }

        bool is_raw = furi_string_equal(temp_str, "RAW");
        if(is_raw) {
            fff_raw = flipper_format_string_alloc();
            subghz_protocol_raw_gen_fff_data(
                fff_raw, file_path, subghz_devices_get_name(js_subghz->radio_device));
            // One repeat by default
            if(!repeat) {
                repeat = 1;
            }
        } else {
            // Simulate holding button by default
            if(!repeat) {
                if(furi_string_equal(temp_str, "CAME Atomo") ||
                   furi_string_equal(temp_str, "CAME TWEE") ||
                   furi_string_equal(temp_str, "Hormann HSM") ||
                   furi_string_equal(temp_str, "Nice FloR-S") ||
                   furi_string_equal(temp_str, "Power Smart")) {
                    // These protocols send multiple frames/packets for each "repeat"
                    // Just 1 full repeat should be sufficient
                    repeat = 1;
                } else {
                    repeat = 10;
                }
            }
            // Pass repeat value to protocol layer
            flipper_format_insert_or_update_uint32(fff_file, "Repeat", &repeat, 1);
            // Repeat variable applies to high-level code here, should only loop once
            repeat = 1;
        }

        transmitter = subghz_transmitter_alloc_init(environment, furi_string_get_cstr(temp_str));
        if(!transmitter) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Failed to init transmitter");
            break;
        }

        SubGhzProtocolStatus status =
            subghz_transmitter_deserialize(transmitter, is_raw ? fff_raw : fff_file);
        if(status != SubGhzProtocolStatusOk) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Failed to deserialize protocol");
            break;
        }
        // Must close file here, otherwise RAW protocol cannot open
        flipper_format_file_close(fff_file);

        if(!js_subghz->is_external) {
            furi_hal_power_suppress_charge_enter();
        }
        subghz_devices_set_tx(js_subghz->radio_device);
        FURI_LOG_I(TAG, "Transmitting file %s", file_path);

        while(repeat) {
            if(!subghz_devices_start_async_tx(
                   js_subghz->radio_device, subghz_transmitter_yield, transmitter)) {
                is_sent = false;
                mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Failed to start async tx");
                break;
            }
            while(!subghz_devices_is_async_complete_tx(js_subghz->radio_device)) {
                furi_delay_ms(100);
            }
            subghz_devices_stop_async_tx(js_subghz->radio_device);
            subghz_transmitter_stop(transmitter);
            is_sent = true;
            repeat--;

            // Only RAW is repeated with this loop, check comments above
            if(!is_raw) {
                break;
            }
            if(repeat) {
                subghz_transmitter_deserialize(transmitter, fff_raw);
                furi_delay_ms(200);
            }
        }

        if(!js_subghz->is_external) {
            furi_hal_power_suppress_charge_exit();
        }
    } while(false);

    subghz_devices_idle(js_subghz->radio_device);
    js_subghz->state = JsSubghzRadioStateIDLE;

    if(transmitter) {
        subghz_transmitter_free(transmitter);
    }
    furi_string_free(temp_str);
    subghz_environment_free(environment);

    if(fff_raw) {
        flipper_format_free(fff_raw);
    }
    flipper_format_free(fff_file);
    furi_record_close(RECORD_STORAGE);

    if(is_sent) {
        // Return true for backwards compatibility
        // Now it will just error if something goes wrong, not return false
        mjs_return(mjs, mjs_mk_boolean(mjs, true));
    } else {
        // Broke out of do...while with an mJS error
        mjs_return(mjs, MJS_UNDEFINED);
    }
}

static void js_subghz_setup(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(js_subghz->radio_device) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is already setup");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    js_subghz->radio_device =
        radio_device_loader_set(js_subghz->radio_device, SubGhzRadioDeviceTypeExternalCC1101);

    if(!subghz_devices_is_connect(js_subghz->radio_device)) {
        js_subghz->is_external = true;
    } else {
        js_subghz->is_external = false;
    }

    js_subghz->state = JsSubghzRadioStateIDLE;
    js_subghz->frequency = 433920000;

    subghz_devices_reset(js_subghz->radio_device);
    subghz_devices_idle(js_subghz->radio_device);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_subghz_end(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(!js_subghz->radio_device) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not setup");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    subghz_devices_sleep(js_subghz->radio_device);
    radio_device_loader_end(js_subghz->radio_device);
    js_subghz->radio_device = NULL;

    js_subghz->is_external = false;
    js_subghz->state = -1;
    js_subghz->frequency = 0;

    mjs_return(mjs, MJS_UNDEFINED);
}

static void* js_subghz_create(struct mjs* mjs, mjs_val_t* object) {
    JsSubghzInst* js_subghz = malloc(sizeof(JsSubghzInst));
    mjs_val_t subghz_obj = mjs_mk_object(mjs);

    subghz_devices_init();

    mjs_set(mjs, subghz_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, js_subghz));
    mjs_set(mjs, subghz_obj, "setup", ~0, MJS_MK_FN(js_subghz_setup));
    mjs_set(mjs, subghz_obj, "end", ~0, MJS_MK_FN(js_subghz_end));
    mjs_set(mjs, subghz_obj, "setRx", ~0, MJS_MK_FN(js_subghz_set_rx));
    mjs_set(mjs, subghz_obj, "setIdle", ~0, MJS_MK_FN(js_subghz_set_idle));
    mjs_set(mjs, subghz_obj, "getRssi", ~0, MJS_MK_FN(js_subghz_get_rssi));
    mjs_set(mjs, subghz_obj, "getState", ~0, MJS_MK_FN(js_subghz_get_state));
    mjs_set(mjs, subghz_obj, "getFrequency", ~0, MJS_MK_FN(js_subghz_get_frequency));
    mjs_set(mjs, subghz_obj, "setFrequency", ~0, MJS_MK_FN(js_subghz_set_frequency));
    mjs_set(mjs, subghz_obj, "isExternal", ~0, MJS_MK_FN(js_subghz_is_external));
    mjs_set(mjs, subghz_obj, "transmitFile", ~0, MJS_MK_FN(js_subghz_transmit_file));

    *object = subghz_obj;

    return js_subghz;
}

static void js_subghz_destroy(void* inst) {
    JsSubghzInst* js_subghz = inst;

    if(js_subghz->radio_device) {
        subghz_devices_sleep(js_subghz->radio_device);
        radio_device_loader_end(js_subghz->radio_device);
    }

    subghz_devices_deinit();

    free(js_subghz);
}

static const JsModuleDescriptor js_subghz_desc = {
    "subghz",
    js_subghz_create,
    js_subghz_destroy,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_subghz_desc,
};

const FlipperAppPluginDescriptor* js_subghz_ep(void) {
    return &plugin_descriptor;
}
