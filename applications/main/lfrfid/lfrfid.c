#include "lfrfid_i.h"
#include <dolphin/dolphin.h>
#include <loader/loader.h>
#include <flipper_application/application_manifest.h> // FAP_MANIFEST_MAX_ICON_SIZE

#define TAG "LfRfid"

static bool lfrfid_debug_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    LfRfid* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool lfrfid_debug_back_event_callback(void* context) {
    furi_assert(context);
    LfRfid* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void rpc_command_callback(const RpcAppSystemEvent* event, void* context) {
    furi_assert(context);
    LfRfid* app = (LfRfid*)context;

    if(event->type == RpcAppEventTypeSessionClose) {
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventRpcSessionClose);
        // Detach RPC
        rpc_system_app_set_callback(app->rpc_ctx, NULL, NULL);
        app->rpc_ctx = NULL;
    } else if(event->type == RpcAppEventTypeAppExit) {
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventExit);
    } else if(event->type == RpcAppEventTypeLoadFile) {
        furi_assert(event->data.type == RpcAppSystemEventDataTypeString);
        furi_string_set(app->file_path, event->data.string);
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventRpcLoadFile);
    } else {
        rpc_system_app_confirm(app->rpc_ctx, false);
    }
}

static LfRfid* lfrfid_alloc(void) {
    LfRfid* lfrfid = malloc(sizeof(LfRfid));

    lfrfid->variable_item_list = NULL;
    lfrfid->storage = furi_record_open(RECORD_STORAGE);
    lfrfid->dialogs = furi_record_open(RECORD_DIALOGS);

    lfrfid->file_name = furi_string_alloc();
    lfrfid->raw_file_name = furi_string_alloc();
    lfrfid->file_path = furi_string_alloc_set(LFRFID_APP_FOLDER);

    lfrfid->dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);

    size_t size = protocol_dict_get_max_data_size(lfrfid->dict);
    lfrfid->new_key_data = (uint8_t*)malloc(size);
    lfrfid->old_key_data = (uint8_t*)malloc(size);

    lfrfid->lfworker = lfrfid_worker_alloc(lfrfid->dict);

    lfrfid->view_dispatcher = view_dispatcher_alloc();
    lfrfid->scene_manager = scene_manager_alloc(&lfrfid_scene_handlers, lfrfid);
    view_dispatcher_set_event_callback_context(lfrfid->view_dispatcher, lfrfid);
    view_dispatcher_set_custom_event_callback(
        lfrfid->view_dispatcher, lfrfid_debug_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        lfrfid->view_dispatcher, lfrfid_debug_back_event_callback);

    // Open GUI record
    lfrfid->gui = furi_record_open(RECORD_GUI);

    // Open Notification record
    lfrfid->notifications = furi_record_open(RECORD_NOTIFICATION);

    // Submenu
    lfrfid->submenu = submenu_alloc();
    view_dispatcher_add_view(
        lfrfid->view_dispatcher, LfRfidViewSubmenu, submenu_get_view(lfrfid->submenu));

    // Dialog
    lfrfid->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        lfrfid->view_dispatcher, LfRfidViewDialogEx, dialog_ex_get_view(lfrfid->dialog_ex));

    // Popup
    lfrfid->popup = popup_alloc();
    view_dispatcher_add_view(
        lfrfid->view_dispatcher, LfRfidViewPopup, popup_get_view(lfrfid->popup));

    // Widget
    lfrfid->widget = widget_alloc();
    view_dispatcher_add_view(
        lfrfid->view_dispatcher, LfRfidViewWidget, widget_get_view(lfrfid->widget));

    // Text Input
    lfrfid->text_input = text_input_alloc();
    view_dispatcher_add_view(
        lfrfid->view_dispatcher, LfRfidViewTextInput, text_input_get_view(lfrfid->text_input));

    // Byte Input
    lfrfid->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        lfrfid->view_dispatcher, LfRfidViewByteInput, byte_input_get_view(lfrfid->byte_input));

    // Number Input
    lfrfid->number_input = number_input_alloc();
    view_dispatcher_add_view(
        lfrfid->view_dispatcher,
        LfRfidViewNumberInput,
        number_input_get_view(lfrfid->number_input));

    // Read custom view
    lfrfid->read_view = lfrfid_view_read_alloc();
    view_dispatcher_add_view(
        lfrfid->view_dispatcher, LfRfidViewRead, lfrfid_view_read_get_view(lfrfid->read_view));

    return lfrfid;
} //-V773

static void lfrfid_free(LfRfid* lfrfid) {
    furi_assert(lfrfid);

    furi_string_free(lfrfid->raw_file_name);
    furi_string_free(lfrfid->file_name);
    furi_string_free(lfrfid->file_path);
    protocol_dict_free(lfrfid->dict);

    lfrfid_worker_free(lfrfid->lfworker);

    if(lfrfid->rpc_ctx) {
        rpc_system_app_set_callback(lfrfid->rpc_ctx, NULL, NULL);
        rpc_system_app_send_exited(lfrfid->rpc_ctx);
    }

    free(lfrfid->new_key_data);
    free(lfrfid->old_key_data);

    // Submenu
    view_dispatcher_remove_view(lfrfid->view_dispatcher, LfRfidViewSubmenu);
    submenu_free(lfrfid->submenu);

    // DialogEx
    view_dispatcher_remove_view(lfrfid->view_dispatcher, LfRfidViewDialogEx);
    dialog_ex_free(lfrfid->dialog_ex);

    // Popup
    view_dispatcher_remove_view(lfrfid->view_dispatcher, LfRfidViewPopup);
    popup_free(lfrfid->popup);

    // Widget
    view_dispatcher_remove_view(lfrfid->view_dispatcher, LfRfidViewWidget);
    widget_free(lfrfid->widget);

    // TextInput
    view_dispatcher_remove_view(lfrfid->view_dispatcher, LfRfidViewTextInput);
    text_input_free(lfrfid->text_input);

    // ByteInput
    view_dispatcher_remove_view(lfrfid->view_dispatcher, LfRfidViewByteInput);
    byte_input_free(lfrfid->byte_input);

    // Number Input
    view_dispatcher_remove_view(lfrfid->view_dispatcher, LfRfidViewNumberInput);
    number_input_free(lfrfid->number_input);

    // Variable Item List - only allocated if the user opened a settings page
    if(lfrfid->variable_item_list) {
        view_dispatcher_remove_view(lfrfid->view_dispatcher, LfRfidViewVariableItemList);
        variable_item_list_free(lfrfid->variable_item_list);
    }

    // Read custom view
    view_dispatcher_remove_view(lfrfid->view_dispatcher, LfRfidViewRead);
    lfrfid_view_read_free(lfrfid->read_view);

    // View Dispatcher
    view_dispatcher_free(lfrfid->view_dispatcher);

    // Scene Manager
    scene_manager_free(lfrfid->scene_manager);

    // GUI
    furi_record_close(RECORD_GUI);
    lfrfid->gui = NULL;

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    lfrfid->notifications = NULL;

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_DIALOGS);

    free(lfrfid);
}

// The file the saved list opens on, for a launch argument naming a dump. Not this app's own
// string: the dump may have been renamed or deleted by the app that was handed it, and the
// argument can come from a favourite or the CLI, so anything that is not a file in this app's
// folder falls back to the folder rather than pointing the browser somewhere it cannot return
// from.
static void lfrfid_set_browse_path(LfRfid* app, const char* path) {
    // Asked of the storage service rather than compared here: /ext is case-insensitive and has
    // aliases, so a path that is in the folder can still fail a byte compare against it.
    if(!*path || !storage_common_is_subdir(app->storage, LFRFID_APP_FOLDER, path)) {
        FURI_LOG_W(TAG, "Browse path outside %s: %s", LFRFID_APP_FOLDER, path);
        furi_string_set(app->file_path, LFRFID_APP_FOLDER);
        return;
    }

    furi_string_set_str(app->file_path, path);
    if(storage_file_exists(app->storage, path)) return;

    // The usual way here: the dump was renamed or deleted in the app it was handed to, and the
    // argument was built before that happened. The folder it was in is still the right place to
    // open - the renamed file is in it - so only the name is given up.
    FURI_LOG_W(TAG, "Browse path is gone, opening its folder: %s", path);
    FuriString* folder = furi_string_alloc();
    path_extract_dirname(path, folder);
    furi_string_set(app->file_path, folder);
    furi_string_free(folder);
}

int32_t lfrfid_app(void* p) {
    LfRfid* app = lfrfid_alloc();
    char* args = p;

    lfrfid_make_app_folder(app);

    if(args && strlen(args)) {
        uint32_t rpc_ctx_ptr = 0;
        if(sscanf(args, "RPC %lX", &rpc_ctx_ptr) == 1) {
            app->rpc_ctx = (RpcAppSystem*)rpc_ctx_ptr;
            rpc_system_app_set_callback(app->rpc_ctx, rpc_command_callback, app);
            rpc_system_app_send_started(app->rpc_ctx);
            view_dispatcher_attach_to_gui(
                app->view_dispatcher, app->gui, ViewDispatcherTypeDesktop);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneRpc);
            dolphin_deed(DolphinDeedRfidEmulate);
        } else {
            furi_string_set(app->file_path, args);
            if(furi_string_end_withi(app->file_path, T5577_FILE_EXTENSION)) {
                // A dump is not a key this app can emulate, so a path to one means the saved
                // list opened on it: the argument this app queues for itself behind a handoff,
                // and what a favourite or the CLI gets instead of an app that quits on startup.
                // The list sits on the start menu, as it does when opened from there.
                lfrfid_set_browse_path(app, args);
                view_dispatcher_attach_to_gui(
                    app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
                scene_manager_set_scene_state(
                    app->scene_manager, LfRfidSceneStart, LfRfidMenuIndexSaved);
                scene_manager_next_scene(app->scene_manager, LfRfidSceneStart);
                scene_manager_next_scene(app->scene_manager, LfRfidSceneSelectKey);
            } else if(lfrfid_load_key_data(app, app->file_path, true)) {
                view_dispatcher_attach_to_gui(
                    app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
                scene_manager_next_scene(app->scene_manager, LfRfidSceneEmulate);
                dolphin_deed(DolphinDeedRfidEmulate);
            } else {
                view_dispatcher_stop(app->view_dispatcher);
            }
        }
    } else {
        view_dispatcher_attach_to_gui(
            app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
        scene_manager_next_scene(app->scene_manager, LfRfidSceneStart);
    }

    view_dispatcher_run(app->view_dispatcher);

    lfrfid_free(app);

    return 0;
}

bool lfrfid_save_key(LfRfid* app) {
    furi_assert(app);

    bool result = false;

    lfrfid_make_app_folder(app);

    if(furi_string_end_with(app->file_path, LFRFID_APP_FILENAME_EXTENSION)) {
        size_t filename_start = furi_string_search_rchar(app->file_path, '/');
        furi_string_left(app->file_path, filename_start);
    }

    furi_string_cat_printf(
        app->file_path,
        "/%s%s",
        furi_string_get_cstr(app->file_name),
        LFRFID_APP_FILENAME_EXTENSION);

    result = lfrfid_save_key_data(app, app->file_path);
    return result;
}

// Extensions are hidden in the list, so a dump and a key saved off the same tag come out as the
// same line. The icon is what tells them apart: a copy of the T5577 app's own glyph, kept in the
// firmware assets, so the same picture marks the file here, in the archive and in that app.
//
// The copy is the one loader_applications makes: a fixed size into a buffer the browser
// allocates at its own CUSTOM_ICON_MAX_SIZE, which is private to it and equal to this.
static bool lfrfid_file_browser_item_callback(
    FuriString* path,
    void* context,
    uint8_t** icon,
    FuriString* item_name) {
    UNUSED(context);
    UNUSED(item_name); // left empty, so the browser names the item the way it always does

    if(!furi_string_end_withi(path, T5577_FILE_EXTENSION)) return false;

    memcpy(*icon, icon_get_frame_data(&I_t5577_10px, 0), FAP_MANIFEST_MAX_ICON_SIZE);
    return true;
}

bool lfrfid_select_key_file(LfRfid* app) {
    furi_assert(app);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, LFRFID_APP_FILENAME_EXTENSION "|" T5577_FILE_EXTENSION, &I_125_10px);
    browser_options.base_path = LFRFID_APP_FOLDER;
    browser_options.item_loader_callback = lfrfid_file_browser_item_callback;

    // Input events and views are managed by file_browser
    return dialog_file_browser_show(
        app->dialogs, app->file_path, app->file_path, &browser_options);
}

// Something to read and an OK, for the one thing this app has to say that is not a key.
static void lfrfid_say(LfRfid* app, const char* header, const char* text) {
    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, header, 64, 4, AlignCenter, AlignTop);
    dialog_message_set_text(message, text, 64, 34, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, NULL, "OK", NULL);
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);
}

bool lfrfid_run_t5577_app(LfRfid* app) {
    furi_assert(app);

    // Asked here rather than left to the loader. The loader does report a missing app and still
    // runs the relaunch queued behind it, so the list does come back - but its answer is a bare
    // "App Not Found" that names nothing, for an app the user never asked to start. The .fap
    // ships in the firmware's SD resources, so the way to get it back is worth saying.
    if(!storage_file_exists(app->storage, T5577_APP_PATH)) {
        lfrfid_say(
            app, "No T5577 app", "It comes with the\nfirmware - install its\nresources to get it");
        return false;
    }

    Loader* loader = furi_record_open(RECORD_LOADER);

    loader_enqueue_launch(
        loader, T5577_APP_PATH, furi_string_get_cstr(app->file_path), LoaderDeferredLaunchFlagGui);

    // This app again behind that one, told to reopen the saved list on the file it handed over:
    // returning to a start menu would make a look at one dump cost the walk back to the list.
    // By launch path, so it works whether this app is built into the firmware or is a .fap.
    // Same handoff as nfc_app_run_external(), which chains mfkey32 the same way.
    FuriString* self_path = furi_string_alloc();
    if(loader_get_application_launch_path(loader, self_path)) {
        loader_enqueue_launch(
            loader,
            furi_string_get_cstr(self_path),
            furi_string_get_cstr(app->file_path),
            LoaderDeferredLaunchFlagGui);
    } else {
        // Not worth refusing the handoff over: the dump still opens, and the user comes back to
        // the desktop instead of to the list.
        FURI_LOG_W(TAG, "No launch path for self, the saved list will not reopen");
    }
    furi_string_free(self_path);
    furi_record_close(RECORD_LOADER);

    view_dispatcher_stop(app->view_dispatcher);
    return true;
}

bool lfrfid_load_raw_key_from_file_select(LfRfid* app) {
    furi_assert(app);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, ".raw", &I_125_10px);
    browser_options.base_path = LFRFID_APP_FOLDER;

    // Input events and views are managed by file_browser
    bool result =
        dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options);

    if(result) {
        // Extract .raw
        path_extract_filename(app->file_path, app->file_name, true);
        //path_extract_filename(app->file_name, app->file_name, true);
    }

    return result;
}

bool lfrfid_delete_key_file(LfRfid* app, const FuriString* path) {
    furi_assert(app);
    furi_assert(path);

    return storage_simply_remove(app->storage, furi_string_get_cstr(path));
}

bool lfrfid_delete_key(LfRfid* app) {
    furi_assert(app);

    return lfrfid_delete_key_file(app, app->file_path);
}

bool lfrfid_load_key_data(LfRfid* app, FuriString* path, bool show_dialog) {
    bool result = false;

    do {
        app->protocol_id = lfrfid_dict_file_load(app->dict, furi_string_get_cstr(path));
        if(app->protocol_id == PROTOCOL_NO) break;

        path_extract_filename(path, app->file_name, true);
        result = true;
    } while(0);

    if((!result) && (show_dialog)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot load\nkey file");
    }

    return result;
}

bool lfrfid_save_key_data(LfRfid* app, FuriString* path) {
    bool result = lfrfid_dict_file_save(app->dict, app->protocol_id, furi_string_get_cstr(path));

    if(!result) {
        dialog_message_show_storage_error(app->dialogs, "Cannot save\nkey file");
    }

    return result;
}

void lfrfid_make_app_folder(LfRfid* app) {
    furi_assert(app);

    if(!storage_simply_mkdir(app->storage, LFRFID_APP_FOLDER)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot create\napp folder");
    }
}

void lfrfid_text_store_set(LfRfid* app, const char* text, ...) {
    furi_assert(app);
    va_list args;
    va_start(args, text);

    // sizeof, not LFRFID_TEXT_STORE_SIZE: the buffer carries the extra byte for the terminator,
    // so passing the size without it cost every caller one character.
    vsnprintf(app->text_store, sizeof(app->text_store), text, args);

    va_end(args);
}

void lfrfid_text_store_clear(LfRfid* app) {
    furi_assert(app);
    memset(app->text_store, 0, sizeof(app->text_store));
}

void lfrfid_popup_timeout_callback(void* context) {
    LfRfid* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventPopupClosed);
}

void lfrfid_widget_callback(GuiButtonType result, InputType type, void* context) {
    LfRfid* app = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(app->view_dispatcher, result);
    }
}

void lfrfid_text_input_callback(void* context) {
    LfRfid* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventNext);
}
