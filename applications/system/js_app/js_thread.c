#include <common/cs_dbg.h>
#include <toolbox/path.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/strint.h>
#include <loader/firmware_api/firmware_api.h>
#include <flipper_application/api_hashtable/api_hashtable.h>
#include <flipper_application/plugins/composite_resolver.h>
#include <furi_hal.h>
#include "plugin_api/app_api_interface.h"
#include "js_thread.h"
#include "js_thread_i.h"
#include "js_modules.h"

#define TAG "JS"

struct JsThread {
    FuriThread* thread;
    FuriString* path;
    CompositeApiResolver* resolver;
    JsThreadCallback app_callback;
    void* context;
    JsModules* modules;
};

static void js_str_print(FuriString* msg_str, struct mjs* mjs) {
    size_t num_args = mjs_nargs(mjs);
    for(size_t i = 0; i < num_args; i++) {
        char* name = NULL;
        size_t name_len = 0;
        int need_free = 0;
        mjs_val_t arg = mjs_arg(mjs, i);
        mjs_err_t err = mjs_to_string(mjs, &arg, &name, &name_len, &need_free);
        if(err != MJS_OK) {
            furi_string_cat_printf(msg_str, "err %s ", mjs_strerror(mjs, err));
        } else {
            furi_string_cat_printf(msg_str, "%s ", name);
        }
        if(need_free) {
            free(name);
            name = NULL;
        }
    }
}

static void js_print(struct mjs* mjs) {
    FuriString* msg_str = furi_string_alloc();
    js_str_print(msg_str, mjs);

    JsThread* worker = mjs_get_context(mjs);
    furi_assert(worker);
    if(worker->app_callback) {
        worker->app_callback(JsThreadEventPrint, furi_string_get_cstr(msg_str), worker->context);
    } else {
        FURI_LOG_D(TAG, "%s\r\n", furi_string_get_cstr(msg_str));
    }

    furi_string_free(msg_str);

    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_console_log(struct mjs* mjs) {
    FuriString* msg_str = furi_string_alloc();
    js_str_print(msg_str, mjs);
    FURI_LOG_I(TAG, "%s", furi_string_get_cstr(msg_str));
    furi_string_free(msg_str);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_console_warn(struct mjs* mjs) {
    FuriString* msg_str = furi_string_alloc();
    js_str_print(msg_str, mjs);
    FURI_LOG_W(TAG, "%s", furi_string_get_cstr(msg_str));
    furi_string_free(msg_str);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_console_error(struct mjs* mjs) {
    FuriString* msg_str = furi_string_alloc();
    js_str_print(msg_str, mjs);
    FURI_LOG_E(TAG, "%s", furi_string_get_cstr(msg_str));
    furi_string_free(msg_str);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_console_debug(struct mjs* mjs) {
    FuriString* msg_str = furi_string_alloc();
    js_str_print(msg_str, mjs);
    FURI_LOG_D(TAG, "%s", furi_string_get_cstr(msg_str));
    furi_string_free(msg_str);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void js_exit_flag_poll(struct mjs* mjs) {
    uint32_t flags = furi_thread_flags_wait(ThreadEventStop, FuriFlagWaitAny | FuriFlagNoClear, 0);
    if(flags & FuriFlagError) {
        return;
    }
    if(flags & ThreadEventStop) {
        mjs_exit(mjs);
    }
}

bool js_delay_with_flags(struct mjs* mjs, uint32_t time) {
    uint32_t flags =
        furi_thread_flags_wait(ThreadEventStop, FuriFlagWaitAny | FuriFlagNoClear, time);
    if(flags & FuriFlagError) {
        return false;
    }
    if(flags & ThreadEventStop) {
        mjs_exit(mjs);
        return true;
    }
    return false;
}

void js_flags_set(struct mjs* mjs, uint32_t flags) {
    JsThread* worker = mjs_get_context(mjs);
    furi_assert(worker);
    furi_thread_flags_set(furi_thread_get_id(worker->thread), flags);
}

uint32_t js_flags_wait(struct mjs* mjs, uint32_t flags_mask, uint32_t timeout) {
    flags_mask |= ThreadEventStop;
    uint32_t flags = furi_thread_flags_get();
    furi_check((flags & FuriFlagError) == 0);
    if(flags == 0) {
        flags = furi_thread_flags_wait(flags_mask, FuriFlagWaitAny | FuriFlagNoClear, timeout);
    } else {
        uint32_t state = furi_thread_flags_clear(flags & flags_mask);
        furi_check((state & FuriFlagError) == 0);
    }

    if(flags & FuriFlagError) {
        return 0;
    }
    if(flags & ThreadEventStop) {
        mjs_exit(mjs);
    }
    return flags;
}

static void js_delay(struct mjs* mjs) {
    bool args_correct = false;
    int ms = 0;

    if(mjs_nargs(mjs) == 1) {
        mjs_val_t arg = mjs_arg(mjs, 0);
        if(mjs_is_number(arg)) {
            ms = mjs_get_int(mjs, arg);
            args_correct = true;
        }
    }
    if(!args_correct) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "");
        mjs_return(mjs, MJS_UNDEFINED);
        return;
    }
    js_delay_with_flags(mjs, ms);
    mjs_return(mjs, MJS_UNDEFINED);
}

static void* js_dlsym(void* handle, const char* name) {
    CompositeApiResolver* resolver = handle;
    Elf32_Addr addr = 0;
    uint32_t hash = elf_symbolname_hash(name);
    const ElfApiInterface* api = composite_api_resolver_get(resolver);

    if(!api->resolver_callback(api, hash, &addr)) {
        FURI_LOG_E(TAG, "FFI: cannot find \"%s\"", name);
        return NULL;
    }

    return (void*)addr;
}

static void js_ffi_address(struct mjs* mjs) {
    mjs_val_t name_v = mjs_arg(mjs, 0);
    size_t len;
    const char* name = mjs_get_string(mjs, &name_v, &len);
    void* addr = mjs_ffi_resolve(mjs, name);
    mjs_return(mjs, mjs_mk_foreign(mjs, addr));
}

static void js_require(struct mjs* mjs) {
    mjs_val_t name_v = mjs_arg(mjs, 0);
    size_t len;
    const char* name = mjs_get_string(mjs, &name_v, &len);
    mjs_val_t req_object = MJS_UNDEFINED;
    if((len == 0) || (name == NULL)) {
        mjs_prepend_errorf(mjs, MJS_BAD_ARGS_ERROR, "String argument is expected");
    } else {
        JsThread* worker = mjs_get_context(mjs);
        furi_assert(worker);
        req_object = js_module_require(worker->modules, name, len);
    }
    mjs_return(mjs, req_object);
}

static void js_parse_int(struct mjs* mjs) {
    static const JsValueDeclaration js_parse_int_arg_list[] = {
        JS_VALUE_SIMPLE(JsValueTypeString),
        JS_VALUE_SIMPLE_W_DEFAULT(JsValueTypeInt32, int32_val, 10),
    };
    static const JsValueArguments js_parse_int_args = JS_VALUE_ARGS(js_parse_int_arg_list);

    const char* str;
    int32_t base;
    JS_VALUE_PARSE_ARGS_OR_RETURN(mjs, &js_parse_int_args, &str, &base);

    int32_t num;
    if(strint_to_int32(str, NULL, &num, base) != StrintParseNoError) {
        num = 0;
    }
    mjs_return(mjs, mjs_mk_number(mjs, num));
}

#ifdef JS_DEBUG
static void js_dump_write_callback(void* ctx, const char* format, ...) {
    File* file = ctx;
    furi_assert(ctx);

    FuriString* str = furi_string_alloc();

    va_list args;
    va_start(args, format);
    furi_string_vprintf(str, format, args);
    furi_string_cat(str, "\n");
    va_end(args);

    storage_file_write(file, furi_string_get_cstr(str), furi_string_size(str));
    furi_string_free(str);
}
#endif

static int32_t js_thread(void* arg) {
    JsThread* worker = arg;
    worker->resolver = composite_api_resolver_alloc();
    composite_api_resolver_add(worker->resolver, firmware_api_interface);
    composite_api_resolver_add(worker->resolver, application_api_interface);

    struct mjs* mjs = mjs_create(worker);
    worker->modules = js_modules_create(mjs, worker->resolver);
    mjs_val_t global = mjs_get_global(mjs);
    mjs_val_t console_obj = mjs_mk_object(mjs);

    if(worker->path) {
        FuriString* dirpath = furi_string_alloc();
        path_extract_dirname(furi_string_get_cstr(worker->path), dirpath);
        mjs_set(
            mjs,
            global,
            "__filename",
            ~0,
            mjs_mk_string(
                mjs, furi_string_get_cstr(worker->path), furi_string_size(worker->path), true));
        mjs_set(
            mjs,
            global,
            "__dirname",
            ~0,
            mjs_mk_string(mjs, furi_string_get_cstr(dirpath), furi_string_size(dirpath), true));
        furi_string_free(dirpath);
    }

    JS_ASSIGN_MULTI(mjs, global) {
        JS_FIELD("print", MJS_MK_FN(js_print));
        JS_FIELD("delay", MJS_MK_FN(js_delay));
        JS_FIELD("parseInt", MJS_MK_FN(js_parse_int));
        JS_FIELD("ffi_address", MJS_MK_FN(js_ffi_address));
        JS_FIELD("require", MJS_MK_FN(js_require));
        JS_FIELD("console", console_obj);

        JS_FIELD("sdkCompatibilityStatus", MJS_MK_FN(js_sdk_compatibility_status));
        JS_FIELD("isSdkCompatible", MJS_MK_FN(js_is_sdk_compatible));
        JS_FIELD("checkSdkCompatibility", MJS_MK_FN(js_check_sdk_compatibility));
        JS_FIELD("doesSdkSupport", MJS_MK_FN(js_does_sdk_support));
        JS_FIELD("checkSdkFeatures", MJS_MK_FN(js_check_sdk_features));
    }

    JS_ASSIGN_MULTI(mjs, console_obj) {
        JS_FIELD("log", MJS_MK_FN(js_console_log));
        JS_FIELD("warn", MJS_MK_FN(js_console_warn));
        JS_FIELD("error", MJS_MK_FN(js_console_error));
        JS_FIELD("debug", MJS_MK_FN(js_console_debug));
    }

    mjs_set_ffi_resolver(mjs, js_dlsym, worker->resolver);

    mjs_set_exec_flags_poller(mjs, js_exit_flag_poll);

    mjs_err_t err = mjs_exec_file(mjs, furi_string_get_cstr(worker->path), NULL);

#ifdef JS_DEBUG
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        FuriString* dump_path = furi_string_alloc_set(worker->path);
        furi_string_cat(dump_path, ".lst");

        Storage* storage = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(storage);

        if(storage_file_open(
               file, furi_string_get_cstr(dump_path), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            mjs_disasm_all(mjs, js_dump_write_callback, file);
        }

        storage_file_close(file);
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);

        furi_string_free(dump_path);
    }
#endif

    if(err != MJS_OK) {
        FURI_LOG_E(TAG, "Exec error: %s", mjs_strerror(mjs, err));
        if(worker->app_callback) {
            worker->app_callback(JsThreadEventError, mjs_strerror(mjs, err), worker->context);
        }
        const char* stack_trace = mjs_get_stack_trace(mjs);
        if(stack_trace != NULL) {
            FURI_LOG_E(TAG, "Stack trace:\r\n%s", stack_trace);
            if(worker->app_callback) {
                worker->app_callback(JsThreadEventErrorTrace, stack_trace, worker->context);
            }
        }
    } else {
        if(worker->app_callback) {
            worker->app_callback(JsThreadEventDone, NULL, worker->context);
        }
    }

    mjs_destroy(mjs);
    js_modules_destroy(worker->modules);

    composite_api_resolver_free(worker->resolver);

    return 0;
}

JsThread* js_thread_run(const char* script_path, JsThreadCallback callback, void* context) {
    JsThread* worker = malloc(sizeof(JsThread)); //-V799
    worker->path = furi_string_alloc_set(script_path);
    worker->thread = furi_thread_alloc_ex("JsThread", 8 * 1024, js_thread, worker);
    worker->app_callback = callback;
    worker->context = context;
    furi_thread_start(worker->thread);
    return worker;
}

void js_thread_stop(JsThread* worker) {
    furi_thread_flags_set(furi_thread_get_id(worker->thread), ThreadEventStop);
    furi_thread_join(worker->thread);
    furi_thread_free(worker->thread);
    furi_string_free(worker->path);
    free(worker);
}
