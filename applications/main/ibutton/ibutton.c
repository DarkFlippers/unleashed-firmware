#include "ibutton_i.h"

#include <toolbox/path.h>
#include <dolphin/dolphin.h>

#define TAG "IButtonApp"

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
    Storage* storage = furi_record_open(RECORD_STORAGE);

    if(!storage_simply_mkdir(storage, IBUTTON_APP_FOLDER)) {
        dialog_message_show_storage_error(ibutton->dialogs, "Cannot create\napp folder");
    }

    furi_record_close(RECORD_STORAGE);
}

static void ibutton_rpc_command_callback(const RpcAppSystemEvent* event, void* context) {
    furi_assert(context);
    iButton* ibutton = context;

    if(event->type == RpcAppEventTypeSessionClose) {
        view_dispatcher_send_custom_event(
            ibutton->view_dispatcher, iButtonCustomEventRpcSessionClose);
        rpc_system_app_set_callback(ibutton->rpc, NULL, NULL);
        ibutton->rpc = NULL;
    } else if(event->type == RpcAppEventTypeAppExit) {
        view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventRpcExit);
    } else if(event->type == RpcAppEventTypeLoadFile) {
        furi_assert(event->data.type == RpcAppSystemEventDataTypeString);
        furi_string_set(ibutton->file_path, event->data.string);
        view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventRpcLoadFile);
    } else {
        rpc_system_app_confirm(ibutton->rpc, false);
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

iButton* ibutton_alloc(void) {
    iButton* ibutton = malloc(sizeof(iButton));

    ibutton->file_path = furi_string_alloc();

    ibutton->scene_manager = scene_manager_alloc(&ibutton_scene_handlers, ibutton);

    ibutton->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(ibutton->view_dispatcher, ibutton);
    view_dispatcher_set_custom_event_callback(
        ibutton->view_dispatcher, ibutton_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        ibutton->view_dispatcher, ibutton_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        ibutton->view_dispatcher, ibutton_tick_event_callback, 100);

    ibutton->gui = furi_record_open(RECORD_GUI);

    ibutton->dialogs = furi_record_open(RECORD_DIALOGS);
    ibutton->notifications = furi_record_open(RECORD_NOTIFICATION);

    ibutton->protocols = ibutton_protocols_alloc();
    ibutton->key = ibutton_key_alloc(ibutton_protocols_get_max_data_size(ibutton->protocols));
    ibutton->worker = ibutton_worker_alloc(ibutton->protocols);
    ibutton_worker_start_thread(ibutton->worker);

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

    ibutton->loading = loading_alloc();
    view_dispatcher_add_view(
        ibutton->view_dispatcher, iButtonViewLoading, loading_get_view(ibutton->loading));

    return ibutton;
}

void ibutton_free(iButton* ibutton) {
    furi_assert(ibutton);

    view_dispatcher_remove_view(ibutton->view_dispatcher, iButtonViewLoading);
    loading_free(ibutton->loading);

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

    furi_record_close(RECORD_NOTIFICATION);
    ibutton->notifications = NULL;

    furi_record_close(RECORD_DIALOGS);
    ibutton->dialogs = NULL;

    furi_record_close(RECORD_GUI);
    ibutton->gui = NULL;

    ibutton_worker_stop_thread(ibutton->worker);
    ibutton_worker_free(ibutton->worker);
    ibutton_key_free(ibutton->key);
    ibutton_protocols_free(ibutton->protocols);

    furi_string_free(ibutton->file_path);

    free(ibutton);
}

bool ibutton_load_key(iButton* ibutton, bool show_error) {
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewLoading);

    const bool success = ibutton_protocols_load(
        ibutton->protocols, ibutton->key, furi_string_get_cstr(ibutton->file_path));

    if(success) {
        FuriString* tmp = furi_string_alloc();

        path_extract_filename(ibutton->file_path, tmp, true);
        strlcpy(ibutton->key_name, furi_string_get_cstr(tmp), IBUTTON_KEY_NAME_SIZE);

        furi_string_free(tmp);
    } else if(show_error) {
        dialog_message_show_storage_error(ibutton->dialogs, "Cannot load\nkey file");
    }

    return success;
}

bool ibutton_select_and_load_key(iButton* ibutton) {
    DialogsFileBrowserOptions browser_options;
    bool success = false;
    dialog_file_browser_set_basic_options(
        &browser_options, IBUTTON_APP_FILENAME_EXTENSION, &I_ibutt_10px);
    browser_options.base_path = IBUTTON_APP_FOLDER;

    if(furi_string_empty(ibutton->file_path)) {
        furi_string_set(ibutton->file_path, browser_options.base_path);
    }

    do {
        if(!dialog_file_browser_show(
               ibutton->dialogs, ibutton->file_path, ibutton->file_path, &browser_options))
            break;
        success = ibutton_load_key(ibutton, true);
    } while(!success);

    return success;
}

bool ibutton_save_key(iButton* ibutton) {
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewLoading);

    ibutton_make_app_folder(ibutton);

    iButtonKey* key = ibutton->key;
    const bool success =
        ibutton_protocols_save(ibutton->protocols, key, furi_string_get_cstr(ibutton->file_path));

    if(!success) {
        dialog_message_show_storage_error(ibutton->dialogs, "Cannot save\nkey file");
    }

    return success;
}

bool ibutton_delete_key(iButton* ibutton) {
    bool result = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    result = storage_simply_remove(storage, furi_string_get_cstr(ibutton->file_path));
    furi_record_close(RECORD_STORAGE);

    ibutton_reset_key(ibutton);

    return result;
}

void ibutton_reset_key(iButton* ibutton) {
    ibutton->key_name[0] = '\0';
    furi_string_reset(ibutton->file_path);
    ibutton_key_reset(ibutton->key);
}

void ibutton_notification_message(iButton* ibutton, uint32_t message) {
    furi_assert(message < sizeof(ibutton_notification_sequences) / sizeof(NotificationSequence*));
    notification_message(ibutton->notifications, ibutton_notification_sequences[message]);
}

void ibutton_submenu_callback(void* context, uint32_t index) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, index);
}

void ibutton_widget_callback(GuiButtonType result, InputType type, void* context) {
    iButton* ibutton = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(ibutton->view_dispatcher, result);
    }
}

int32_t ibutton_app(void* arg) {
    iButton* ibutton = ibutton_alloc();

    ibutton_make_app_folder(ibutton);

    bool key_loaded = false;

    if((arg != NULL) && (strlen(arg) != 0)) {
        if(sscanf(arg, "RPC %lX", (uint32_t*)&ibutton->rpc) == 1) {
            FURI_LOG_D(TAG, "Running in RPC mode");

            rpc_system_app_set_callback(ibutton->rpc, ibutton_rpc_command_callback, ibutton);
            rpc_system_app_send_started(ibutton->rpc);

        } else {
            furi_string_set(ibutton->file_path, (const char*)arg);
            key_loaded = ibutton_load_key(ibutton, true);
        }
    }

    if(ibutton->rpc != NULL) {
        view_dispatcher_attach_to_gui(
            ibutton->view_dispatcher, ibutton->gui, ViewDispatcherTypeDesktop);
        scene_manager_next_scene(ibutton->scene_manager, iButtonSceneRpc);
        dolphin_deed(DolphinDeedIbuttonEmulate);

    } else {
        view_dispatcher_attach_to_gui(
            ibutton->view_dispatcher, ibutton->gui, ViewDispatcherTypeFullscreen);
        if(key_loaded) { //-V547
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneEmulate);
            dolphin_deed(DolphinDeedIbuttonEmulate);
        } else {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneStart);
        }
    }

    view_dispatcher_run(ibutton->view_dispatcher);

    if(ibutton->rpc) {
        rpc_system_app_set_callback(ibutton->rpc, NULL, NULL);
        rpc_system_app_send_exited(ibutton->rpc);
    }
    ibutton_free(ibutton);
    return 0;
}
