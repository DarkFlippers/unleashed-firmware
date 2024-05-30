#include "accessor_app.h"
#include <furi.h>
#include <furi_hal.h>
#include <stdarg.h>

void AccessorApp::run(void) {
    AccessorEvent event;
    bool consumed;
    bool exit = false;

    wiegand.begin();
    onewire_host_start(onewire_host);

    scenes[current_scene]->on_enter(this);

    while(!exit) {
        view.receive_event(&event);

        consumed = scenes[current_scene]->on_event(this, &event);

        if(!consumed) {
            if(event.type == AccessorEvent::Type::Back) {
                exit = switch_to_previous_scene();
            }
        }
    };

    scenes[current_scene]->on_exit(this);

    wiegand.end();
    onewire_host_stop(onewire_host);
}

AccessorApp::AccessorApp()
    : text_store{0} {
    notification = static_cast<NotificationApp*>(furi_record_open(RECORD_NOTIFICATION));
    expansion = static_cast<Expansion*>(furi_record_open(RECORD_EXPANSION));
    onewire_host = onewire_host_alloc(&gpio_ibutton);
    expansion_disable(expansion);
    furi_hal_power_enable_otg();
}

AccessorApp::~AccessorApp() {
    furi_hal_power_disable_otg();
    expansion_enable(expansion);
    furi_record_close(RECORD_EXPANSION);
    furi_record_close(RECORD_NOTIFICATION);
    onewire_host_free(onewire_host);
}

AccessorAppViewManager* AccessorApp::get_view_manager() {
    return &view;
}

void AccessorApp::switch_to_next_scene(Scene next_scene) {
    previous_scenes_list.push_front(current_scene);

    if(next_scene != Scene::Exit) {
        scenes[current_scene]->on_exit(this);
        current_scene = next_scene;
        scenes[current_scene]->on_enter(this);
    }
}

void AccessorApp::search_and_switch_to_previous_scene(std::initializer_list<Scene> scenes_list) {
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

bool AccessorApp::switch_to_previous_scene(uint8_t count) {
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

AccessorApp::Scene AccessorApp::get_previous_scene() {
    Scene scene = previous_scenes_list.front();
    previous_scenes_list.pop_front();
    return scene;
}

/***************************** NOTIFY *******************************/

void AccessorApp::notify_green_blink() {
    notification_message(notification, &sequence_blink_green_10);
}

void AccessorApp::notify_success() {
    notification_message(notification, &sequence_success);
}

/*************************** TEXT STORE *****************************/

char* AccessorApp::get_text_store() {
    return text_store;
}

uint8_t AccessorApp::get_text_store_size() {
    return text_store_size;
}

void AccessorApp::set_text_store(const char* text...) {
    va_list args;
    va_start(args, text);

    vsnprintf(text_store, text_store_size, text, args);

    va_end(args);
}

/*************************** APP RESOURCES *****************************/

WIEGAND* AccessorApp::get_wiegand() {
    return &wiegand;
}

OneWireHost* AccessorApp::get_one_wire() {
    return onewire_host;
}
