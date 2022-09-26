#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/loading.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include "elf_cpp/elf_hashtable.h"
#include <flipper_application/flipper_application.h>

#define TAG "fap_loader_app"

typedef struct {
    FlipperApplication* app;
    Storage* storage;
    DialogsApp* dialogs;
    Gui* gui;
    string_t fap_path;

    ViewDispatcher* view_dispatcher;
    Loading* loading;
} FapLoader;

static bool
    fap_loader_item_callback(string_t path, void* context, uint8_t** icon_ptr, string_t item_name) {
    FapLoader* loader = context;
    furi_assert(loader);

    FlipperApplication* app = flipper_application_alloc(loader->storage, &hashtable_api_interface);

    FlipperApplicationPreloadStatus preload_res =
        flipper_application_preload_manifest(app, string_get_cstr(path));

    bool load_success = false;

    if(preload_res == FlipperApplicationPreloadStatusSuccess) {
        const FlipperApplicationManifest* manifest = flipper_application_get_manifest(app);
        if(manifest->has_icon) {
            memcpy(*icon_ptr, manifest->icon, FAP_MANIFEST_MAX_ICON_SIZE);
        }
        string_set_str(item_name, manifest->name);
        load_success = true;
    } else {
        FURI_LOG_E(TAG, "FAP Loader failed to preload %s", string_get_cstr(path));
        load_success = false;
    }

    flipper_application_free(app);
    return load_success;
}

static bool fap_loader_run_selected_app(FapLoader* loader) {
    furi_assert(loader);

    string_t error_message;

    string_init_set(error_message, "unknown error");

    bool file_selected = false;
    bool show_error = true;
    do {
        file_selected = true;
        loader->app = flipper_application_alloc(loader->storage, &hashtable_api_interface);

        FURI_LOG_I(TAG, "FAP Loader is loading %s", string_get_cstr(loader->fap_path));

        FlipperApplicationPreloadStatus preload_res =
            flipper_application_preload(loader->app, string_get_cstr(loader->fap_path));
        if(preload_res != FlipperApplicationPreloadStatusSuccess) {
            const char* err_msg = flipper_application_preload_status_to_string(preload_res);
            string_printf(error_message, "Preload failed: %s", err_msg);
            FURI_LOG_E(
                TAG,
                "FAP Loader failed to preload %s: %s",
                string_get_cstr(loader->fap_path),
                err_msg);
            break;
        }

        FURI_LOG_I(TAG, "FAP Loader is mapping");
        FlipperApplicationLoadStatus load_status = flipper_application_map_to_memory(loader->app);
        if(load_status != FlipperApplicationLoadStatusSuccess) {
            const char* err_msg = flipper_application_load_status_to_string(load_status);
            string_printf(error_message, "Load failed: %s", err_msg);
            FURI_LOG_E(
                TAG,
                "FAP Loader failed to map to memory %s: %s",
                string_get_cstr(loader->fap_path),
                err_msg);
            break;
        }

        FURI_LOG_I(TAG, "FAP Loader is staring app");

        FuriThread* thread = flipper_application_spawn(loader->app, NULL);
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

        string_t buffer;
        string_init(buffer);
        string_printf(buffer, "%s", string_get_cstr(error_message));
        string_replace_str(buffer, ":", "\n");
        dialog_message_set_text(
            message, string_get_cstr(buffer), 64, 32, AlignCenter, AlignCenter);

        dialog_message_show(loader->dialogs, message);
        dialog_message_free(message);
        string_clear(buffer);
    }

    string_clear(error_message);

    if(file_selected) {
        flipper_application_free(loader->app);
    }

    return file_selected;
}

static bool fap_loader_select_app(FapLoader* loader) {
    const DialogsFileBrowserOptions browser_options = {
        .extension = ".fap",
        .skip_assets = true,
        .icon = &I_badusb_10px,
        .hide_ext = true,
        .item_loader_callback = fap_loader_item_callback,
        .item_loader_context = loader,
    };

    return dialog_file_browser_show(
        loader->dialogs, loader->fap_path, loader->fap_path, &browser_options);
}

int32_t fap_loader_app(void* p) {
    FapLoader* loader = malloc(sizeof(FapLoader));
    loader->storage = furi_record_open(RECORD_STORAGE);
    loader->dialogs = furi_record_open(RECORD_DIALOGS);
    loader->gui = furi_record_open(RECORD_GUI);

    loader->view_dispatcher = view_dispatcher_alloc();
    loader->loading = loading_alloc();

    view_dispatcher_attach_to_gui(
        loader->view_dispatcher, loader->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(loader->view_dispatcher, 0, loading_get_view(loader->loading));

    if(p) {
        string_init_set(loader->fap_path, (const char*)p);
        fap_loader_run_selected_app(loader);
    } else {
        string_init_set(loader->fap_path, EXT_PATH("apps"));

        while(fap_loader_select_app(loader)) {
            view_dispatcher_switch_to_view(loader->view_dispatcher, 0);
            fap_loader_run_selected_app(loader);
        };
    }

    view_dispatcher_remove_view(loader->view_dispatcher, 0);
    loading_free(loader->loading);
    view_dispatcher_free(loader->view_dispatcher);

    string_clear(loader->fap_path);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);
    free(loader);
    return 0;
}