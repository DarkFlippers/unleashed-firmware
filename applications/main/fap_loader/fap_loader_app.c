#include <furi.h>
#include <gui/gui.h>
#include <assets_icons.h>
#include <gui/view_dispatcher.h>
#include <storage/storage.h>
#include <gui/modules/loading.h>
#include <dialogs/dialogs.h>
#include <toolbox/path.h>
#include <flipper_application/flipper_application.h>
#include <loader/firmware_api/firmware_api.h>
#include "fap_loader_app.h"

#define TAG "fap_loader_app"

struct FapLoader {
    FlipperApplication* app;
    Storage* storage;
    DialogsApp* dialogs;
    Gui* gui;
    FuriString* fap_path;
    ViewDispatcher* view_dispatcher;
    Loading* loading;
};

bool fap_loader_load_name_and_icon(
    FuriString* path,
    Storage* storage,
    uint8_t** icon_ptr,
    FuriString* item_name) {
    FlipperApplication* app = flipper_application_alloc(storage, firmware_api_interface);

    FlipperApplicationPreloadStatus preload_res =
        flipper_application_preload_manifest(app, furi_string_get_cstr(path));

    bool load_success = false;

    if(preload_res == FlipperApplicationPreloadStatusSuccess) {
        const FlipperApplicationManifest* manifest = flipper_application_get_manifest(app);
        if(manifest->has_icon) {
            memcpy(*icon_ptr, manifest->icon, FAP_MANIFEST_MAX_ICON_SIZE);
        }
        furi_string_set(item_name, manifest->name);
        load_success = true;
    } else {
        FURI_LOG_E(TAG, "FAP Loader failed to preload %s", furi_string_get_cstr(path));
        load_success = false;
    }

    flipper_application_free(app);
    return load_success;
}

static bool fap_loader_item_callback(
    FuriString* path,
    void* context,
    uint8_t** icon_ptr,
    FuriString* item_name) {
    FapLoader* fap_loader = context;
    furi_assert(fap_loader);
    return fap_loader_load_name_and_icon(path, fap_loader->storage, icon_ptr, item_name);
}

static bool fap_loader_run_selected_app(FapLoader* loader) {
    furi_assert(loader);

    FuriString* error_message;

    error_message = furi_string_alloc_set("unknown error");

    bool file_selected = false;
    bool show_error = true;
    do {
        file_selected = true;
        loader->app = flipper_application_alloc(loader->storage, firmware_api_interface);
        size_t start = furi_get_tick();

        FURI_LOG_I(TAG, "FAP Loader is loading %s", furi_string_get_cstr(loader->fap_path));

        FlipperApplicationPreloadStatus preload_res =
            flipper_application_preload(loader->app, furi_string_get_cstr(loader->fap_path));
        if(preload_res != FlipperApplicationPreloadStatusSuccess) {
            const char* err_msg = flipper_application_preload_status_to_string(preload_res);
            furi_string_printf(error_message, "Preload failed: %s", err_msg);
            FURI_LOG_E(
                TAG,
                "FAP Loader failed to preload %s: %s",
                furi_string_get_cstr(loader->fap_path),
                err_msg);
            break;
        }

        FURI_LOG_I(TAG, "FAP Loader is mapping");
        FlipperApplicationLoadStatus load_status = flipper_application_map_to_memory(loader->app);
        if(load_status != FlipperApplicationLoadStatusSuccess) {
            const char* err_msg = flipper_application_load_status_to_string(load_status);
            furi_string_printf(error_message, "Load failed: %s", err_msg);
            FURI_LOG_E(
                TAG,
                "FAP Loader failed to map to memory %s: %s",
                furi_string_get_cstr(loader->fap_path),
                err_msg);
            break;
        }

        FURI_LOG_I(TAG, "Loaded in %ums", (size_t)(furi_get_tick() - start));
        FURI_LOG_I(TAG, "FAP Loader is starting app");

        FuriThread* thread = flipper_application_spawn(loader->app, NULL);

        FuriString* app_name = furi_string_alloc();
        path_extract_filename_no_ext(furi_string_get_cstr(loader->fap_path), app_name);
        furi_thread_set_appid(thread, furi_string_get_cstr(app_name));
        furi_string_free(app_name);

        furi_thread_start(thread);
        furi_thread_join(thread);

        show_error = false;
        int ret = furi_thread_get_return_code(thread);

        FURI_LOG_I(TAG, "FAP app returned: %i", ret);
    } while(0);

    if(show_error) {
        DialogMessage* message = dialog_message_alloc();
        dialog_message_set_header(message, "Error", 64, 0, AlignCenter, AlignTop);
        dialog_message_set_buttons(message, NULL, NULL, NULL);

        FuriString* buffer;
        buffer = furi_string_alloc();
        furi_string_printf(buffer, "%s", furi_string_get_cstr(error_message));
        furi_string_replace(buffer, ":", "\n");
        dialog_message_set_text(
            message, furi_string_get_cstr(buffer), 64, 32, AlignCenter, AlignCenter);

        dialog_message_show(loader->dialogs, message);
        dialog_message_free(message);
        furi_string_free(buffer);
    }

    furi_string_free(error_message);

    if(file_selected) {
        flipper_application_free(loader->app);
    }

    return file_selected;
}

static bool fap_loader_select_app(FapLoader* loader) {
    const DialogsFileBrowserOptions browser_options = {
        .extension = ".fap",
        .skip_assets = true,
        .icon = &I_unknown_10px,
        .hide_ext = true,
        .item_loader_callback = fap_loader_item_callback,
        .item_loader_context = loader,
        .base_path = EXT_PATH("apps"),
    };

    return dialog_file_browser_show(
        loader->dialogs, loader->fap_path, loader->fap_path, &browser_options);
}

static FapLoader* fap_loader_alloc(const char* path) {
    FapLoader* loader = malloc(sizeof(FapLoader)); //-V799
    loader->fap_path = furi_string_alloc_set(path);
    loader->storage = furi_record_open(RECORD_STORAGE);
    loader->dialogs = furi_record_open(RECORD_DIALOGS);
    loader->gui = furi_record_open(RECORD_GUI);
    loader->view_dispatcher = view_dispatcher_alloc();
    loader->loading = loading_alloc();
    view_dispatcher_attach_to_gui(
        loader->view_dispatcher, loader->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(loader->view_dispatcher, 0, loading_get_view(loader->loading));
    return loader;
} //-V773

static void fap_loader_free(FapLoader* loader) {
    view_dispatcher_remove_view(loader->view_dispatcher, 0);
    loading_free(loader->loading);
    view_dispatcher_free(loader->view_dispatcher);
    furi_string_free(loader->fap_path);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);
    free(loader);
}

int32_t fap_loader_app(void* p) {
    FapLoader* loader;
    if(p) {
        loader = fap_loader_alloc((const char*)p);
        view_dispatcher_switch_to_view(loader->view_dispatcher, 0);
        fap_loader_run_selected_app(loader);
    } else {
        loader = fap_loader_alloc(EXT_PATH("apps"));
        while(fap_loader_select_app(loader)) {
            view_dispatcher_switch_to_view(loader->view_dispatcher, 0);
            fap_loader_run_selected_app(loader);
        };
    }

    fap_loader_free(loader);
    return 0;
}
