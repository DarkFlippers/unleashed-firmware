#include "ibutton_app.h"
#include <stdarg.h>
#include <furi_hal.h>
#include <m-string.h>
#include <toolbox/path.h>
#include <flipper_format/flipper_format.h>

const char* iButtonApp::app_folder = "/any/ibutton";
const char* iButtonApp::app_extension = ".ibtn";
const char* iButtonApp::app_filetype = "Flipper iButton key";

void iButtonApp::run(void* args) {
    iButtonEvent event;
    bool consumed;
    bool exit = false;

    make_app_folder();

    if(args && load_key((const char*)args)) {
        current_scene = Scene::SceneEmulate;
    }

    scenes[current_scene]->on_enter(this);

    while(!exit) {
        view.receive_event(&event);

        consumed = scenes[current_scene]->on_event(this, &event);

        if(!consumed) {
            if(event.type == iButtonEvent::Type::EventTypeBack) {
                exit = switch_to_previous_scene();
            }
        }
    };

    scenes[current_scene]->on_exit(this);
}

iButtonApp::iButtonApp()
    : notification{"notification"}
    , storage{"storage"}
    , dialogs{"dialogs"} {
    key = ibutton_key_alloc();
    key_worker = ibutton_worker_alloc();
    ibutton_worker_start_thread(key_worker);
}

iButtonApp::~iButtonApp() {
    for(std::map<Scene, iButtonScene*>::iterator it = scenes.begin(); it != scenes.end(); ++it) {
        delete it->second;
    }
    scenes.clear();

    ibutton_worker_stop_thread(key_worker);
    ibutton_worker_free(key_worker);
    ibutton_key_free(key);
}

iButtonAppViewManager* iButtonApp::get_view_manager() {
    return &view;
}

void iButtonApp::switch_to_next_scene(Scene next_scene) {
    previous_scenes_list.push_front(current_scene);

    if(next_scene != Scene::SceneExit) {
        scenes[current_scene]->on_exit(this);
        current_scene = next_scene;
        scenes[current_scene]->on_enter(this);
    }
}

void iButtonApp::search_and_switch_to_previous_scene(std::initializer_list<Scene> scenes_list) {
    Scene previous_scene = Scene::SceneStart;
    bool scene_found = false;

    while(!scene_found) {
        previous_scene = get_previous_scene();
        for(Scene element : scenes_list) {
            if(previous_scene == element || previous_scene == Scene::SceneStart) {
                scene_found = true;
                break;
            }
        }
    }

    scenes[current_scene]->on_exit(this);
    current_scene = previous_scene;
    scenes[current_scene]->on_enter(this);
}

bool iButtonApp::switch_to_previous_scene(uint8_t count) {
    Scene previous_scene = Scene::SceneStart;

    for(uint8_t i = 0; i < count; i++) {
        previous_scene = get_previous_scene();
        if(previous_scene == Scene::SceneExit) break;
    }

    if(previous_scene == Scene::SceneExit) {
        return true;
    } else {
        scenes[current_scene]->on_exit(this);
        current_scene = previous_scene;
        scenes[current_scene]->on_enter(this);
        return false;
    }
}

iButtonApp::Scene iButtonApp::get_previous_scene() {
    Scene scene = previous_scenes_list.front();
    previous_scenes_list.pop_front();
    return scene;
}

iButtonWorker* iButtonApp::get_key_worker() {
    return key_worker;
}

iButtonKey* iButtonApp::get_key() {
    return key;
}

char* iButtonApp::get_file_name() {
    return file_name;
}

uint8_t iButtonApp::get_file_name_size() {
    return file_name_size;
}

void iButtonApp::notify_read() {
    notification_message(notification, &sequence_blink_cyan_10);
}

void iButtonApp::notify_emulate() {
    notification_message(notification, &sequence_blink_magenta_10);
}

void iButtonApp::notify_yellow_blink() {
    notification_message(notification, &sequence_blink_yellow_10);
}

void iButtonApp::notify_error() {
    notification_message(notification, &sequence_error);
}

void iButtonApp::notify_success() {
    notification_message(notification, &sequence_success);
}

void iButtonApp::notify_green_on() {
    notification_message_block(notification, &sequence_set_green_255);
}

void iButtonApp::notify_green_off() {
    notification_message(notification, &sequence_reset_green);
}

void iButtonApp::notify_red_on() {
    notification_message_block(notification, &sequence_set_red_255);
}

void iButtonApp::notify_red_off() {
    notification_message(notification, &sequence_reset_red);
}

void iButtonApp::set_text_store(const char* text...) {
    va_list args;
    va_start(args, text);

    vsnprintf(text_store, text_store_size, text, args);

    va_end(args);
}

char* iButtonApp::get_text_store() {
    return text_store;
}

uint8_t iButtonApp::get_text_store_size() {
    return text_store_size;
}

// file managment
bool iButtonApp::save_key(const char* key_name) {
    // Create ibutton directory if necessary
    make_app_folder();

    FlipperFormat* file = flipper_format_file_alloc(storage);
    string_t key_file_name;
    bool result = false;
    string_init(key_file_name);

    do {
        // First remove key if it was saved (we rename the key)
        if(!delete_key()) break;

        // Save the key
        ibutton_key_set_name(key, key_name);

        // Set full file name, for new key
        string_printf(
            key_file_name, "%s/%s%s", app_folder, ibutton_key_get_name_p(key), app_extension);

        // Open file for write
        if(!flipper_format_file_open_always(file, string_get_cstr(key_file_name))) break;

        // Write header
        if(!flipper_format_write_header_cstr(file, iButtonApp::app_filetype, 1)) break;

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
        dialog_message_show_storage_error(dialogs, "Cannot save\nkey file");
    }

    return result;
}

bool iButtonApp::load_key_data(string_t key_path) {
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool result = false;
    string_t data;
    string_init(data);

    do {
        if(!flipper_format_file_open_existing(file, string_get_cstr(key_path))) break;

        // header
        uint32_t version;
        if(!flipper_format_read_header(file, data, &version)) break;
        if(string_cmp_str(data, iButtonApp::app_filetype) != 0) break;
        if(version != 1) break;

        // key type
        iButtonKeyType type;
        if(!flipper_format_read_string(file, "Key type", data)) break;
        if(!ibutton_key_get_type_by_string(string_get_cstr(data), &type)) break;

        // key data
        uint8_t key_data[IBUTTON_KEY_DATA_SIZE] = {0};
        if(!flipper_format_read_hex(file, "Data", key_data, ibutton_key_get_size_by_type(type)))
            break;

        ibutton_key_set_type(key, type);
        ibutton_key_set_data(key, key_data, IBUTTON_KEY_DATA_SIZE);

        result = true;
    } while(false);

    flipper_format_free(file);
    string_clear(data);

    if(!result) {
        dialog_message_show_storage_error(dialogs, "Cannot load\nkey file");
    }

    return result;
}

bool iButtonApp::load_key(const char* key_name) {
    bool result = false;
    string_t key_path;

    string_init_set_str(key_path, key_name);

    result = load_key_data(key_path);
    if(result) {
        path_extract_filename_no_ext(key_name, key_path);
        ibutton_key_set_name(key, string_get_cstr(key_path));
    }
    string_clear(key_path);
    return result;
}

bool iButtonApp::load_key() {
    bool result = false;

    // Input events and views are managed by file_select
    bool res = dialog_file_select_show(
        dialogs,
        app_folder,
        app_extension,
        get_file_name(),
        get_file_name_size(),
        ibutton_key_get_name_p(key));

    if(res) {
        string_t key_str;

        // Get key file path
        string_init_printf(key_str, "%s/%s%s", app_folder, get_file_name(), app_extension);

        result = load_key_data(key_str);
        if(result) {
            ibutton_key_set_name(key, get_file_name());
        }
        string_clear(key_str);
    }

    return result;
}

bool iButtonApp::delete_key() {
    string_t file_name;
    bool result = false;

    string_init_printf(
        file_name, "%s/%s%s", app_folder, ibutton_key_get_name_p(key), app_extension);
    result = storage_simply_remove(storage, string_get_cstr(file_name));
    string_clear(file_name);

    return result;
}

void iButtonApp::make_app_folder() {
    if(!storage_simply_mkdir(storage, app_folder)) {
        dialog_message_show_storage_error(dialogs, "Cannot create\napp folder");
    }
}
