#include "../js_modules.h"
#include <furi_hal_i2c.h>

static void ret_bad_args(struct mjs* mjs, const char* error) {
    mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "%s", error);
    mjs_return(mjs, MJS_UNDEFINED);
}

static bool check_arg_count_range(struct mjs* mjs, size_t min_count, size_t max_count) {
    size_t num_args = mjs_nargs(mjs);
    if(num_args < min_count || num_args > max_count) {
        ret_bad_args(mjs, "Wrong argument count");
        return false;
    }
    return true;
}

static void js_i2c_is_device_ready(struct mjs* mjs) {
    if(!check_arg_count_range(mjs, 1, 2)) return;

    mjs_val_t addr_arg = mjs_arg(mjs, 0);
    if(!mjs_is_number(addr_arg)) {
        ret_bad_args(mjs, "Addr must be a number");
        return;
    }
    uint32_t addr = mjs_get_int32(mjs, addr_arg);

    uint32_t timeout = 1;
    if(mjs_nargs(mjs) > 1) { // Timeout is optional argument
        mjs_val_t timeout_arg = mjs_arg(mjs, 1);
        if(!mjs_is_number(timeout_arg)) {
            ret_bad_args(mjs, "Timeout must be a number");
            return;
        }
        timeout = mjs_get_int32(mjs, timeout_arg);
    }

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    bool ready = furi_hal_i2c_is_device_ready(&furi_hal_i2c_handle_external, addr, timeout);
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);

    mjs_return(mjs, mjs_mk_boolean(mjs, ready));
}

static void js_i2c_write(struct mjs* mjs) {
    if(!check_arg_count_range(mjs, 2, 3)) return;

    mjs_val_t addr_arg = mjs_arg(mjs, 0);
    if(!mjs_is_number(addr_arg)) {
        ret_bad_args(mjs, "Addr must be a number");
        return;
    }
    uint32_t addr = mjs_get_int32(mjs, addr_arg);

    mjs_val_t tx_buf_arg = mjs_arg(mjs, 1);
    bool tx_buf_was_allocated = false;
    uint8_t* tx_buf = NULL;
    size_t tx_len = 0;
    if(mjs_is_array(tx_buf_arg)) {
        tx_len = mjs_array_length(mjs, tx_buf_arg);
        if(tx_len == 0) {
            ret_bad_args(mjs, "Data array must not be empty");
            return;
        }
        tx_buf = malloc(tx_len);
        tx_buf_was_allocated = true;
        for(size_t i = 0; i < tx_len; i++) {
            mjs_val_t val = mjs_array_get(mjs, tx_buf_arg, i);
            if(!mjs_is_number(val)) {
                ret_bad_args(mjs, "Data array must contain only numbers");
                free(tx_buf);
                return;
            }
            uint32_t byte_val = mjs_get_int32(mjs, val);
            if(byte_val > 0xFF) {
                ret_bad_args(mjs, "Data array values must be 0-255");
                free(tx_buf);
                return;
            }
            tx_buf[i] = byte_val;
        }
    } else if(mjs_is_typed_array(tx_buf_arg)) {
        mjs_val_t array_buf = tx_buf_arg;
        if(mjs_is_data_view(tx_buf_arg)) {
            array_buf = mjs_dataview_get_buf(mjs, tx_buf_arg);
        }
        tx_buf = (uint8_t*)mjs_array_buf_get_ptr(mjs, array_buf, &tx_len);
        if(tx_len == 0) {
            ret_bad_args(mjs, "Data array must not be empty");
            return;
        }
    } else {
        ret_bad_args(mjs, "Data must be an array, arraybuf or dataview");
        return;
    }

    uint32_t timeout = 1;
    if(mjs_nargs(mjs) > 2) { // Timeout is optional argument
        mjs_val_t timeout_arg = mjs_arg(mjs, 2);
        if(!mjs_is_number(timeout_arg)) {
            ret_bad_args(mjs, "Timeout must be a number");
            if(tx_buf_was_allocated) free(tx_buf);
            return;
        }
        timeout = mjs_get_int32(mjs, timeout_arg);
    }

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    bool result = furi_hal_i2c_tx(&furi_hal_i2c_handle_external, addr, tx_buf, tx_len, timeout);
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);

    if(tx_buf_was_allocated) free(tx_buf);
    mjs_return(mjs, mjs_mk_boolean(mjs, result));
}

static void js_i2c_read(struct mjs* mjs) {
    if(!check_arg_count_range(mjs, 2, 3)) return;

    mjs_val_t addr_arg = mjs_arg(mjs, 0);
    if(!mjs_is_number(addr_arg)) {
        ret_bad_args(mjs, "Addr must be a number");
        return;
    }
    uint32_t addr = mjs_get_int32(mjs, addr_arg);

    mjs_val_t rx_len_arg = mjs_arg(mjs, 1);
    if(!mjs_is_number(rx_len_arg)) {
        ret_bad_args(mjs, "Length must be a number");
        return;
    }
    size_t rx_len = mjs_get_int32(mjs, rx_len_arg);
    if(rx_len == 0) {
        ret_bad_args(mjs, "Length must not zero");
        return;
    }
    uint8_t* rx_buf = malloc(rx_len);

    uint32_t timeout = 1;
    if(mjs_nargs(mjs) > 2) { // Timeout is optional argument
        mjs_val_t timeout_arg = mjs_arg(mjs, 2);
        if(!mjs_is_number(timeout_arg)) {
            ret_bad_args(mjs, "Timeout must be a number");
            free(rx_buf);
            return;
        }
        timeout = mjs_get_int32(mjs, timeout_arg);
    }

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    bool result = furi_hal_i2c_rx(&furi_hal_i2c_handle_external, addr, rx_buf, rx_len, timeout);
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);

    mjs_val_t ret = MJS_UNDEFINED;
    if(result) {
        ret = mjs_mk_array_buf(mjs, (char*)rx_buf, rx_len);
    }
    free(rx_buf);
    mjs_return(mjs, ret);
}

static void js_i2c_write_read(struct mjs* mjs) {
    if(!check_arg_count_range(mjs, 3, 4)) return;

    mjs_val_t addr_arg = mjs_arg(mjs, 0);
    if(!mjs_is_number(addr_arg)) {
        ret_bad_args(mjs, "Addr must be a number");
        return;
    }
    uint32_t addr = mjs_get_int32(mjs, addr_arg);

    mjs_val_t tx_buf_arg = mjs_arg(mjs, 1);
    bool tx_buf_was_allocated = false;
    uint8_t* tx_buf = NULL;
    size_t tx_len = 0;
    if(mjs_is_array(tx_buf_arg)) {
        tx_len = mjs_array_length(mjs, tx_buf_arg);
        if(tx_len == 0) {
            ret_bad_args(mjs, "Data array must not be empty");
            return;
        }
        tx_buf = malloc(tx_len);
        tx_buf_was_allocated = true;
        for(size_t i = 0; i < tx_len; i++) {
            mjs_val_t val = mjs_array_get(mjs, tx_buf_arg, i);
            if(!mjs_is_number(val)) {
                ret_bad_args(mjs, "Data array must contain only numbers");
                free(tx_buf);
                return;
            }
            uint32_t byte_val = mjs_get_int32(mjs, val);
            if(byte_val > 0xFF) {
                ret_bad_args(mjs, "Data array values must be 0-255");
                free(tx_buf);
                return;
            }
            tx_buf[i] = byte_val;
        }
    } else if(mjs_is_typed_array(tx_buf_arg)) {
        mjs_val_t array_buf = tx_buf_arg;
        if(mjs_is_data_view(tx_buf_arg)) {
            array_buf = mjs_dataview_get_buf(mjs, tx_buf_arg);
        }
        tx_buf = (uint8_t*)mjs_array_buf_get_ptr(mjs, array_buf, &tx_len);
        if(tx_len == 0) {
            ret_bad_args(mjs, "Data array must not be empty");
            return;
        }
    } else {
        ret_bad_args(mjs, "Data must be an array, arraybuf or dataview");
        return;
    }

    mjs_val_t rx_len_arg = mjs_arg(mjs, 2);
    if(!mjs_is_number(rx_len_arg)) {
        ret_bad_args(mjs, "Length must be a number");
        if(tx_buf_was_allocated) free(tx_buf);
        return;
    }
    size_t rx_len = mjs_get_int32(mjs, rx_len_arg);
    if(rx_len == 0) {
        ret_bad_args(mjs, "Length must not zero");
        if(tx_buf_was_allocated) free(tx_buf);
        return;
    }
    uint8_t* rx_buf = malloc(rx_len);

    uint32_t timeout = 1;
    if(mjs_nargs(mjs) > 3) { // Timeout is optional argument
        mjs_val_t timeout_arg = mjs_arg(mjs, 3);
        if(!mjs_is_number(timeout_arg)) {
            ret_bad_args(mjs, "Timeout must be a number");
            if(tx_buf_was_allocated) free(tx_buf);
            free(rx_buf);
            return;
        }
        timeout = mjs_get_int32(mjs, timeout_arg);
    }

    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    bool result = furi_hal_i2c_trx(
        &furi_hal_i2c_handle_external, addr, tx_buf, tx_len, rx_buf, rx_len, timeout);
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);

    mjs_val_t ret = MJS_UNDEFINED;
    if(result) {
        ret = mjs_mk_array_buf(mjs, (char*)rx_buf, rx_len);
    }
    if(tx_buf_was_allocated) free(tx_buf);
    free(rx_buf);
    mjs_return(mjs, ret);
}

static void* js_i2c_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    UNUSED(modules);
    mjs_val_t i2c_obj = mjs_mk_object(mjs);
    mjs_set(mjs, i2c_obj, "isDeviceReady", ~0, MJS_MK_FN(js_i2c_is_device_ready));
    mjs_set(mjs, i2c_obj, "write", ~0, MJS_MK_FN(js_i2c_write));
    mjs_set(mjs, i2c_obj, "read", ~0, MJS_MK_FN(js_i2c_read));
    mjs_set(mjs, i2c_obj, "writeRead", ~0, MJS_MK_FN(js_i2c_write_read));
    *object = i2c_obj;

    return (void*)1;
}

static const JsModuleDescriptor js_i2c_desc = {
    "i2c",
    js_i2c_create,
    NULL,
    NULL,
};

static const FlipperAppPluginDescriptor i2c_plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_i2c_desc,
};

const FlipperAppPluginDescriptor* js_i2c_ep(void) {
    return &i2c_plugin_descriptor;
}
