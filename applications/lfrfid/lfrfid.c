#include "lfrfid_i.h"

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

static void rpc_command_callback(RpcAppSystemEvent rpc_event, void* context) {
    furi_assert(context);
    LfRfid* app = (LfRfid*)context;

    if(rpc_event == RpcAppEventSessionClose) {
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventRpcSessionClose);
        // Detach RPC
        rpc_system_app_set_callback(app->rpc_ctx, NULL, NULL);
        app->rpc_ctx = NULL;
    } else if(rpc_event == RpcAppEventAppExit) {
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventExit);
    } else if(rpc_event == RpcAppEventLoadFile) {
        view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventRpcLoadFile);
    } else {
        rpc_system_app_confirm(app->rpc_ctx, rpc_event, false);
    }
}

static LfRfid* lfrfid_alloc() {
    LfRfid* lfrfid = malloc(sizeof(LfRfid));

    lfrfid->storage = furi_record_open(RECORD_STORAGE);
    lfrfid->dialogs = furi_record_open(RECORD_DIALOGS);

    string_init(lfrfid->file_name);
    string_init(lfrfid->raw_file_name);
    string_init_set_str(lfrfid->file_path, LFRFID_APP_FOLDER);

    lfrfid->dict = protocol_dict_alloc(lfrfid_protocols, LFRFIDProtocolMax);

    size_t size = protocol_dict_get_max_data_size(lfrfid->dict);
    lfrfid->new_key_data = (uint8_t*)malloc(size);
    lfrfid->old_key_data = (uint8_t*)malloc(size);

    lfrfid->lfworker = lfrfid_worker_alloc(lfrfid->dict);

    lfrfid->view_dispatcher = view_dispatcher_alloc();
    lfrfid->scene_manager = scene_manager_alloc(&lfrfid_scene_handlers, lfrfid);
    view_dispatcher_enable_queue(lfrfid->view_dispatcher);
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

    // Read custom view
    lfrfid->read_view = lfrfid_view_read_alloc();
    view_dispatcher_add_view(
        lfrfid->view_dispatcher, LfRfidViewRead, lfrfid_view_read_get_view(lfrfid->read_view));

    return lfrfid;
}

static void lfrfid_free(LfRfid* lfrfid) {
    furi_assert(lfrfid);

    string_clear(lfrfid->raw_file_name);
    string_clear(lfrfid->file_name);
    string_clear(lfrfid->file_path);
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
        } else {
            string_set_str(app->file_path, args);
            lfrfid_load_key_data(app, app->file_path, true);
            view_dispatcher_attach_to_gui(
                app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
            scene_manager_next_scene(app->scene_manager, LfRfidSceneEmulate);
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

    if(string_end_with_str_p(app->file_path, LFRFID_APP_EXTENSION)) {
        size_t filename_start = string_search_rchar(app->file_path, '/');
        string_left(app->file_path, filename_start);
    }

    string_cat_printf(
        app->file_path, "/%s%s", string_get_cstr(app->file_name), LFRFID_APP_EXTENSION);

    result = lfrfid_save_key_data(app, app->file_path);
    return result;
}

bool lfrfid_load_key_from_file_select(LfRfid* app) {
    furi_assert(app);

    bool result = dialog_file_browser_show(
        app->dialogs,
        app->file_path,
        app->file_path,
        LFRFID_APP_EXTENSION,
        true,
        &I_125_10px,
        true);

    if(result) {
        result = lfrfid_load_key_data(app, app->file_path, true);
    }

    return result;
}

bool lfrfid_delete_key(LfRfid* app) {
    furi_assert(app);

    return storage_simply_remove(app->storage, string_get_cstr(app->file_path));
}

bool lfrfid_load_key_data(LfRfid* app, string_t path, bool show_dialog) {
    bool result = false;

    do {
        app->protocol_id = lfrfid_dict_file_load(app->dict, string_get_cstr(path));
        if(app->protocol_id == PROTOCOL_NO) break;

        path_extract_filename(path, app->file_name, true);
        result = true;
    } while(0);

    if((!result) && (show_dialog)) {
        dialog_message_show_storage_error(app->dialogs, "Cannot load\nkey file");
    }

    return result;
}

bool lfrfid_save_key_data(LfRfid* app, string_t path) {
    bool result = lfrfid_dict_file_save(app->dict, app->protocol_id, string_get_cstr(path));

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

    vsnprintf(app->text_store, LFRFID_TEXT_STORE_SIZE, text, args);

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