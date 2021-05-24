#include "lf-rfid-app.h"
#include <furi.h>
#include <api-hal.h>
#include <stdarg.h>

void LfrfidApp::run(void) {
    LfrfidEvent event;
    bool consumed;
    bool exit = false;

    scenes[current_scene]->on_enter(this);

    while(!exit) {
        view.receive_event(&event);

        consumed = scenes[current_scene]->on_event(this, &event);

        if(!consumed) {
            if(event.type == LfrfidEvent::Type::Back) {
                exit = switch_to_previous_scene();
            }
        }
    };

    scenes[current_scene]->on_exit(this);
}

LfrfidApp::LfrfidApp() {
    api_hal_power_insomnia_enter();
    notification = static_cast<NotificationApp*>(furi_record_open("notification"));
}

LfrfidApp::~LfrfidApp() {
    for(std::map<Scene, LfrfidScene*>::iterator it = scenes.begin(); it != scenes.end(); ++it) {
        delete it->second;
        scenes.erase(it);
    }

    furi_record_close("notification");

    api_hal_power_insomnia_exit();
}

LfrfidAppViewManager* LfrfidApp::get_view_manager() {
    return &view;
}

void LfrfidApp::switch_to_next_scene(Scene next_scene) {
    previous_scenes_list.push_front(current_scene);

    if(next_scene != Scene::Exit) {
        scenes[current_scene]->on_exit(this);
        current_scene = next_scene;
        scenes[current_scene]->on_enter(this);
    }
}

void LfrfidApp::search_and_switch_to_previous_scene(std::initializer_list<Scene> scenes_list) {
    Scene previous_scene = Scene::Start;
    bool scene_found = false;

    while(!scene_found) {
        previous_scene = get_previous_scene();
        for(Scene element : scenes_list) {
            if(previous_scene == element || previous_scene == Scene::Start) {
                scene_found = true;
                break;
            }
        }
    }

    scenes[current_scene]->on_exit(this);
    current_scene = previous_scene;
    scenes[current_scene]->on_enter(this);
}

bool LfrfidApp::switch_to_previous_scene(uint8_t count) {
    Scene previous_scene = Scene::Start;

    for(uint8_t i = 0; i < count; i++) {
        previous_scene = get_previous_scene();
        if(previous_scene == Scene::Exit) break;
    }

    if(previous_scene == Scene::Exit) {
        return true;
    } else {
        scenes[current_scene]->on_exit(this);
        current_scene = previous_scene;
        scenes[current_scene]->on_enter(this);
        return false;
    }
}

LfrfidApp::Scene LfrfidApp::get_previous_scene() {
    Scene scene = previous_scenes_list.front();
    previous_scenes_list.pop_front();
    return scene;
}

/***************************** NOTIFY *******************************/

void LfrfidApp::notify_green_blink() {
    notification_message(notification, &sequence_blink_green_10);
}

void LfrfidApp::notify_success() {
    notification_message(notification, &sequence_success);
}

/*************************** TEXT STORE *****************************/

char* LfrfidApp::get_text_store() {
    return text_store;
}

uint8_t LfrfidApp::get_text_store_size() {
    return text_store_size;
}

void LfrfidApp::set_text_store(const char* text...) {
    va_list args;
    va_start(args, text);

    vsnprintf(text_store, text_store_size, text, args);

    va_end(args);
}

RfidReader* LfrfidApp::get_reader() {
    return &reader;
}

RfidTimerEmulator* LfrfidApp::get_emulator() {
    return &emulator;
}

RfidWriter* LfrfidApp::get_writer() {
    return &writer;
}