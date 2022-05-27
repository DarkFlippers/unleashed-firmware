#include "ibutton.h"
#include "ibutton_i.h"
#include "ibutton/scenes/ibutton_scene.h"

#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>

static const NotificationSequence* ibutton_notification_sequences[] = {
    &sequence_error,
    &sequence_success,
    &sequence_blink_cyan_10,
    &sequence_blink_magenta_10,
    &sequence_blink_yellow_10,
    &sequence_set_red_255,
    &sequence_reset_red,
    &sequence_set_green_255,
    &sequence_reset_green,
};

static void ibutton_make_app_folder(iButton* ibutton) {
    if(!storage_simply_mkdir(ibutton->storage, IBUTTON_APP_FOLDER)) {
        dialog_message_show_storage_error(ibutton->dialogs, "Cannot create\napp folder");
    }
}

static bool ibutton_load_key_data(iButton* ibutton, string_t key_path) {
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

    if(!result) {
        dialog_message_show_storage_error(ibutton->dialogs, "Cannot load\nkey file");
    }

    return result;
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

    ibutton->gui = furi_record_open("gui");
    view_dispatcher_attach_to_gui(
        ibutton->view_dispatcher, ibutton->gui, ViewDispatcherTypeFullscreen);

    ibutton->storage = furi_record_open("storage");
    ibutton->dialogs = furi_record_open("dialogs");
    ibutton->notifications = furi_record_open("notification");

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

    furi_record_close("storage");
    ibutton->storage = NULL;

    furi_record_close("notification");
    ibutton->notifications = NULL;

    furi_record_close("dialogs");
    ibutton->dialogs = NULL;

    furi_record_close("gui");
    ibutton->gui = NULL;

    ibutton_worker_stop_thread(ibutton->key_worker);
    ibutton_worker_free(ibutton->key_worker);
    ibutton_key_free(ibutton->key);

    free(ibutton);
}

bool ibutton_file_select(iButton* ibutton) {
    bool success = dialog_file_select_show(
        ibutton->dialogs,
        IBUTTON_APP_FOLDER,
        IBUTTON_APP_EXTENSION,
        ibutton->file_name,
        IBUTTON_FILE_NAME_SIZE,
        ibutton_key_get_name_p(ibutton->key));

    if(success) {
        string_t key_str;
        string_init_printf(
            key_str, "%s/%s%s", IBUTTON_APP_FOLDER, ibutton->file_name, IBUTTON_APP_EXTENSION);
        success = ibutton_load_key_data(ibutton, key_str);

        if(success) {
            ibutton_key_set_name(ibutton->key, ibutton->file_name);
        }

        string_clear(key_str);
    }

    return success;
}

bool ibutton_load_key(iButton* ibutton, const char* key_name) {
    string_t key_path;
    string_init_set_str(key_path, key_name);

    const bool success = ibutton_load_key_data(ibutton, key_path);

    if(success) {
        path_extract_filename_no_ext(key_name, key_path);
        ibutton_key_set_name(ibutton->key, string_get_cstr(key_path));
    }

    string_clear(key_path);
    return success;
}

bool ibutton_save_key(iButton* ibutton, const char* key_name) {
    // Create ibutton directory if necessary
    ibutton_make_app_folder(ibutton);

    FlipperFormat* file = flipper_format_file_alloc(ibutton->storage);
    iButtonKey* key = ibutton->key;

    string_t key_file_name;
    bool result = false;
    string_init(key_file_name);

    do {
        // First remove key if it was saved (we rename the key)
        if(!ibutton_delete_key(ibutton)) break;

        // Save the key
        ibutton_key_set_name(key, key_name);

        // Set full file name, for new key
        string_printf(
            key_file_name,
            "%s/%s%s",
            IBUTTON_APP_FOLDER,
            ibutton_key_get_name_p(key),
            IBUTTON_APP_EXTENSION);

        // Open file for write
        if(!flipper_format_file_open_always(file, string_get_cstr(key_file_name))) break;

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

    string_clear(key_file_name);

    if(!result) {
        dialog_message_show_storage_error(ibutton->dialogs, "Cannot save\nkey file");
    }

    return result;
}

bool ibutton_delete_key(iButton* ibutton) {
    string_t file_name;
    bool result = false;

    string_init_printf(
        file_name,
        "%s/%s%s",
        IBUTTON_APP_FOLDER,
        ibutton_key_get_name_p(ibutton->key),
        IBUTTON_APP_EXTENSION);
    result = storage_simply_remove(ibutton->storage, string_get_cstr(file_name));
    string_clear(file_name);

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

void ibutton_switch_to_previous_scene_one_of(
    iButton* ibutton,
    const uint32_t* scene_ids,
    size_t scene_ids_size) {
    furi_assert(scene_ids_size);
    SceneManager* scene_manager = ibutton->scene_manager;

    for(size_t i = 0; i < scene_ids_size; ++i) {
        const uint32_t scene_id = scene_ids[i];
        if(scene_manager_has_previous_scene(scene_manager, scene_id)) {
            scene_manager_search_and_switch_to_previous_scene(scene_manager, scene_id);
            return;
        }
    }
}

void ibutton_notification_message(iButton* ibutton, uint32_t message) {
    furi_assert(message < sizeof(ibutton_notification_sequences) / sizeof(NotificationSequence*));
    notification_message(ibutton->notifications, ibutton_notification_sequences[message]);
}

int32_t ibutton_app(void* p) {
    iButton* ibutton = ibutton_alloc();

    ibutton_make_app_folder(ibutton);

    if(p && ibutton_load_key(ibutton, (const char*)p)) {
        // TODO: Display an error if the key from p could not be loaded
        scene_manager_next_scene(ibutton->scene_manager, iButtonSceneEmulate);
    } else {
        scene_manager_next_scene(ibutton->scene_manager, iButtonSceneStart);
    }

    view_dispatcher_run(ibutton->view_dispatcher);

    ibutton_free(ibutton);
    return 0;
}
