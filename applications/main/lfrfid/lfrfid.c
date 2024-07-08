#include "lfrfid_i.h"
#include <dolphin/dolphin.h>

//TODO: use .txt file in resources for passwords.
const uint32_t default_passwords[] = {
    0x00000000, 0x00000001, 0x00000002, 0x0000000A, 0x0000000B, 0x00012323, 0x000D8787, 0x00434343,
    0x01010101, 0x01020304, 0x01234567, 0x02030405, 0x03040506, 0x04050607, 0x05060708, 0x05D73B9F,
    0x06070809, 0x0708090A, 0x07CEE75D, 0x07D7BB0B, 0x08090A0B, 0x090A0B0C, 0x0A0B0C0D, 0x0B0C0D0E,
    0x0C0D0E0F, 0x0CB7E7FC, 0x10000000, 0x10041004, 0x10101010, 0x11111111, 0x11112222, 0x11223344,
    0x12121212, 0x121AD038, 0x12341234, 0x12344321, 0x12345678, 0x1234ABCD, 0x126C248A, 0x13131313,
    0x19721972, 0x19920427, 0x1C0B5848, 0x20000000, 0x20002000, 0x20206666, 0x22222222, 0x22334455,
    0x27182818, 0x30000000, 0x31415926, 0x314159E0, 0x33333333, 0x33445566, 0x40000000, 0x44444444,
    0x444E4752, 0x44556677, 0x44B44CAE, 0x4E457854, 0x4F271149, 0x50000000, 0x50415353, 0x50520901,
    0x50524F58, 0x51243648, 0x5469616E, 0x55555555, 0x55667788, 0x55AA55AA, 0x575F4F4B, 0x57721566,
    0x60000000, 0x65857569, 0x66666666, 0x66778899, 0x69314718, 0x69696969, 0x70000000, 0x7686962A,
    0x77777777, 0x778899AA, 0x7854794A, 0x80000000, 0x87654321, 0x88661858, 0x88888888, 0x8899AABB,
    0x89A69E60, 0x90000000, 0x932D9963, 0x93C467E3, 0x9636EF8F, 0x99999999, 0x99AABBCC, 0x9E3779B9,
    0xA0000000, 0xA0A1A2A3, 0xA5B4C3D2, 0xAA55AA55, 0xAA55BBBB, 0xAAAAAAAA, 0xAABBCCDD, 0xABCD1234,
    0xB0000000, 0xB0B1B2B3, 0xB5F44686, 0xBBBBBBBB, 0xBBCCDDEE, 0xC0000000, 0xC0F5009A, 0xC6EF3720,
    0xCCCCCCCC, 0xCCDDEEFF, 0xD0000000, 0xDDDDDDDD, 0xDEADC0DE, 0xE0000000, 0xE4204998, 0xE9920427,
    0xEEEEEEEE, 0xF0000000, 0xF1EA5EED, 0xF9DCEBA0, 0xFABADA11, 0xFEEDBEEF, 0xFFFFFFFF};

const uint32_t* lfrfid_get_t5577_default_passwords(uint8_t* len) {
    *len = sizeof(default_passwords) / sizeof(uint32_t);
    return default_passwords;
}

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
            dolphin_deed(DolphinDeedRfidEmulate);
        } else {
            furi_string_set(app->file_path, args);
            if(lfrfid_load_key_data(app, app->file_path, true)) {
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

bool lfrfid_load_key_from_file_select(LfRfid* app) {
    furi_assert(app);

    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(
        &browser_options, LFRFID_APP_FILENAME_EXTENSION, &I_125_10px);
    browser_options.base_path = LFRFID_APP_FOLDER;

    // Input events and views are managed by file_browser
    bool result =
        dialog_file_browser_show(app->dialogs, app->file_path, app->file_path, &browser_options);

    if(result) {
        result = lfrfid_load_key_data(app, app->file_path, true);
    }

    return result;
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

bool lfrfid_delete_key(LfRfid* app) {
    furi_assert(app);

    return storage_simply_remove(app->storage, furi_string_get_cstr(app->file_path));
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
