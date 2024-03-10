#include "../../js_modules.h"
#include "radio_device_loader.h"

#include <lib/subghz/transmitter.h>
#include <lib/subghz/devices/devices.h>
#include <lib/subghz/protocols/protocol_items.h>

#include <flipper_format/flipper_format_i.h>

#define tag "js_subghz"

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

// from subghz cli
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
        FURI_LOG_I(tag, "unknown preset");
    }
    return preset;
}

static int32_t get_int_arg(struct mjs* mjs, size_t index, int32_t* value) {
    mjs_val_t int_obj = mjs_arg(mjs, index);
    if(!mjs_is_number(int_obj)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "Argument must be a number");
        mjs_return(mjs, MJS_UNDEFINED);
        return false;
    }
    *value = mjs_get_int(mjs, int_obj);
    return true;
}

static void js_subghz_set_rx(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(js_subghz->state == JsSubghzRadioStateRX) {
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    subghz_devices_set_rx(js_subghz->radio_device);
    js_subghz->state = JsSubghzRadioStateRX;
}

static void js_subgjz_set_idle(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(js_subghz->state == JsSubghzRadioStateIDLE) {
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    subghz_devices_idle(js_subghz->radio_device);
    js_subghz->state = JsSubghzRadioStateIDLE;
}

static void js_subghz_get_rssi(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

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

    mjs_return(mjs, mjs_mk_boolean(mjs, js_subghz->is_external));
}

static void js_subghz_set_frequency(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

    if(js_subghz->state != JsSubghzRadioStateIDLE) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Radio is not in IDLE state");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    int32_t frequency;
    if(!get_int_arg(mjs, 0, &frequency)) return;

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

    mjs_return(mjs, mjs_mk_number(mjs, (double)js_subghz->frequency));
}

static void js_subghz_transmit_file(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

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

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_file = flipper_format_file_alloc(storage);
    FlipperFormat* fff_data_raw = flipper_format_string_alloc();

    if(!flipper_format_file_open_existing(fff_file, file_path)) {
        flipper_format_free(fff_file);
        furi_record_close(RECORD_STORAGE);
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Failed to open file");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    SubGhzEnvironment* environment = subghz_environment_alloc();
    if(!subghz_environment_load_keystore(environment, SUBGHZ_KEYSTORE_DIR_NAME)) {
        FURI_LOG_I(tag, "Load_keystore keeloq_mfcodes \033[0;31mERROR\033[0m\r\n");
    }
    if(!subghz_environment_load_keystore(environment, SUBGHZ_KEYSTORE_DIR_USER_NAME)) {
        FURI_LOG_I(tag, "Load_keystore keeloq_mfcodes_user \033[0;33mAbsent\033[0m\r\n");
    }
    subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
        environment, SUBGHZ_ALUTECH_AT_4N_DIR_NAME);
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        environment, SUBGHZ_NICE_FLOR_S_DIR_NAME);
    subghz_environment_set_protocol_registry(environment, (void*)&subghz_protocol_registry);

    FuriString* temp_str = furi_string_alloc();
    SubGhzTransmitter* transmitter = NULL;
    bool is_init_protocol = true;
    bool is_sent = false;
    uint32_t frequency = 0;
    uint32_t repeat = 10;

    do {
        //Load frequency
        if(!flipper_format_read_uint32(fff_file, "Frequency", &frequency, 1)) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Failed to read frequency from file");
            mjs_return(mjs, MJS_UNDEFINED);
            break;
        }

        if(!subghz_devices_is_frequency_valid(js_subghz->radio_device, frequency)) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Invalid frequency");
            mjs_return(mjs, MJS_UNDEFINED);
            break;
        }

        if(!flipper_format_read_string(fff_file, "Preset", temp_str)) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Failed to read preset from file");
            mjs_return(mjs, MJS_UNDEFINED);
            break;
        }

        subghz_devices_reset(js_subghz->radio_device);

        if(!strcmp(furi_string_get_cstr(temp_str), "FuriHalSubGhzPresetCustom")) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Custom presets are not supported (yet)");
            mjs_return(mjs, MJS_UNDEFINED);
            break;
        } else {
            subghz_devices_load_preset(
                js_subghz->radio_device,
                js_subghz_get_preset_name(furi_string_get_cstr(temp_str)),
                NULL);
        }

        js_subghz->frequency = subghz_devices_set_frequency(js_subghz->radio_device, frequency);

        if(!flipper_format_read_string(fff_file, "Protocol", temp_str)) {
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Failed to read protocol from file");
            mjs_return(mjs, MJS_UNDEFINED);
            break;
        }

        SubGhzProtocolStatus status;
        bool is_raw = false;

        if(!strcmp(furi_string_get_cstr(temp_str), "RAW")) {
            subghz_protocol_raw_gen_fff_data(
                fff_data_raw, file_path, subghz_devices_get_name(js_subghz->radio_device));
            is_raw = true;
        }

        transmitter = subghz_transmitter_alloc_init(environment, furi_string_get_cstr(temp_str));
        if(transmitter == NULL) {
            is_init_protocol = false;
        }

        if(is_init_protocol) {
            status = subghz_transmitter_deserialize(transmitter, is_raw ? fff_data_raw : fff_file);
            if(status != SubGhzProtocolStatusOk) {
                FURI_LOG_I(tag, "failed to deserialize transmitter");
                is_init_protocol = false;
            }
        } else {
            FURI_LOG_I(tag, "failed to allocate transmitter");
            subghz_devices_idle(js_subghz->radio_device);
            js_subghz->state = JsSubghzRadioStateIDLE;
        }
    } while(false);

    if(is_init_protocol) {
        if(!js_subghz->is_external) {
            furi_hal_power_suppress_charge_enter();
        }

        FURI_LOG_I(tag, "transmitting file %s", file_path);

        do {
            furi_delay_ms(200);
            if(subghz_devices_start_async_tx(
                   js_subghz->radio_device, subghz_transmitter_yield, transmitter)) {
                while(!subghz_devices_is_async_complete_tx(js_subghz->radio_device)) {
                    furi_delay_ms(333);
                }
                subghz_devices_stop_async_tx(js_subghz->radio_device);
                is_sent = true;
            } else {
                FURI_LOG_E(tag, "failed to start async tx");
            }

        } while(repeat && !strcmp(furi_string_get_cstr(temp_str), "RAW"));

        subghz_devices_idle(js_subghz->radio_device);
        js_subghz->state = JsSubghzRadioStateIDLE;

        if(!js_subghz->is_external) {
            furi_hal_power_suppress_charge_exit();
        }
    }

    furi_string_free(temp_str);
    flipper_format_free(fff_file);
    flipper_format_free(fff_data_raw);
    furi_record_close(RECORD_STORAGE);

    subghz_environment_reset_keeloq(environment);
    subghz_environment_free(environment);
    subghz_transmitter_free(transmitter);

    mjs_return(mjs, mjs_mk_boolean(mjs, is_sent));
}

static void js_subghz_setup(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSubghzInst* js_subghz = mjs_get_ptr(mjs, obj_inst);
    furi_assert(js_subghz);

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

static void* js_subghz_create(struct mjs* mjs, mjs_val_t* object) {
    JsSubghzInst* js_subghz = malloc(sizeof(JsSubghzInst));
    mjs_val_t subghz_obj = mjs_mk_object(mjs);

    subghz_devices_init();

    mjs_set(mjs, subghz_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, js_subghz));
    mjs_set(mjs, subghz_obj, "setup", ~0, MJS_MK_FN(js_subghz_setup));
    mjs_set(mjs, subghz_obj, "setRx", ~0, MJS_MK_FN(js_subghz_set_rx));
    mjs_set(mjs, subghz_obj, "setIdle", ~0, MJS_MK_FN(js_subgjz_set_idle));
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
