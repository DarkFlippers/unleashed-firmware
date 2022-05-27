#include "infrared_app.h"
#include "m-string.h"
#include <infrared_worker.h>
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdio.h>
#include <callback-connector.h>

int32_t InfraredApp::run(void* args) {
    InfraredAppEvent event;
    bool consumed;
    bool exit = false;

    if(args) {
        string_t path;
        string_init_set_str(path, (char*)args);
        if(string_end_with_str_p(path, InfraredApp::infrared_extension)) {
            bool result = remote_manager.load(path);
            if(result) {
                current_scene = InfraredApp::Scene::Remote;
            } else {
                printf("Failed to load remote \'%s\'\r\n", string_get_cstr(path));
                return -1;
            }
        }
        string_clear(path);
    }

    scenes[current_scene]->on_enter(this);

    while(!exit) {
        view_manager.receive_event(&event);

        if(event.type == InfraredAppEvent::Type::Exit) break;

        consumed = scenes[current_scene]->on_event(this, &event);

        if(!consumed) {
            if(event.type == InfraredAppEvent::Type::Back) {
                exit = switch_to_previous_scene();
            }
        }
    };

    scenes[current_scene]->on_exit(this);

    return 0;
};

InfraredApp::InfraredApp() {
    furi_check(InfraredAppRemoteManager::max_button_name_length < get_text_store_size());
    string_init_set_str(file_path, InfraredApp::infrared_directory);
    notification = static_cast<NotificationApp*>(furi_record_open("notification"));
    dialogs = static_cast<DialogsApp*>(furi_record_open("dialogs"));
    infrared_worker = infrared_worker_alloc();
}

InfraredApp::~InfraredApp() {
    infrared_worker_free(infrared_worker);
    furi_record_close("notification");
    furi_record_close("dialogs");
    string_clear(file_path);
    for(auto& [key, scene] : scenes) delete scene;
}

InfraredAppViewManager* InfraredApp::get_view_manager() {
    return &view_manager;
}

void InfraredApp::set_learn_new_remote(bool value) {
    learn_new_remote = value;
}

bool InfraredApp::get_learn_new_remote() {
    return learn_new_remote;
}

void InfraredApp::switch_to_next_scene(Scene next_scene) {
    previous_scenes_list.push_front(current_scene);
    switch_to_next_scene_without_saving(next_scene);
}

void InfraredApp::switch_to_next_scene_without_saving(Scene next_scene) {
    if(next_scene != Scene::Exit) {
        scenes[current_scene]->on_exit(this);
        current_scene = next_scene;
        scenes[current_scene]->on_enter(this);
        view_manager.clear_events();
    }
}

void InfraredApp::search_and_switch_to_previous_scene(
    const std::initializer_list<Scene>& scenes_list) {
    Scene previous_scene = Scene::Start;
    bool scene_found = false;

    while(!scene_found) {
        previous_scene = get_previous_scene();

        if(previous_scene == Scene::Exit) break;

        for(Scene element : scenes_list) {
            if(previous_scene == element) {
                scene_found = true;
                break;
            }
        }
    }

    if(previous_scene == Scene::Exit) {
        InfraredAppEvent event;
        event.type = InfraredAppEvent::Type::Exit;
        view_manager.send_event(&event);
    } else {
        scenes[current_scene]->on_exit(this);
        current_scene = previous_scene;
        scenes[current_scene]->on_enter(this);
        view_manager.clear_events();
    }
}

bool InfraredApp::switch_to_previous_scene(uint8_t count) {
    Scene previous_scene = Scene::Start;

    for(uint8_t i = 0; i < count; i++) previous_scene = get_previous_scene();

    if(previous_scene == Scene::Exit) return true;

    scenes[current_scene]->on_exit(this);
    current_scene = previous_scene;
    scenes[current_scene]->on_enter(this);
    view_manager.clear_events();
    return false;
}

InfraredApp::Scene InfraredApp::get_previous_scene() {
    Scene scene = Scene::Exit;

    if(!previous_scenes_list.empty()) {
        scene = previous_scenes_list.front();
        previous_scenes_list.pop_front();
    }

    return scene;
}

InfraredAppRemoteManager* InfraredApp::get_remote_manager() {
    return &remote_manager;
}

void InfraredApp::set_text_store(uint8_t index, const char* text...) {
    furi_check(index < text_store_max);

    va_list args;
    va_start(args, text);

    vsnprintf(text_store[index], text_store_size, text, args);

    va_end(args);
}

char* InfraredApp::get_text_store(uint8_t index) {
    furi_check(index < text_store_max);

    return text_store[index];
}

uint8_t InfraredApp::get_text_store_size() {
    return text_store_size;
}

void InfraredApp::text_input_callback(void* context) {
    InfraredApp* app = static_cast<InfraredApp*>(context);
    InfraredAppEvent event;
    event.type = InfraredAppEvent::Type::TextEditDone;
    app->get_view_manager()->send_event(&event);
}

void InfraredApp::popup_callback(void* context) {
    InfraredApp* app = static_cast<InfraredApp*>(context);
    InfraredAppEvent event;
    event.type = InfraredAppEvent::Type::PopupTimer;
    app->get_view_manager()->send_event(&event);
}

void InfraredApp::set_edit_element(InfraredApp::EditElement value) {
    element = value;
}

InfraredApp::EditElement InfraredApp::get_edit_element(void) {
    return element;
}

void InfraredApp::set_edit_action(InfraredApp::EditAction value) {
    action = value;
}

InfraredApp::EditAction InfraredApp::get_edit_action(void) {
    return action;
}

void InfraredApp::set_current_button(int value) {
    current_button = value;
}

int InfraredApp::get_current_button() {
    return current_button;
}

void InfraredApp::notify_success() {
    notification_message(notification, &sequence_success);
}

void InfraredApp::notify_blink_read() {
    notification_message(notification, &sequence_blink_cyan_10);
}

void InfraredApp::notify_blink_send() {
    notification_message(notification, &sequence_blink_magenta_10);
}

DialogsApp* InfraredApp::get_dialogs() {
    return dialogs;
}

void InfraredApp::notify_green_on() {
    notification_message(notification, &sequence_set_only_green_255);
}

void InfraredApp::notify_green_off() {
    notification_message(notification, &sequence_reset_green);
}

InfraredWorker* InfraredApp::get_infrared_worker() {
    return infrared_worker;
}

const InfraredAppSignal& InfraredApp::get_received_signal() const {
    return received_signal;
}

void InfraredApp::set_received_signal(const InfraredAppSignal& signal) {
    received_signal = signal;
}

void InfraredApp::signal_sent_callback(void* context) {
    InfraredApp* app = static_cast<InfraredApp*>(context);
    app->notify_blink_send();
}
