#include <core/common_defines.h>
#include <expansion/expansion.h>
#include <furi_hal.h>
#include "../js_modules.h"
#include <m-array.h>

#define TAG "JsSerial"

#define RX_BUF_LEN 2048

typedef struct {
    bool setup_done;
    FuriStreamBuffer* rx_stream;
    FuriHalSerialHandle* serial_handle;
    struct mjs* mjs;
} JsSerialInst;

typedef struct {
    size_t len;
    char* data;
} PatternArrayItem;

static const struct {
    const char* name;
    const FuriHalSerialId value;
} serial_channels[] = {
    {"usart", FuriHalSerialIdUsart},
    {"lpuart", FuriHalSerialIdLpuart},
};

ARRAY_DEF(PatternArray, PatternArrayItem, M_POD_OPLIST);

static void
    js_serial_on_async_rx(FuriHalSerialHandle* handle, FuriHalSerialRxEvent event, void* context) {
    JsSerialInst* serial = context;
    furi_assert(serial);

    if(event & FuriHalSerialRxEventData) {
        uint8_t data = furi_hal_serial_async_rx(handle);
        furi_stream_buffer_send(serial->rx_stream, &data, 1, 0);
        js_flags_set(serial->mjs, ThreadEventCustomDataRx);
    }
}

static void js_serial_setup(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSerialInst* serial = mjs_get_ptr(mjs, obj_inst);
    furi_assert(serial);

    if(serial->setup_done) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Serial is already configured");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    bool args_correct = false;
    FuriHalSerialId serial_id = FuriHalSerialIdMax;
    uint32_t baudrate = 0;

    do {
        if(mjs_nargs(mjs) != 2) break;

        mjs_val_t arg = mjs_arg(mjs, 0);
        if(!mjs_is_string(arg)) break;

        size_t str_len = 0;
        const char* arg_str = mjs_get_string(mjs, &arg, &str_len);
        for(size_t i = 0; i < COUNT_OF(serial_channels); i++) {
            size_t name_len = strlen(serial_channels[i].name);
            if(str_len != name_len) continue;
            if(strncmp(arg_str, serial_channels[i].name, str_len) == 0) {
                serial_id = serial_channels[i].value;
                break;
            }
        }
        if(serial_id == FuriHalSerialIdMax) {
            break;
        }

        arg = mjs_arg(mjs, 1);
        if(!mjs_is_number(arg)) break;
        baudrate = mjs_get_int32(mjs, arg);

        args_correct = true;
    } while(0);

    if(!args_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    expansion_disable(furi_record_open(RECORD_EXPANSION));
    furi_record_close(RECORD_EXPANSION);

    serial->serial_handle = furi_hal_serial_control_acquire(serial_id);
    if(serial->serial_handle) {
        serial->rx_stream = furi_stream_buffer_alloc(RX_BUF_LEN, 1);
        furi_hal_serial_init(serial->serial_handle, baudrate);
        furi_hal_serial_async_rx_start(
            serial->serial_handle, js_serial_on_async_rx, serial, false);
        serial->setup_done = true;
    } else {
        expansion_enable(furi_record_open(RECORD_EXPANSION));
        furi_record_close(RECORD_EXPANSION);
    }
}

static void js_serial_deinit(JsSerialInst* js_serial) {
    if(js_serial->setup_done) {
        furi_hal_serial_async_rx_stop(js_serial->serial_handle);
        furi_hal_serial_deinit(js_serial->serial_handle);
        furi_hal_serial_control_release(js_serial->serial_handle);
        js_serial->serial_handle = NULL;
        furi_stream_buffer_free(js_serial->rx_stream);

        expansion_enable(furi_record_open(RECORD_EXPANSION));
        furi_record_close(RECORD_EXPANSION);

        js_serial->setup_done = false;
    }
}

static void js_serial_end(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSerialInst* serial = mjs_get_ptr(mjs, obj_inst);
    furi_assert(serial);

    if(!serial->setup_done) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Serial is not configured");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    js_serial_deinit(serial);
}

static void js_serial_write(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSerialInst* serial = mjs_get_ptr(mjs, obj_inst);
    furi_assert(serial);
    if(!serial->setup_done) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Serial is not configured");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    bool args_correct = true;

    size_t num_args = mjs_nargs(mjs);
    for(size_t i = 0; i < num_args; i++) {
        mjs_val_t arg = mjs_arg(mjs, i);
        if(mjs_is_string(arg)) {
            size_t str_len = 0;
            const char* arg_str = mjs_get_string(mjs, &arg, &str_len);
            if((str_len == 0) || (arg_str == NULL)) {
                args_correct = false;
                break;
            }
            furi_hal_serial_tx(serial->serial_handle, (uint8_t*)arg_str, str_len);
        } else if(mjs_is_number(arg)) {
            uint32_t byte_val = mjs_get_int32(mjs, arg);
            if(byte_val > 0xFF) {
                args_correct = false;
                break;
            }
            furi_hal_serial_tx(serial->serial_handle, (uint8_t*)&byte_val, 1);
        } else if(mjs_is_array(arg)) {
            size_t array_len = mjs_array_length(mjs, arg);
            for(size_t i = 0; i < array_len; i++) {
                mjs_val_t array_arg = mjs_array_get(mjs, arg, i);
                if(!mjs_is_number(array_arg)) {
                    args_correct = false;
                    break;
                }
                uint32_t byte_val = mjs_get_int32(mjs, array_arg);
                if(byte_val > 0xFF) {
                    args_correct = false;
                    break;
                }
                furi_hal_serial_tx(serial->serial_handle, (uint8_t*)&byte_val, 1);
            }
            if(!args_correct) {
                break;
            }
        } else if(mjs_is_typed_array(arg)) {
            mjs_val_t array_buf = arg;
            if(mjs_is_data_view(arg)) {
                array_buf = mjs_dataview_get_buf(mjs, arg);
            }
            size_t len = 0;
            char* buf = mjs_array_buf_get_ptr(mjs, array_buf, &len);
            furi_hal_serial_tx(serial->serial_handle, (uint8_t*)buf, len);
        } else {
            args_correct = false;
            break;
        }
    }

    if(!args_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
    }
    mjs_return(mjs, MJS_UNDEFINED);
}

static size_t js_serial_receive(JsSerialInst* serial, char* buf, size_t len, uint32_t timeout) {
    size_t bytes_read = 0;
    while(1) {
        uint32_t flags = ThreadEventCustomDataRx;
        if(furi_stream_buffer_is_empty(serial->rx_stream)) {
            flags = js_flags_wait(serial->mjs, ThreadEventCustomDataRx, timeout);
        }
        if(flags == 0) { // Timeout
            break;
        } else if(flags & ThreadEventStop) { // Exit flag
            bytes_read = 0;
            break;
        } else if(flags & ThreadEventCustomDataRx) { // New data received
            size_t rx_len = furi_stream_buffer_receive(
                serial->rx_stream, &buf[bytes_read], len - bytes_read, 0);
            bytes_read += rx_len;
            if(bytes_read == len) {
                break;
            }
        }
    }
    return bytes_read;
}

static void js_serial_read(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSerialInst* serial = mjs_get_ptr(mjs, obj_inst);
    furi_assert(serial);
    if(!serial->setup_done) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Serial is not configured");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    size_t read_len = 0;
    uint32_t timeout = FuriWaitForever;

    do {
        size_t num_args = mjs_nargs(mjs);
        if(num_args == 1) {
            mjs_val_t arg = mjs_arg(mjs, 0);
            if(!mjs_is_number(arg)) {
                break;
            }
            read_len = mjs_get_int32(mjs, arg);
        } else if(num_args == 2) {
            mjs_val_t len_arg = mjs_arg(mjs, 0);
            mjs_val_t timeout_arg = mjs_arg(mjs, 1);
            if((!mjs_is_number(len_arg)) || (!mjs_is_number(timeout_arg))) {
                break;
            }
            read_len = mjs_get_int32(mjs, len_arg);
            timeout = mjs_get_int32(mjs, timeout_arg);
        }
    } while(0);

    if(read_len == 0) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    char* read_buf = malloc(read_len);
    size_t bytes_read = js_serial_receive(serial, read_buf, read_len, timeout);

    mjs_val_t return_obj = MJS_UNDEFINED;
    if(bytes_read > 0) {
        return_obj = mjs_mk_string(mjs, read_buf, bytes_read, true);
    }
    mjs_return(mjs, return_obj);
    free(read_buf);
}

static void js_serial_readln(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSerialInst* serial = mjs_get_ptr(mjs, obj_inst);
    furi_assert(serial);
    if(!serial->setup_done) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Serial is not configured");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    bool args_correct = false;
    uint32_t timeout = FuriWaitForever;

    do {
        size_t num_args = mjs_nargs(mjs);
        if(num_args > 1) {
            break;
        } else if(num_args == 1) {
            mjs_val_t arg = mjs_arg(mjs, 0);
            if(!mjs_is_number(arg)) {
                break;
            }
            timeout = mjs_get_int32(mjs, arg);
        }
        args_correct = true;
    } while(0);

    if(!args_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }
    FuriString* rx_buf = furi_string_alloc();
    size_t bytes_read = 0;
    char read_char = 0;

    while(1) {
        size_t read_len = js_serial_receive(serial, &read_char, 1, timeout);
        if(read_len != 1) {
            break;
        }
        if((read_char == '\r') || (read_char == '\n')) {
            break;
        } else {
            furi_string_push_back(rx_buf, read_char);
            bytes_read++;
        }
    }

    mjs_val_t return_obj = MJS_UNDEFINED;
    if(bytes_read > 0) {
        return_obj = mjs_mk_string(mjs, furi_string_get_cstr(rx_buf), bytes_read, true);
    }
    mjs_return(mjs, return_obj);
    furi_string_free(rx_buf);
}

static void js_serial_read_bytes(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSerialInst* serial = mjs_get_ptr(mjs, obj_inst);
    furi_assert(serial);
    if(!serial->setup_done) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Serial is not configured");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    size_t read_len = 0;
    uint32_t timeout = FuriWaitForever;

    do {
        size_t num_args = mjs_nargs(mjs);
        if(num_args == 1) {
            mjs_val_t arg = mjs_arg(mjs, 0);
            if(!mjs_is_number(arg)) {
                break;
            }
            read_len = mjs_get_int32(mjs, arg);
        } else if(num_args == 2) {
            mjs_val_t len_arg = mjs_arg(mjs, 0);
            mjs_val_t timeout_arg = mjs_arg(mjs, 1);
            if((!mjs_is_number(len_arg)) || (!mjs_is_number(timeout_arg))) {
                break;
            }
            read_len = mjs_get_int32(mjs, len_arg);
            timeout = mjs_get_int32(mjs, timeout_arg);
        }
    } while(0);

    if(read_len == 0) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    char* read_buf = malloc(read_len);
    size_t bytes_read = js_serial_receive(serial, read_buf, read_len, timeout);

    mjs_val_t return_obj = MJS_UNDEFINED;
    if(bytes_read > 0) {
        return_obj = mjs_mk_array_buf(mjs, read_buf, bytes_read);
    }
    mjs_return(mjs, return_obj);
    free(read_buf);
}

static char* js_serial_receive_any(JsSerialInst* serial, size_t* len, uint32_t timeout) {
    uint32_t flags = ThreadEventCustomDataRx;
    if(furi_stream_buffer_is_empty(serial->rx_stream)) {
        flags = js_flags_wait(serial->mjs, ThreadEventCustomDataRx, timeout);
    }
    if(flags & ThreadEventCustomDataRx) { // New data received
        *len = furi_stream_buffer_bytes_available(serial->rx_stream);
        if(!*len) return NULL;
        char* buf = malloc(*len);
        furi_stream_buffer_receive(serial->rx_stream, buf, *len, 0);
        return buf;
    }
    return NULL;
}

static void js_serial_read_any(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSerialInst* serial = mjs_get_ptr(mjs, obj_inst);
    furi_assert(serial);
    if(!serial->setup_done) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Serial is not configured");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    uint32_t timeout = FuriWaitForever;

    do {
        size_t num_args = mjs_nargs(mjs);
        if(num_args == 1) {
            mjs_val_t timeout_arg = mjs_arg(mjs, 0);
            if(!mjs_is_number(timeout_arg)) {
                break;
            }
            timeout = mjs_get_int32(mjs, timeout_arg);
        }
    } while(0);

    size_t bytes_read = 0;
    char* read_buf = js_serial_receive_any(serial, &bytes_read, timeout);

    mjs_val_t return_obj = MJS_UNDEFINED;
    if(bytes_read > 0 && read_buf) {
        return_obj = mjs_mk_string(mjs, read_buf, bytes_read, true);
    }
    mjs_return(mjs, return_obj);
    free(read_buf);
}

static bool
    js_serial_expect_parse_string(struct mjs* mjs, mjs_val_t arg, PatternArray_t patterns) {
    size_t str_len = 0;
    const char* arg_str = mjs_get_string(mjs, &arg, &str_len);
    if((str_len == 0) || (arg_str == NULL)) {
        return false;
    }
    PatternArrayItem* item = PatternArray_push_new(patterns);
    item->data = malloc(str_len + 1);
    memcpy(item->data, arg_str, str_len);
    item->len = str_len;
    return true;
}

static bool js_serial_expect_parse_array(struct mjs* mjs, mjs_val_t arg, PatternArray_t patterns) {
    size_t array_len = mjs_array_length(mjs, arg);
    if(array_len == 0) {
        return false;
    }
    char* array_data = malloc(array_len + 1);

    for(size_t i = 0; i < array_len; i++) {
        mjs_val_t array_arg = mjs_array_get(mjs, arg, i);
        if(!mjs_is_number(array_arg)) {
            free(array_data);
            return false;
        }

        uint32_t byte_val = mjs_get_int32(mjs, array_arg);
        if(byte_val > 0xFF) {
            free(array_data);
            return false;
        }
        array_data[i] = byte_val;
    }

    PatternArrayItem* item = PatternArray_push_new(patterns);
    item->data = array_data;
    item->len = array_len;
    return true;
}

static bool
    js_serial_expect_parse_args(struct mjs* mjs, PatternArray_t patterns, uint32_t* timeout) {
    size_t num_args = mjs_nargs(mjs);
    if(num_args == 2) {
        mjs_val_t timeout_arg = mjs_arg(mjs, 1);
        if(!mjs_is_number(timeout_arg)) {
            return false;
        }
        *timeout = mjs_get_int32(mjs, timeout_arg);
    } else if(num_args != 1) {
        return false;
    }
    mjs_val_t patterns_arg = mjs_arg(mjs, 0);
    if(mjs_is_string(patterns_arg)) { // Single string pattern
        if(!js_serial_expect_parse_string(mjs, patterns_arg, patterns)) {
            return false;
        }
    } else if(mjs_is_array(patterns_arg)) {
        size_t array_len = mjs_array_length(mjs, patterns_arg);
        if(array_len == 0) {
            return false;
        }
        mjs_val_t array_arg = mjs_array_get(mjs, patterns_arg, 0);

        if(mjs_is_number(array_arg)) { // Binary array pattern
            if(!js_serial_expect_parse_array(mjs, patterns_arg, patterns)) {
                return false;
            }
        } else if((mjs_is_string(array_arg)) || (mjs_is_array(array_arg))) { // Multiple patterns
            for(size_t i = 0; i < array_len; i++) {
                mjs_val_t arg = mjs_array_get(mjs, patterns_arg, i);

                if(mjs_is_string(arg)) {
                    if(!js_serial_expect_parse_string(mjs, arg, patterns)) {
                        return false;
                    }
                } else if(mjs_is_array(arg)) {
                    if(!js_serial_expect_parse_array(mjs, arg, patterns)) {
                        return false;
                    }
                }
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

static int32_t js_serial_expect_check_pattern_start(
    PatternArray_t patterns,
    char value,
    int32_t pattern_last) {
    size_t array_len = PatternArray_size(patterns);
    if((pattern_last + 1) >= (int32_t)array_len) {
        return -1;
    }
    for(size_t i = pattern_last + 1; i < array_len; i++) {
        if(PatternArray_get(patterns, i)->data[0] == value) {
            return i;
        }
    }
    return -1;
}

static void js_serial_expect(struct mjs* mjs) {
    mjs_val_t obj_inst = mjs_get(mjs, mjs_get_this(mjs), INST_PROP_NAME, ~0);
    JsSerialInst* serial = mjs_get_ptr(mjs, obj_inst);
    furi_assert(serial);
    if(!serial->setup_done) {
        mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "Serial is not configured");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }

    uint32_t timeout = FuriWaitForever;
    PatternArray_t patterns;
    PatternArray_it_t it;
    PatternArray_init(patterns);

    if(!js_serial_expect_parse_args(mjs, patterns, &timeout)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        for(PatternArray_it(it, patterns); !PatternArray_end_p(it); PatternArray_next(it)) {
            const PatternArrayItem* item = PatternArray_cref(it);
            free(item->data);
        }
        PatternArray_clear(patterns);
        return;
    }

    size_t pattern_len_max = 0;
    for(PatternArray_it(it, patterns); !PatternArray_end_p(it); PatternArray_next(it)) {
        const PatternArrayItem* item = PatternArray_cref(it);
        if(item->len > pattern_len_max) {
            pattern_len_max = item->len;
        }
    }

    char* compare_buf = malloc(pattern_len_max);
    int32_t pattern_found = -1;
    int32_t pattern_candidate = -1;
    size_t buf_len = 0;
    bool is_timeout = false;

    while(1) {
        if(buf_len == 0) {
            // Empty buffer - read by 1 byte to find pattern start
            size_t bytes_read = js_serial_receive(serial, &compare_buf[0], 1, timeout);
            if(bytes_read != 1) {
                is_timeout = true;
                break;
            }
            pattern_candidate = js_serial_expect_check_pattern_start(patterns, compare_buf[0], -1);
            if(pattern_candidate == -1) {
                continue;
            }
            buf_len = 1;
        }
        assert(pattern_candidate >= 0);

        // Read next and try to find pattern match
        PatternArrayItem* pattern_cur = PatternArray_get(patterns, pattern_candidate);
        pattern_found = pattern_candidate;
        for(size_t i = 0; i < pattern_cur->len; i++) {
            if(i >= buf_len) {
                size_t bytes_read = js_serial_receive(serial, &compare_buf[i], 1, timeout);
                if(bytes_read != 1) {
                    is_timeout = true;
                    break;
                }
                buf_len++;
            }
            if(compare_buf[i] != pattern_cur->data[i]) {
                pattern_found = -1;
                break;
            }
        }
        if((is_timeout) || (pattern_found >= 0)) {
            break;
        }

        // Search other patterns with the same start char
        pattern_candidate =
            js_serial_expect_check_pattern_start(patterns, compare_buf[0], pattern_candidate);
        if(pattern_candidate >= 0) {
            continue;
        }

        // Look for another pattern start
        for(size_t i = 1; i < buf_len; i++) {
            pattern_candidate = js_serial_expect_check_pattern_start(patterns, compare_buf[i], -1);
            if(pattern_candidate >= 0) {
                memmove(&compare_buf[0], &compare_buf[i], buf_len - i);
                buf_len -= i;
                break;
            }
        }
        if(pattern_candidate >= 0) {
            continue;
        }
        // Nothing found - reset buffer
        buf_len = 0;
    }

    if(is_timeout) {
        FURI_LOG_W(TAG, "Expect: timeout");
    }

    for(PatternArray_it(it, patterns); !PatternArray_end_p(it); PatternArray_next(it)) {
        const PatternArrayItem* item = PatternArray_cref(it);
        free(item->data);
    }
    PatternArray_clear(patterns);
    free(compare_buf);

    if(pattern_found >= 0) {
        mjs_return(mjs, mjs_mk_number(mjs, pattern_found));
    } else {
        mjs_return(mjs, MJS_UNDEFINED);
    }
}

static void* js_serial_create(struct mjs* mjs, mjs_val_t* object, JsModules* modules) {
    UNUSED(modules);
    JsSerialInst* js_serial = malloc(sizeof(JsSerialInst));
    js_serial->mjs = mjs;
    mjs_val_t serial_obj = mjs_mk_object(mjs);
    mjs_set(mjs, serial_obj, INST_PROP_NAME, ~0, mjs_mk_foreign(mjs, js_serial));
    mjs_set(mjs, serial_obj, "setup", ~0, MJS_MK_FN(js_serial_setup));
    mjs_set(mjs, serial_obj, "end", ~0, MJS_MK_FN(js_serial_end));
    mjs_set(mjs, serial_obj, "write", ~0, MJS_MK_FN(js_serial_write));
    mjs_set(mjs, serial_obj, "read", ~0, MJS_MK_FN(js_serial_read));
    mjs_set(mjs, serial_obj, "readln", ~0, MJS_MK_FN(js_serial_readln));
    mjs_set(mjs, serial_obj, "readBytes", ~0, MJS_MK_FN(js_serial_read_bytes));
    mjs_set(mjs, serial_obj, "readAny", ~0, MJS_MK_FN(js_serial_read_any));
    mjs_set(mjs, serial_obj, "expect", ~0, MJS_MK_FN(js_serial_expect));
    *object = serial_obj;

    return js_serial;
}

static void js_serial_destroy(void* inst) {
    JsSerialInst* js_serial = inst;
    js_serial_deinit(js_serial);
    free(js_serial);
}

static const JsModuleDescriptor js_serial_desc = {
    "serial",
    js_serial_create,
    js_serial_destroy,
    NULL,
};

static const FlipperAppPluginDescriptor plugin_descriptor = {
    .appid = PLUGIN_APP_ID,
    .ep_api_version = PLUGIN_API_VERSION,
    .entry_point = &js_serial_desc,
};

const FlipperAppPluginDescriptor* js_serial_ep(void) {
    return &plugin_descriptor;
}
