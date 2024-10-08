#include "loader.h"
#include "loader_i.h"
#include <applications.h>
#include <storage/storage.h>
#include <furi_hal.h>
#include <assets_icons.h>

#include <dialogs/dialogs.h>
#include <toolbox/path.h>
#include <flipper_application/flipper_application.h>
#include <loader/firmware_api/firmware_api.h>

#define TAG "Loader"

#define LOADER_MAGIC_THREAD_VALUE 0xDEADBEEF

// helpers

static const char* loader_find_external_application_by_name(const char* app_name) {
    for(size_t i = 0; i < FLIPPER_EXTERNAL_APPS_COUNT; i++) {
        if(strcmp(FLIPPER_EXTERNAL_APPS[i].name, app_name) == 0) {
            return FLIPPER_EXTERNAL_APPS[i].path;
        }
    }

    return NULL;
}

// API

static LoaderMessageLoaderStatusResult loader_start_internal(
    Loader* loader,
    const char* name,
    const char* args,
    FuriString* error_message) {
    LoaderMessage message;
    LoaderMessageLoaderStatusResult result;

    message.type = LoaderMessageTypeStartByName;
    message.start.name = name;
    message.start.args = args;
    message.start.error_message = error_message;
    message.api_lock = api_lock_alloc_locked();
    message.status_value = &result;
    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result;
}

typedef struct {
    const char* error;
    const char* description;
    const char* url;
    const Icon* icon;
} LoaderError;

static const LoaderError err_app_not_found =
    {"App Not Found", "Update firmware or app", "err_01", &I_err_01};
static const LoaderError err_invalid_flie = {"Invalid File", "Update the app", "err_02", &I_err_02};
static const LoaderError err_invalid_manifest =
    {"Invalid Manifest", "Update firmware or app", "err_03", &I_err_03};
static const LoaderError err_missing_imports =
    {"Missing Imports", "Update firmware", "err_04", &I_err_04};
static const LoaderError err_hw_target_mismatch =
    {"HW Target\nMismatch", "App not supported", "err_05", &I_err_05};
static const LoaderError err_outdated_app = {"Outdated App", "Update the app", "err_06", &I_err_06};
static const LoaderError err_outdated_firmware =
    {"Outdated\nFirmware", "Update firmware", "err_07", &I_err_07};

static void loader_dialog_prepare_and_show(DialogsApp* dialogs, const LoaderError* err) {
    FuriString* header = furi_string_alloc_printf("Error: %s", err->error);
    FuriString* text =
        furi_string_alloc_printf("%s\nLearn more:\nr.flipper.net/%s", err->description, err->url);
    DialogMessage* message = dialog_message_alloc();

    dialog_message_set_header(message, furi_string_get_cstr(header), 64, 0, AlignCenter, AlignTop);
    dialog_message_set_text(message, furi_string_get_cstr(text), 0, 63, AlignLeft, AlignBottom);
    dialog_message_set_icon(message, err->icon, 128 - 25, 64 - 25);
    dialog_message_show(dialogs, message);

    dialog_message_free(message);
    furi_string_free(header);
    furi_string_free(text);
}

static void loader_show_gui_error(
    LoaderMessageLoaderStatusResult status,
    const char* name,
    FuriString* error_message) {
    DialogsApp* dialogs = furi_record_open(RECORD_DIALOGS);
    DialogMessage* message = dialog_message_alloc();

    if(status.value == LoaderStatusErrorUnknownApp &&
       loader_find_external_application_by_name(name) != NULL) {
        // Special case for external apps
        const char* header = NULL;
        const char* text = NULL;
        Storage* storage = furi_record_open(RECORD_STORAGE);
        if(storage_sd_status(storage) == FSE_OK) {
            header = "Update needed";
            text = "Update firmware\nto run this app";
        } else {
            header = "SD card needed";
            text = "Install SD card\nto run this app";
        }
        furi_record_close(RECORD_STORAGE);
        dialog_message_set_header(message, header, 64, 3, AlignCenter, AlignTop);
        dialog_message_set_icon(message, &I_WarningDolphinFlip_45x42, 83, 22);
        dialog_message_set_text(message, text, 3, 26, AlignLeft, AlignTop);
        dialog_message_show(dialogs, message);
    } else if(status.value == LoaderStatusErrorUnknownApp) {
        loader_dialog_prepare_and_show(dialogs, &err_app_not_found);
    } else if(status.value == LoaderStatusErrorInternal) {
        // TODO FL-3522: we have many places where we can emit a double start, ex: desktop, menu
        // so i prefer to not show LoaderStatusErrorAppStarted error message for now
        switch(status.error) {
        case LoaderStatusErrorInvalidFile:
            loader_dialog_prepare_and_show(dialogs, &err_invalid_flie);
            break;
        case LoaderStatusErrorInvalidManifest:
            loader_dialog_prepare_and_show(dialogs, &err_invalid_manifest);
            break;
        case LoaderStatusErrorMissingImports:
            loader_dialog_prepare_and_show(dialogs, &err_missing_imports);
            break;
        case LoaderStatusErrorHWMismatch:
            loader_dialog_prepare_and_show(dialogs, &err_hw_target_mismatch);
            break;
        case LoaderStatusErrorOutdatedApp:
            loader_dialog_prepare_and_show(dialogs, &err_outdated_app);
            break;
        case LoaderStatusErrorOutdatedFirmware:
            loader_dialog_prepare_and_show(dialogs, &err_outdated_firmware);
            break;
        case LoaderStatusErrorOutOfMemory:
            dialog_message_set_header(
                message, "Error: Out of Memory", 64, 0, AlignCenter, AlignTop);
            dialog_message_set_text(
                message,
                "Not enough RAM to run the\napp. Please reboot the device",
                64,
                13,
                AlignCenter,
                AlignTop);
            dialog_message_set_buttons(message, NULL, NULL, "Reboot");
            if(dialog_message_show(dialogs, message) == DialogMessageButtonRight) {
                furi_hal_power_reset();
            }
            break;
        default:
            // Generic error
            dialog_message_set_header(message, "Error", 64, 0, AlignCenter, AlignTop);

            furi_string_replace(error_message, "/ext/apps/", "");
            furi_string_replace(error_message, ", ", "\n");
            furi_string_replace(error_message, ": ", "\n");

            dialog_message_set_text(
                message, furi_string_get_cstr(error_message), 64, 35, AlignCenter, AlignCenter);
            dialog_message_show(dialogs, message);
            break;
        }
    }

    dialog_message_free(message);
    furi_record_close(RECORD_DIALOGS);
}

LoaderStatus
    loader_start(Loader* loader, const char* name, const char* args, FuriString* error_message) {
    furi_check(loader);
    furi_check(name);

    LoaderMessageLoaderStatusResult result =
        loader_start_internal(loader, name, args, error_message);
    return result.value;
}

LoaderStatus loader_start_with_gui_error(Loader* loader, const char* name, const char* args) {
    furi_check(loader);
    furi_check(name);

    FuriString* error_message = furi_string_alloc();
    LoaderMessageLoaderStatusResult result =
        loader_start_internal(loader, name, args, error_message);
    loader_show_gui_error(result, name, error_message);
    furi_string_free(error_message);
    return result.value;
}

void loader_start_detached_with_gui_error(Loader* loader, const char* name, const char* args) {
    furi_check(loader);
    furi_check(name);

    LoaderMessage message = {
        .type = LoaderMessageTypeStartByNameDetachedWithGuiError,
        .start.name = strdup(name),
        .start.args = args ? strdup(args) : NULL,
    };
    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
}

bool loader_lock(Loader* loader) {
    furi_check(loader);

    LoaderMessage message;
    LoaderMessageBoolResult result;
    message.type = LoaderMessageTypeLock;
    message.api_lock = api_lock_alloc_locked();
    message.bool_value = &result;
    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result.value;
}

void loader_unlock(Loader* loader) {
    furi_check(loader);

    LoaderMessage message;
    message.type = LoaderMessageTypeUnlock;

    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
}

bool loader_is_locked(Loader* loader) {
    furi_check(loader);

    LoaderMessage message;
    LoaderMessageBoolResult result;
    message.type = LoaderMessageTypeIsLocked;
    message.api_lock = api_lock_alloc_locked();
    message.bool_value = &result;
    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result.value;
}

void loader_show_menu(Loader* loader) {
    furi_check(loader);

    LoaderMessage message;
    message.type = LoaderMessageTypeShowMenu;

    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
}

FuriPubSub* loader_get_pubsub(Loader* loader) {
    furi_check(loader);
    // it's safe to return pubsub without locking
    // because it's never freed and loader is never exited
    // also the loader instance cannot be obtained until the pubsub is created
    return loader->pubsub;
}

bool loader_signal(Loader* loader, uint32_t signal, void* arg) {
    furi_check(loader);

    LoaderMessageBoolResult result;

    LoaderMessage message = {
        .type = LoaderMessageTypeSignal,
        .api_lock = api_lock_alloc_locked(),
        .signal.signal = signal,
        .signal.arg = arg,
        .bool_value = &result,
    };

    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result.value;
}

bool loader_get_application_name(Loader* loader, FuriString* name) {
    furi_check(loader);

    LoaderMessageBoolResult result;

    LoaderMessage message = {
        .type = LoaderMessageTypeGetApplicationName,
        .api_lock = api_lock_alloc_locked(),
        .application_name = name,
        .bool_value = &result,
    };

    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
    api_lock_wait_unlock_and_free(message.api_lock);

    return result.value;
}

// callbacks

static void loader_menu_closed_callback(void* context) {
    Loader* loader = context;
    LoaderMessage message;
    message.type = LoaderMessageTypeMenuClosed;
    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
}

static void loader_applications_closed_callback(void* context) {
    Loader* loader = context;
    LoaderMessage message;
    message.type = LoaderMessageTypeApplicationsClosed;
    furi_message_queue_put(loader->queue, &message, FuriWaitForever);
}

static void
    loader_thread_state_callback(FuriThread* thread, FuriThreadState thread_state, void* context) {
    UNUSED(thread);
    furi_assert(context);

    if(thread_state == FuriThreadStateStopped) {
        Loader* loader = context;

        LoaderMessage message;
        message.type = LoaderMessageTypeAppClosed;
        furi_message_queue_put(loader->queue, &message, FuriWaitForever);
    }
}

// implementation

static Loader* loader_alloc(void) {
    Loader* loader = malloc(sizeof(Loader));
    loader->pubsub = furi_pubsub_alloc();
    loader->queue = furi_message_queue_alloc(1, sizeof(LoaderMessage));
    loader->loader_menu = NULL;
    loader->loader_applications = NULL;
    loader->app.args = NULL;
    loader->app.thread = NULL;
    loader->app.insomniac = false;
    loader->app.fap = NULL;
    return loader;
}

static FlipperInternalApplication const* loader_find_application_by_name_in_list(
    const char* name,
    const FlipperInternalApplication* list,
    const uint32_t n_apps) {
    for(size_t i = 0; i < n_apps; i++) {
        if((strcmp(name, list[i].name) == 0) || (strcmp(name, list[i].appid) == 0)) {
            return &list[i];
        }
    }
    return NULL;
}

static const FlipperInternalApplication* loader_find_application_by_name(const char* name) {
    const struct {
        const FlipperInternalApplication* list;
        const uint32_t count;
    } lists[] = {
        {FLIPPER_SETTINGS_APPS, FLIPPER_SETTINGS_APPS_COUNT},
        {FLIPPER_SYSTEM_APPS, FLIPPER_SYSTEM_APPS_COUNT},
        {FLIPPER_DEBUG_APPS, FLIPPER_DEBUG_APPS_COUNT},
    };

    for(size_t i = 0; i < COUNT_OF(lists); i++) {
        const FlipperInternalApplication* application =
            loader_find_application_by_name_in_list(name, lists[i].list, lists[i].count);
        if(application) {
            return application;
        }
    }

    return NULL;
}

static void loader_start_app_thread(Loader* loader, FlipperInternalApplicationFlag flags) {
    // setup heap trace
    FuriHalRtcHeapTrackMode mode = furi_hal_rtc_get_heap_track_mode();
    if(mode > FuriHalRtcHeapTrackModeNone) {
        furi_thread_enable_heap_trace(loader->app.thread);
    } else {
        furi_thread_disable_heap_trace(loader->app.thread);
    }

    // setup insomnia
    if(!(flags & FlipperInternalApplicationFlagInsomniaSafe)) {
        furi_hal_power_insomnia_enter();
        loader->app.insomniac = true;
    } else {
        loader->app.insomniac = false;
    }

    // setup thread state callbacks
    furi_thread_set_state_context(loader->app.thread, loader);
    furi_thread_set_state_callback(loader->app.thread, loader_thread_state_callback);

    // start app thread
    furi_thread_start(loader->app.thread);
}

static void loader_start_internal_app(
    Loader* loader,
    const FlipperInternalApplication* app,
    const char* args) {
    FURI_LOG_I(TAG, "Starting %s", app->name);
    LoaderEvent event;
    event.type = LoaderEventTypeApplicationBeforeLoad;
    furi_pubsub_publish(loader->pubsub, &event);

    // store args
    furi_assert(loader->app.args == NULL);
    if(args && strlen(args) > 0) {
        loader->app.args = strdup(args);
    }

    loader->app.thread =
        furi_thread_alloc_ex(app->name, app->stack_size, app->app, loader->app.args);
    furi_thread_set_appid(loader->app.thread, app->appid);

    loader_start_app_thread(loader, app->flags);
}

static void loader_log_status_error(
    LoaderStatus status,
    FuriString* error_message,
    const char* format,
    va_list args) {
    if(error_message) {
        furi_string_vprintf(error_message, format, args);
        FURI_LOG_E(TAG, "Status [%d]: %s", status, furi_string_get_cstr(error_message));
    } else {
        FURI_LOG_E(TAG, "Status [%d]", status);
    }
}

static LoaderStatus loader_make_status_error(
    LoaderStatus status,
    FuriString* error_message,
    const char* format,
    ...) {
    va_list args;
    va_start(args, format);
    loader_log_status_error(status, error_message, format, args);
    va_end(args);
    return status;
}

static LoaderStatus loader_make_success_status(FuriString* error_message) {
    if(error_message) {
        furi_string_set(error_message, "App started");
    }

    return LoaderStatusOk;
}

static LoaderStatusError
    loader_status_error_from_preload_status(FlipperApplicationPreloadStatus status) {
    switch(status) {
    case FlipperApplicationPreloadStatusInvalidFile:
        return LoaderStatusErrorInvalidFile;
    case FlipperApplicationPreloadStatusNotEnoughMemory:
        return LoaderStatusErrorOutOfMemory;
    case FlipperApplicationPreloadStatusInvalidManifest:
        return LoaderStatusErrorInvalidManifest;
    case FlipperApplicationPreloadStatusApiTooOld:
        return LoaderStatusErrorOutdatedApp;
    case FlipperApplicationPreloadStatusApiTooNew:
        return LoaderStatusErrorOutdatedFirmware;
    case FlipperApplicationPreloadStatusTargetMismatch:
        return LoaderStatusErrorHWMismatch;
    default:
        return LoaderStatusErrorUnknown;
    }
}

static LoaderStatusError
    loader_status_error_from_load_status(FlipperApplicationLoadStatus status) {
    switch(status) {
    case FlipperApplicationLoadStatusMissingImports:
        return LoaderStatusErrorMissingImports;
    default:
        return LoaderStatusErrorUnknown;
    }
}

static LoaderMessageLoaderStatusResult loader_start_external_app(
    Loader* loader,
    Storage* storage,
    const char* path,
    const char* args,
    FuriString* error_message) {
    LoaderMessageLoaderStatusResult result;
    result.value = loader_make_success_status(error_message);
    result.error = LoaderStatusErrorUnknown;

    LoaderEvent event;
    event.type = LoaderEventTypeApplicationBeforeLoad;
    furi_pubsub_publish(loader->pubsub, &event);

    do {
        loader->app.fap = flipper_application_alloc(storage, firmware_api_interface);
        size_t start = furi_get_tick();

        FURI_LOG_I(TAG, "Loading %s", path);

        FlipperApplicationPreloadStatus preload_res =
            flipper_application_preload(loader->app.fap, path);
        if(preload_res != FlipperApplicationPreloadStatusSuccess) {
            const char* err_msg = flipper_application_preload_status_to_string(preload_res);
            result.value = loader_make_status_error(
                LoaderStatusErrorInternal, error_message, "Preload failed, %s: %s", path, err_msg);
            result.error = loader_status_error_from_preload_status(preload_res);
            break;
        }

        FlipperApplicationLoadStatus load_status =
            flipper_application_map_to_memory(loader->app.fap);
        if(load_status != FlipperApplicationLoadStatusSuccess) {
            const char* err_msg = flipper_application_load_status_to_string(load_status);
            result.value = loader_make_status_error(
                LoaderStatusErrorInternal, error_message, "Load failed, %s: %s", path, err_msg);
            result.error = loader_status_error_from_load_status(load_status);
            break;
        }

        FURI_LOG_I(TAG, "Loaded in %zums", (size_t)(furi_get_tick() - start));

        if(flipper_application_is_plugin(loader->app.fap)) {
            result.value = loader_make_status_error(
                LoaderStatusErrorInternal, error_message, "Plugin %s is not runnable", path);
            break;
        }

        loader->app.thread = flipper_application_alloc_thread(loader->app.fap, args);
        FuriString* app_name = furi_string_alloc();
        path_extract_filename_no_ext(path, app_name);
        furi_thread_set_appid(loader->app.thread, furi_string_get_cstr(app_name));
        furi_string_free(app_name);

        /* This flag is set by the debugger - to break on app start */
        if(furi_hal_debug_is_gdb_session_active()) {
            FURI_LOG_W(TAG, "Triggering BP for debugger");
            /* After hitting this, you can set breakpoints in your .fap's code
             * Note that you have to toggle breakpoints that were set before */
            __asm volatile("bkpt 0");
        }

        loader_start_app_thread(loader, FlipperInternalApplicationFlagDefault);
    } while(0);

    if(result.value != LoaderStatusOk) {
        flipper_application_free(loader->app.fap);
        loader->app.fap = NULL;
        event.type = LoaderEventTypeApplicationLoadFailed;
        furi_pubsub_publish(loader->pubsub, &event);
    }

    return result;
}

// process messages

static void loader_do_menu_show(Loader* loader) {
    if(!loader->loader_menu) {
        loader->loader_menu = loader_menu_alloc(loader_menu_closed_callback, loader);
    }
}

static void loader_do_menu_closed(Loader* loader) {
    if(loader->loader_menu) {
        loader_menu_free(loader->loader_menu);
        loader->loader_menu = NULL;
    }
}

static void loader_do_applications_show(Loader* loader) {
    if(!loader->loader_applications) {
        loader->loader_applications =
            loader_applications_alloc(loader_applications_closed_callback, loader);
    }
}

static void loader_do_applications_closed(Loader* loader) {
    if(loader->loader_applications) {
        loader_applications_free(loader->loader_applications);
        loader->loader_applications = NULL;
    }
}

static bool loader_do_is_locked(Loader* loader) {
    return loader->app.thread != NULL;
}

static LoaderMessageLoaderStatusResult loader_do_start_by_name(
    Loader* loader,
    const char* name,
    const char* args,
    FuriString* error_message) {
    LoaderMessageLoaderStatusResult status;
    status.value = loader_make_success_status(error_message);
    status.error = LoaderStatusErrorUnknown;

    do {
        // check lock
        if(loader_do_is_locked(loader)) {
            if(loader->app.thread == (FuriThread*)LOADER_MAGIC_THREAD_VALUE) {
                status.value = loader_make_status_error(
                    LoaderStatusErrorAppStarted, error_message, "Loader is locked");
            } else {
                const char* current_thread_name =
                    furi_thread_get_name(furi_thread_get_id(loader->app.thread));

                status.value = loader_make_status_error(
                    LoaderStatusErrorAppStarted,
                    error_message,
                    "Loader is locked, please close the \"%s\" first",
                    current_thread_name);
            }
            break;
        }

        // check internal apps
        {
            const FlipperInternalApplication* app = loader_find_application_by_name(name);
            if(app) {
                loader_start_internal_app(loader, app, args);
                status.value = loader_make_success_status(error_message);
                break;
            }
        }

        // check Applications
        if(strcmp(name, LOADER_APPLICATIONS_NAME) == 0) {
            loader_do_applications_show(loader);
            status.value = loader_make_success_status(error_message);
            break;
        }

        // check External Applications
        {
            const char* path = loader_find_external_application_by_name(name);
            if(path) {
                name = path;
            }
        }

        // check Faps
        {
            Storage* storage = furi_record_open(RECORD_STORAGE);
            if(storage_file_exists(storage, name)) {
                status = loader_start_external_app(loader, storage, name, args, error_message);
                furi_record_close(RECORD_STORAGE);
                break;
            }
            furi_record_close(RECORD_STORAGE);
        }

        status.value = loader_make_status_error(
            LoaderStatusErrorUnknownApp, error_message, "Application \"%s\" not found", name);
    } while(false);

    return status;
}

static bool loader_do_lock(Loader* loader) {
    if(loader->app.thread) {
        return false;
    }

    loader->app.thread = (FuriThread*)LOADER_MAGIC_THREAD_VALUE;
    return true;
}

static void loader_do_unlock(Loader* loader) {
    furi_check(loader->app.thread == (FuriThread*)LOADER_MAGIC_THREAD_VALUE);
    loader->app.thread = NULL;
}

static void loader_do_app_closed(Loader* loader) {
    furi_assert(loader->app.thread);

    furi_thread_join(loader->app.thread);
    FURI_LOG_I(TAG, "App returned: %li", furi_thread_get_return_code(loader->app.thread));

    if(loader->app.args) {
        free(loader->app.args);
        loader->app.args = NULL;
    }

    if(loader->app.insomniac) {
        furi_hal_power_insomnia_exit();
    }

    if(loader->app.fap) {
        flipper_application_free(loader->app.fap);
        loader->app.fap = NULL;
        loader->app.thread = NULL;
    } else {
        furi_thread_free(loader->app.thread);
        loader->app.thread = NULL;
    }

    FURI_LOG_I(TAG, "Application stopped. Free heap: %zu", memmgr_get_free_heap());

    LoaderEvent event;
    event.type = LoaderEventTypeApplicationStopped;
    furi_pubsub_publish(loader->pubsub, &event);
}

static bool loader_is_application_running(Loader* loader) {
    FuriThread* app_thread = loader->app.thread;
    return app_thread && (app_thread != (FuriThread*)LOADER_MAGIC_THREAD_VALUE);
}

static bool loader_do_signal(Loader* loader, uint32_t signal, void* arg) {
    if(loader_is_application_running(loader)) {
        return furi_thread_signal(loader->app.thread, signal, arg);
    }

    return false;
}

static bool loader_do_get_application_name(Loader* loader, FuriString* name) {
    if(loader_is_application_running(loader)) {
        furi_string_set(name, furi_thread_get_name(furi_thread_get_id(loader->app.thread)));
        return true;
    }

    return false;
}

// app

int32_t loader_srv(void* p) {
    UNUSED(p);
    Loader* loader = loader_alloc();
    furi_record_create(RECORD_LOADER, loader);

    FURI_LOG_I(TAG, "Executing system start hooks");
    for(size_t i = 0; i < FLIPPER_ON_SYSTEM_START_COUNT; i++) {
        FLIPPER_ON_SYSTEM_START[i]();
    }

    if((furi_hal_rtc_get_boot_mode() == FuriHalRtcBootModeNormal) && FLIPPER_AUTORUN_APP_NAME &&
       strlen(FLIPPER_AUTORUN_APP_NAME)) {
        FURI_LOG_I(TAG, "Starting autorun app: %s", FLIPPER_AUTORUN_APP_NAME);
        loader_do_start_by_name(loader, FLIPPER_AUTORUN_APP_NAME, NULL, NULL);
    }

    LoaderMessage message;
    while(true) {
        if(furi_message_queue_get(loader->queue, &message, FuriWaitForever) == FuriStatusOk) {
            switch(message.type) {
            case LoaderMessageTypeStartByName:
                *(message.status_value) = loader_do_start_by_name(
                    loader, message.start.name, message.start.args, message.start.error_message);
                api_lock_unlock(message.api_lock);
                break;
            case LoaderMessageTypeStartByNameDetachedWithGuiError: {
                FuriString* error_message = furi_string_alloc();
                LoaderMessageLoaderStatusResult status = loader_do_start_by_name(
                    loader, message.start.name, message.start.args, error_message);
                loader_show_gui_error(status, message.start.name, error_message);
                if(message.start.name) free((void*)message.start.name);
                if(message.start.args) free((void*)message.start.args);
                furi_string_free(error_message);
                break;
            }
            case LoaderMessageTypeShowMenu:
                loader_do_menu_show(loader);
                break;
            case LoaderMessageTypeMenuClosed:
                loader_do_menu_closed(loader);
                break;
            case LoaderMessageTypeIsLocked:
                message.bool_value->value = loader_do_is_locked(loader);
                api_lock_unlock(message.api_lock);
                break;
            case LoaderMessageTypeAppClosed:
                loader_do_app_closed(loader);
                break;
            case LoaderMessageTypeLock:
                message.bool_value->value = loader_do_lock(loader);
                api_lock_unlock(message.api_lock);
                break;
            case LoaderMessageTypeUnlock:
                loader_do_unlock(loader);
                break;
            case LoaderMessageTypeApplicationsClosed:
                loader_do_applications_closed(loader);
                break;
            case LoaderMessageTypeSignal:
                message.bool_value->value =
                    loader_do_signal(loader, message.signal.signal, message.signal.arg);
                api_lock_unlock(message.api_lock);
                break;
            case LoaderMessageTypeGetApplicationName:
                message.bool_value->value =
                    loader_do_get_application_name(loader, message.application_name);
                api_lock_unlock(message.api_lock);
                break;
            }
        }
    }

    return 0;
}
