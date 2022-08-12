#include "ibutton.h"
#include "assets_icons.h"
#include "ibutton_i.h"
#include "ibutton/scenes/ibutton_scene.h"
#include "m-string.h"
#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>
#include <rpc/rpc_app.h>

#define TAG "iButtonApp"

static const NotificationSequence sequence_blink_set_yellow = {
    &message_blink_set_color_yellow,
    NULL,
};

static const NotificationSequence sequence_blink_set_magenta = {
    &message_blink_set_color_magenta,
    NULL,
};

static const NotificationSequence* ibutton_notification_sequences[] = {
    &sequence_error,
    &sequence_success,
    &sequence_blink_start_cyan,
    &sequence_blink_start_magenta,
    &sequence_blink_set_yellow,
    &sequence_blink_set_magenta,
    &sequence_set_red_255,
    &sequence_reset_red,
    &sequence_set_green_255,
    &sequence_reset_green,
    &sequence_blink_stop,
};

static void ibutton_make_app_folder(iButton* ibutton) {
    if(!storage_simply_mkdir(ibutton->storage, IBUTTON_APP_FOLDER)) {
        dialog_message_show_storage_error(ibutton->dialogs, "Cannot create\napp folder");
    }
}

bool ibutton_load_key_data(iButton* ibutton, string_t key_path, bool show_dialog) {
    FlipperFormat* file = flipper_format_file_alloc(ibutton->storage);
    bool result = false;
    string_t data;
    string_init(data);

    do {
        if(!flipper_format_file_open_existing(file, string_get_cstr(key_path))) break;

        // header
        uint32_t version;
        if(!flipper_format_read_header(file, data, &version)) break;
        if(string_cmp_str(data, IBUTTON_APP_FILE_TYPE) != 0) break;
        if(version != 1) break;

        // key type
        iButtonKeyType type;
        if(!flipper_format_read_string(file, "Key type", data)) break;
        if(!ibutton_key_get_type_by_string(string_get_cstr(data), &type)) break;

        // key data
        uint8_t key_data[IBUTTON_KEY_DATA_SIZE] = {0};
        if(!flipper_format_read_hex(file, "Data", key_data, ibutton_key_get_size_by_type(type)))
            break;

        ibutton_key_set_type(ibutton->key, type);
        ibutton_key_set_data(ibutton->key, key_data, IBUTTON_KEY_DATA_SIZE);

        result = true;
    } while(false);

    flipper_format_free(file);
    string_clear(data);

    if((!result) && (show_dialog)) {
        dialog_message_show_storage_error(ibutton->dialogs, "Cannot load\nkey file");
    }

    return result;
}

static void ibutton_rpc_command_callback(RpcAppSystemEvent event, void* context) {
    furi_assert(context);
    iButton* ibutton = context;

    if(event == RpcAppEventSessionClose) {
        view_dispatcher_send_custom_event(
            ibutton->view_dispatcher, iButtonCustomEventRpcSessionClose);
        rpc_system_app_set_callback(ibutton->rpc_ctx, NULL, NULL);
        ibutton->rpc_ctx = NULL;
    } else if(event == RpcAppEventAppExit) {
        view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventRpcExit);
    } else if(event == RpcAppEventLoadFile) {
        view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventRpcLoad);
    } else {
        rpc_system_app_confirm(ibutton->rpc_ctx, event, false);
    }
}

bool ibutton_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    iButton* ibutton = context;
    return scene_manager_handle_custom_event(ibutton->scene_manager, event);
}

bool ibutton_back_event_callback(void* context) {
    furi_assert(context);
    iButton* ibutton = context;
    return scene_manager_handle_back_event(ibutton->scene_manager);
}

void ibutton_tick_event_callback(void* context) {
    furi_assert(context);
    iButton* ibutton = context;
    scene_manager_handle_tick_event(ibutton->scene_manager);
}

iButton* ibutton_alloc() {
    iButton* ibutton = malloc(sizeof(iButton));

    string_init(ibutton->file_path);

    ibutton->scene_manager = scene_manager_alloc(&ibutton_scene_handlers, ibutton);

    ibutton->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(ibutton->view_dispatcher);
    view_dispatcher_set_event_callback_context(ibutton->view_dispatcher, ibutton);
    view_dispatcher_set_custom_event_callback(
        ibutton->view_dispatcher, ibutton_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        ibutton->view_dispatcher, ibutton_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        ibutton->view_dispatcher, ibutton_tick_event_callback, 100);

    ibutton->gui = furi_record_open(RECORD_GUI);

    ibutton->storage = furi_record_open(RECORD_STORAGE);
    ibutton->dialogs = furi_record_open(RECORD_DIALOGS);
    ibutton->notifications = furi_record_open(RECORD_NOTIFICATION);

    ibutton->key = ibutton_key_alloc();
    ibutton->key_worker = ibutton_worker_alloc();
    ibutton_worker_start_thread(ibutton->key_worker);

    ibutton->submenu = submenu_alloc();
    view_dispatcher_add_view(
        ibutton->view_dispatcher, iButtonViewSubmenu, submenu_get_view(ibutton->submenu));

    ibutton->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        ibutton->view_dispatcher, iButtonViewByteInput, byte_input_get_view(ibutton->byte_input));

    ibutton->text_input = text_input_alloc();
    view_dispatcher_add_view(
        ibutton->view_dispatcher, iButtonViewTextInput, text_input_get_view(ibutton->text_input));

    ibutton->popup = popup_alloc();
    view_dispatcher_add_view(
        ibutton->view_dispatcher, iButtonViewPopup, popup_get_view(ibutton->popup));

    ibutton->widget = widget_alloc();
    view_dispatcher_add_view(
        ibutton->view_dispatcher, iButtonViewWidget, widget_get_view(ibutton->widget));

    ibutton->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        ibutton->view_dispatcher, iButtonViewDialogEx, dialog_ex_get_view(ibutton->dialog_ex));

    return ibutton;
}

void ibutton_free(iButton* ibutton) {
    furi_assert(ibutton);

    view_dispatcher_remove_view(ibutton->view_dispatcher, iButtonViewDialogEx);
    dialog_ex_free(ibutton->dialog_ex);

    view_dispatcher_remove_view(ibutton->view_dispatcher, iButtonViewWidget);
    widget_free(ibutton->widget);

    view_dispatcher_remove_view(ibutton->view_dispatcher, iButtonViewPopup);
    popup_free(ibutton->popup);

    view_dispatcher_remove_view(ibutton->view_dispatcher, iButtonViewTextInput);
    text_input_free(ibutton->text_input);

    view_dispatcher_remove_view(ibutton->view_dispatcher, iButtonViewByteInput);
    byte_input_free(ibutton->byte_input);

    view_dispatcher_remove_view(ibutton->view_dispatcher, iButtonViewSubmenu);
    submenu_free(ibutton->submenu);

    view_dispatcher_free(ibutton->view_dispatcher);
    scene_manager_free(ibutton->scene_manager);

    furi_record_close(RECORD_STORAGE);
    ibutton->storage = NULL;

    furi_record_close(RECORD_NOTIFICATION);
    ibutton->notifications = NULL;

    furi_record_close(RECORD_DIALOGS);
    ibutton->dialogs = NULL;

    furi_record_close(RECORD_GUI);
    ibutton->gui = NULL;

    ibutton_worker_stop_thread(ibutton->key_worker);
    ibutton_worker_free(ibutton->key_worker);
    ibutton_key_free(ibutton->key);

    string_clear(ibutton->file_path);

    free(ibutton);
}

bool ibutton_file_select(iButton* ibutton) {
    bool success = dialog_file_browser_show(
        ibutton->dialogs,
        ibutton->file_path,
        ibutton->file_path,
        IBUTTON_APP_EXTENSION,
        true,
        &I_ibutt_10px,
        true);

    if(success) {
        success = ibutton_load_key_data(ibutton, ibutton->file_path, true);
    }

    return success;
}

bool ibutton_save_key(iButton* ibutton, const char* key_name) {
    // Create ibutton directory if necessary
    ibutton_make_app_folder(ibutton);

    FlipperFormat* file = flipper_format_file_alloc(ibutton->storage);
    iButtonKey* key = ibutton->key;

    bool result = false;

    do {
        // Check if we has old key
        if(string_end_with_str_p(ibutton->file_path, IBUTTON_APP_EXTENSION)) {
            // First remove old key
            ibutton_delete_key(ibutton);

            // Remove old key name from path
            size_t filename_start = string_search_rchar(ibutton->file_path, '/');
            string_left(ibutton->file_path, filename_start);
        }

        string_cat_printf(ibutton->file_path, "/%s%s", key_name, IBUTTON_APP_EXTENSION);

        // Open file for write
        if(!flipper_format_file_open_always(file, string_get_cstr(ibutton->file_path))) break;

        // Write header
        if(!flipper_format_write_header_cstr(file, IBUTTON_APP_FILE_TYPE, 1)) break;

        // Write key type
        if(!flipper_format_write_comment_cstr(file, "Key type can be Cyfral, Dallas or Metakom"))
            break;
        const char* key_type = ibutton_key_get_string_by_type(ibutton_key_get_type(key));
        if(!flipper_format_write_string_cstr(file, "Key type", key_type)) break;

        // Write data
        if(!flipper_format_write_comment_cstr(
               file, "Data size for Cyfral is 2, for Metakom is 4, for Dallas is 8"))
            break;

        if(!flipper_format_write_hex(
               file, "Data", ibutton_key_get_data_p(key), ibutton_key_get_data_size(key)))
            break;
        result = true;

    } while(false);

    flipper_format_free(file);

    if(!result) {
        dialog_message_show_storage_error(ibutton->dialogs, "Cannot save\nkey file");
    }

    return result;
}

bool ibutton_delete_key(iButton* ibutton) {
    bool result = false;
    result = storage_simply_remove(ibutton->storage, string_get_cstr(ibutton->file_path));

    return result;
}

void ibutton_text_store_set(iButton* ibutton, const char* text, ...) {
    va_list args;
    va_start(args, text);

    vsnprintf(ibutton->text_store, IBUTTON_TEXT_STORE_SIZE, text, args);

    va_end(args);
}

void ibutton_text_store_clear(iButton* ibutton) {
    memset(ibutton->text_store, 0, IBUTTON_TEXT_STORE_SIZE);
}

void ibutton_notification_message(iButton* ibutton, uint32_t message) {
    furi_assert(message < sizeof(ibutton_notification_sequences) / sizeof(NotificationSequence*));
    notification_message(ibutton->notifications, ibutton_notification_sequences[message]);
}

int32_t ibutton_app(void* p) {
    iButton* ibutton = ibutton_alloc();

    ibutton_make_app_folder(ibutton);

    bool key_loaded = false;
    bool rpc_mode = false;

    if(p && strlen(p)) {
        uint32_t rpc_ctx = 0;
        if(sscanf(p, "RPC %lX", &rpc_ctx) == 1) {
            FURI_LOG_D(TAG, "Running in RPC mode");
            ibutton->rpc_ctx = (void*)rpc_ctx;
            rpc_mode = true;
            rpc_system_app_set_callback(ibutton->rpc_ctx, ibutton_rpc_command_callback, ibutton);
            rpc_system_app_send_started(ibutton->rpc_ctx);
        } else {
            string_set_str(ibutton->file_path, (const char*)p);
            if(ibutton_load_key_data(ibutton, ibutton->file_path, true)) {
                key_loaded = true;
                // TODO: Display an error if the key from p could not be loaded
            }
        }
    }

    if(rpc_mode) {
        view_dispatcher_attach_to_gui(
            ibutton->view_dispatcher, ibutton->gui, ViewDispatcherTypeDesktop);
        scene_manager_next_scene(ibutton->scene_manager, iButtonSceneRpc);
    } else {
        view_dispatcher_attach_to_gui(
            ibutton->view_dispatcher, ibutton->gui, ViewDispatcherTypeFullscreen);
        if(key_loaded) {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneEmulate);
        } else {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneStart);
        }
    }

    view_dispatcher_run(ibutton->view_dispatcher);

    if(ibutton->rpc_ctx) {
        rpc_system_app_set_callback(ibutton->rpc_ctx, NULL, NULL);
        rpc_system_app_send_exited(ibutton->rpc_ctx);
    }
    ibutton_free(ibutton);
    return 0;
}
