#include "ibutton-app.h"
#include <stdarg.h>
#include <callback-connector.h>
#include <m-string.h>
#include <file-worker-cpp.h>
#include <lib/toolbox/path.h>

const char* iButtonApp::app_folder = "/any/ibutton";
const char* iButtonApp::app_extension = ".ibtn";

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
    : notification{"notification"} {
    furi_hal_power_insomnia_enter();
    key_worker = new KeyWorker(&ibutton_gpio);
}

iButtonApp::~iButtonApp() {
    for(std::map<Scene, iButtonScene*>::iterator it = scenes.begin(); it != scenes.end(); ++it) {
        delete it->second;
        scenes.erase(it);
    }
    delete key_worker;

    furi_hal_power_insomnia_exit();
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

const GpioPin* iButtonApp::get_ibutton_pin() {
    // TODO open record
    return &ibutton_gpio;
}

KeyWorker* iButtonApp::get_key_worker() {
    return key_worker;
}

iButtonKey* iButtonApp::get_key() {
    return &key;
}

char* iButtonApp::get_file_name() {
    return file_name;
}

uint8_t iButtonApp::get_file_name_size() {
    return file_name_size;
}

void iButtonApp::notify_green_blink() {
    notification_message(notification, &sequence_blink_green_10);
}

void iButtonApp::notify_yellow_blink() {
    notification_message(notification, &sequence_blink_yellow_10);
}

void iButtonApp::notify_red_blink() {
    notification_message(notification, &sequence_blink_red_10);
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

    FileWorkerCpp file_worker;
    string_t key_file_name;
    bool result = false;

    // First remove key if it was saved
    string_init_printf(key_file_name, "%s/%s%s", app_folder, get_key()->get_name(), app_extension);
    if(!file_worker.remove(string_get_cstr(key_file_name))) {
        string_clear(key_file_name);
        return false;
    };

    // Save the key
    get_key()->set_name(key_name);
    string_printf(key_file_name, "%s/%s%s", app_folder, get_key()->get_name(), app_extension);

    bool res = file_worker.open(string_get_cstr(key_file_name), FSAM_WRITE, FSOM_CREATE_ALWAYS);
    string_clear(key_file_name);

    if(res) {
        // type header
        const char* key_type = "E ";

        switch(get_key()->get_key_type()) {
        case iButtonKeyType::KeyCyfral:
            key_type = "C ";
            break;
        case iButtonKeyType::KeyDallas:
            key_type = "D ";
            break;
        case iButtonKeyType::KeyMetakom:
            key_type = "M ";
            break;
        }

        if(!file_worker.write(key_type, 2)) {
            file_worker.close();
            return false;
        }

        if(!file_worker.write_hex(get_key()->get_data(), get_key()->get_type_data_size())) {
            file_worker.close();
            return false;
        }
        result = true;
    }

    file_worker.close();

    return result;
}

bool iButtonApp::load_key_data(string_t key_path) {
    FileWorkerCpp file_worker;

    // Open key file
    if(!file_worker.open(string_get_cstr(key_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
        file_worker.close();
        return false;
    }

    const uint8_t byte_text_size = 4;
    char byte_text[byte_text_size] = {0, 0, 0, 0};

    // Load type header
    if(!file_worker.read(byte_text, 2)) {
        file_worker.close();
        return false;
    }

    iButtonKeyType key_type = iButtonKeyType::KeyCyfral;
    if(strcmp(byte_text, "C ") == 0) {
        key_type = iButtonKeyType::KeyCyfral;
    } else if(strcmp(byte_text, "M ") == 0) {
        key_type = iButtonKeyType::KeyMetakom;
    } else if(strcmp(byte_text, "D ") == 0) {
        key_type = iButtonKeyType::KeyDallas;
    } else {
        file_worker.show_error("Cannot parse\nkey file");
        file_worker.close();
        return false;
    }

    iButtonKeyType old_type = get_key()->get_key_type();
    get_key()->set_type(key_type);

    uint8_t key_data[IBUTTON_KEY_DATA_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0};
    if(!file_worker.read_hex(key_data, get_key()->get_type_data_size())) {
        get_key()->set_type(old_type);
        file_worker.close();
        return false;
    }

    file_worker.close();
    get_key()->set_data(key_data, IBUTTON_KEY_DATA_SIZE);

    return true;
}

bool iButtonApp::load_key(const char* key_name) {
    bool result = false;
    string_t key_path;

    string_init_set_str(key_path, key_name);

    result = load_key_data(key_path);
    if(result) {
        path_extract_filename_no_ext(key_name, key_path);
        get_key()->set_name(string_get_cstr(key_path));
    }
    string_clear(key_path);
    return result;
}

bool iButtonApp::load_key() {
    bool result = false;
    FileWorkerCpp file_worker;

    // Input events and views are managed by file_select
    bool res = file_worker.file_select(
        app_folder, app_extension, get_file_name(), get_file_name_size(), get_key()->get_name());

    if(res) {
        string_t key_str;

        // Get key file path
        string_init_printf(key_str, "%s/%s%s", app_folder, get_file_name(), app_extension);

        result = load_key_data(key_str);
        if(result) {
            get_key()->set_name(get_file_name());
        }
        string_clear(key_str);
    }

    return result;
}

bool iButtonApp::delete_key() {
    string_t file_name;
    bool result = false;
    FileWorkerCpp file_worker;

    string_init_printf(file_name, "%s/%s%s", app_folder, get_key()->get_name(), app_extension);
    result = file_worker.remove(string_get_cstr(file_name));
    string_clear(file_name);

    return result;
}

void iButtonApp::make_app_folder() {
    FileWorkerCpp file_worker;
    file_worker.mkdir(app_folder);
}