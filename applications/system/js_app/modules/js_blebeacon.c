#include "../js_modules.h"
#include <furi_hal_bt.h>
#include <extra_beacon.h>

typedef struct {
    bool saved_prev_cfg;
    bool prev_cfg_set;
    GapExtraBeaconConfig prev_cfg;

    bool saved_prev_data;
    uint8_t prev_data[EXTRA_BEACON_MAX_DATA_SIZE];
    uint8_t prev_data_len;

    bool saved_prev_active;
    bool prev_active;

    bool keep_alive;
} JsBlebeaconInst;

static JsBlebeaconInst* get_this_ctx(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsBlebeaconInst* blebeacon = mjs_get_ptr(mjs, obj_inst);
    furi_assert(blebeacon);
    return blebeacon;
}

static void ret_bad_args(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "%s", error);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void ret_int_err(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "%s", error);
    mjs_return(mjs, MJS_UNDEFINED);
}

static bool check_arg_count(struct mjs* mjs, size_t count) {
    size_t num_args = mjs_nargs(mjs);
    if(num_args != count) {
        ret_bad_args(mjs, "Wrong argument count");
        return false;
    }
    return true;
}

static void js_blebeacon_is_active(struct mjs* mjs) {
    JsBlebeaconInst* blebeacon = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;
    UNUSED(blebeacon);

    mjs_return(mjs, mjs_mk_boolean(mjs, furi_hal_bt_extra_beacon_is_active()));
}

static void js_blebeacon_set_config(struct mjs* mjs) {
    JsBlebeaconInst* blebeacon = get_this_ctx(mjs);
    if(mjs_nargs(mjs) < 1 || mjs_nargs(mjs) > 4) {
        ret_bad_args(mjs, "Wrong argument count");
        return;
    }

    char* mac = NULL;
    size_t mac_len = 0;
    mjs_val_t mac_arg = mjs_arg(mjs, 0);
    if(mjs_is_typed_array(mac_arg)) {
        if(mjs_is_data_view(mac_arg)) {
            mac_arg = mjs_dataview_get_buf(mjs, mac_arg);
        }
        mac = mjs_array_buf_get_ptr(mjs, mac_arg, &mac_len);
    }
    if(!mac || mac_len != EXTRA_BEACON_MAC_ADDR_SIZE) {
        ret_bad_args(mjs, "Wrong MAC address");
        return;
    }

    uint8_t power = GapAdvPowerLevel_0dBm;
    mjs_val_t power_arg = mjs_arg(mjs, 1);
    if(mjs_is_number(power_arg)) {
        power = mjs_get_int32(mjs, power_arg);
    }
    power = CLAMP(power, GapAdvPowerLevel_6dBm, GapAdvPowerLevel_Neg40dBm);

    uint8_t intv_min = 50;
    mjs_val_t intv_min_arg = mjs_arg(mjs, 2);
    if(mjs_is_number(intv_min_arg)) {
        intv_min = mjs_get_int32(mjs, intv_min_arg);
    }
    intv_min = MAX(intv_min, 20);

    uint8_t intv_max = 150;
    mjs_val_t intv_max_arg = mjs_arg(mjs, 3);
    if(mjs_is_number(intv_max_arg)) {
        intv_max = mjs_get_int32(mjs, intv_max_arg);
    }
    intv_max = MAX(intv_max, intv_min);

    GapExtraBeaconConfig config = {
        .min_adv_interval_ms = intv_min,
        .max_adv_interval_ms = intv_max,
        .adv_channel_map = GapAdvChannelMapAll,
        .adv_power_level = power,
        .address_type = GapAddressTypePublic,
    };
    memcpy(config.address, (uint8_t*)mac, sizeof(config.address));

    if(!blebeacon->saved_prev_cfg) {
        blebeacon->saved_prev_cfg = true;
        const GapExtraBeaconConfig* prev_cfg_ptr = furi_hal_bt_extra_beacon_get_config();
        if(prev_cfg_ptr) {
            blebeacon->prev_cfg_set = true;
            memcpy(&blebeacon->prev_cfg, prev_cfg_ptr, sizeof(blebeacon->prev_cfg));
        } else {
            blebeacon->prev_cfg_set = false;
        }
    }
    if(!furi_hal_bt_extra_beacon_set_config(&config)) {
        ret_int_err(mjs, "Failed setting beacon config");
        return;
    }

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_blebeacon_set_data(struct mjs* mjs) {
    JsBlebeaconInst* blebeacon = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 1)) return;

    char* data = NULL;
    size_t data_len = 0;
    mjs_val_t data_arg = mjs_arg(mjs, 0);
    if(mjs_is_typed_array(data_arg)) {
        if(mjs_is_data_view(data_arg)) {
            data_arg = mjs_dataview_get_buf(mjs, data_arg);
        }
        data = mjs_array_buf_get_ptr(mjs, data_arg, &data_len);
    }
    if(!data) {
        ret_bad_args(mjs, "Data must be a Uint8Array");
        return;
    }

    if(!blebeacon->saved_prev_data) {
        blebeacon->saved_prev_data = true;
        blebeacon->prev_data_len = furi_hal_bt_extra_beacon_get_data(blebeacon->prev_data);
    }
    if(!furi_hal_bt_extra_beacon_set_data((uint8_t*)data, data_len)) {
        ret_int_err(mjs, "Failed setting beacon data");
        return;
    }

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_blebeacon_start(struct mjs* mjs) {
    JsBlebeaconInst* blebeacon = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;

    if(!blebeacon->saved_prev_active) {
        blebeacon->saved_prev_active = true;
        blebeacon->prev_active = furi_hal_bt_extra_beacon_is_active();
    }
    if(!furi_hal_bt_extra_beacon_start()) {
        ret_int_err(mjs, "Failed starting beacon");
        return;
    }

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_blebeacon_stop(struct mjs* mjs) {
    JsBlebeaconInst* blebeacon = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 0)) return;
    UNUSED(blebeacon);

    if(!blebeacon->saved_prev_active) {
        blebeacon->saved_prev_active = true;
        blebeacon->prev_active = furi_hal_bt_extra_beacon_is_active();
    }
    if(!furi_hal_bt_extra_beacon_stop()) {
        ret_int_err(mjs, "Failed stopping beacon");
        return;
    }

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_blebeacon_keep_alive(struct mjs* mjs) {
    JsBlebeaconInst* blebeacon = get_this_ctx(mjs);
    if(!check_arg_count(mjs, 1)) return;

    mjs_val_t bool_obj = mjs_arg(mjs, 0);
    blebeacon->keep_alive = mjs_get_bool(mjs, bool_obj);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void* js_blebeacon_create(struct mjs* mjs, mjs_val_t* object) {
    JsBlebeaconInst* blebeacon = malloc(sizeof(JsBlebeaconInst));
    mjs_val_t blebeacon_obj = mjs_mk_object(mjs);
    mjs_set(mjs, blebeacon_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, blebeacon));
    mjs_set(mjs, blebeacon_obj, "isActive", ~0, MJS_MK_FN(js_blebeacon_is_active));
    mjs_set(mjs, blebeacon_obj, "setConfig", ~0, MJS_MK_FN(js_blebeacon_set_config));
    mjs_set(mjs, blebeacon_obj, "setData", ~0, MJS_MK_FN(js_blebeacon_set_data));
    mjs_set(mjs, blebeacon_obj, "start", ~0, MJS_MK_FN(js_blebeacon_start));
    mjs_set(mjs, blebeacon_obj, "stop", ~0, MJS_MK_FN(js_blebeacon_stop));
    mjs_set(mjs, blebeacon_obj, "keepAlive", ~0, MJS_MK_FN(js_blebeacon_keep_alive));
    *object = blebeacon_obj;
    return blebeacon;
}

static void js_blebeacon_destroy(void* inst) {
    JsBlebeaconInst* blebeacon = inst;
    if(!blebeacon->keep_alive) {
        if(furi_hal_bt_extra_beacon_is_active()) {
            furi_check(furi_hal_bt_extra_beacon_stop());
        }
        if(blebeacon->saved_prev_cfg && blebeacon->prev_cfg_set) {
            furi_check(furi_hal_bt_extra_beacon_set_config(&blebeacon->prev_cfg));
        }
        if(blebeacon->saved_prev_data) {
            furi_check(
                furi_hal_bt_extra_beacon_set_data(blebeacon->prev_data, blebeacon->prev_data_len));
        }
        if(blebeacon->prev_active) {
            furi_check(furi_hal_bt_extra_beacon_start());
        }
    }
    free(blebeacon);
}

static const JsModuleDescriptor js_blebeacon_desc = {
    "blebeacon",
    js_blebeacon_create,
    js_blebeacon_destroy,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_blebeacon_desc,
};

const FlipperAppPluginDescriptor* js_blebeacon_ep(void) {
    return &plugin_descriptor;
}
