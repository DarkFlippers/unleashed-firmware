#include "irda-app.h"
#include "irda/irda-app-file-parser.h"
#include <irda_worker.h>
#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdio.h>
#include <callback-connector.h>

int32_t IrdaApp::run(void* args) {
    IrdaAppEvent event;
    bool consumed;
    bool exit = false;

    if(args) {
        std::string remote_name;
        {
            IrdaAppFileParser file_parser;
            remote_name = file_parser.make_name(static_cast<const char*>(args));
        }
        bool result = remote_manager.load(remote_name);
        if(result) {
            current_scene = IrdaApp::Scene::Remote;
        } else {
            printf("Failed to load remote \'%s\'\r\n", remote_name.c_str());
            return -1;
        }
    }

    scenes[current_scene]->on_enter(this);

    while(!exit) {
        view_manager.receive_event(&event);

        if(event.type == IrdaAppEvent::Type::Exit) break;

        consumed = scenes[current_scene]->on_event(this, &event);

        if(!consumed) {
            if(event.type == IrdaAppEvent::Type::Back) {
                exit = switch_to_previous_scene();
            }
        }
    };

    scenes[current_scene]->on_exit(this);

    return 0;
};

IrdaApp::IrdaApp() {
    furi_check(IrdaAppRemoteManager::max_button_name_length < get_text_store_size());
    notification = static_cast<NotificationApp*>(furi_record_open("notification"));
    irda_worker = irda_worker_alloc();
}

IrdaApp::~IrdaApp() {
    irda_worker_free(irda_worker);
    furi_record_close("notification");
    for(auto& [key, scene] : scenes) delete scene;
}

IrdaAppViewManager* IrdaApp::get_view_manager() {
    return &view_manager;
}

void IrdaApp::set_learn_new_remote(bool value) {
    learn_new_remote = value;
}

bool IrdaApp::get_learn_new_remote() {
    return learn_new_remote;
}

void IrdaApp::switch_to_next_scene(Scene next_scene) {
    previous_scenes_list.push_front(current_scene);
    switch_to_next_scene_without_saving(next_scene);
}

void IrdaApp::switch_to_next_scene_without_saving(Scene next_scene) {
    if(next_scene != Scene::Exit) {
        scenes[current_scene]->on_exit(this);
        current_scene = next_scene;
        scenes[current_scene]->on_enter(this);
        view_manager.clear_events();
    }
}

void IrdaApp::search_and_switch_to_previous_scene(const std::initializer_list<Scene>& scenes_list) {
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
        IrdaAppEvent event;
        event.type = IrdaAppEvent::Type::Exit;
        view_manager.send_event(&event);
    } else {
        scenes[current_scene]->on_exit(this);
        current_scene = previous_scene;
        scenes[current_scene]->on_enter(this);
        view_manager.clear_events();
    }
}

bool IrdaApp::switch_to_previous_scene(uint8_t count) {
    Scene previous_scene = Scene::Start;

    for(uint8_t i = 0; i < count; i++) previous_scene = get_previous_scene();

    if(previous_scene == Scene::Exit) return true;

    scenes[current_scene]->on_exit(this);
    current_scene = previous_scene;
    scenes[current_scene]->on_enter(this);
    view_manager.clear_events();
    return false;
}

IrdaApp::Scene IrdaApp::get_previous_scene() {
    Scene scene = Scene::Exit;

    if(!previous_scenes_list.empty()) {
        scene = previous_scenes_list.front();
        previous_scenes_list.pop_front();
    }

    return scene;
}

IrdaAppRemoteManager* IrdaApp::get_remote_manager() {
    return &remote_manager;
}

void IrdaApp::set_text_store(uint8_t index, const char* text...) {
    furi_check(index < text_store_max);

    va_list args;
    va_start(args, text);

    vsnprintf(text_store[index], text_store_size, text, args);

    va_end(args);
}

char* IrdaApp::get_text_store(uint8_t index) {
    furi_check(index < text_store_max);

    return text_store[index];
}

uint8_t IrdaApp::get_text_store_size() {
    return text_store_size;
}

void IrdaApp::text_input_callback(void* context) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;
    event.type = IrdaAppEvent::Type::TextEditDone;
    app->get_view_manager()->send_event(&event);
}

void IrdaApp::popup_callback(void* context) {
    IrdaApp* app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;
    event.type = IrdaAppEvent::Type::PopupTimer;
    app->get_view_manager()->send_event(&event);
}

void IrdaApp::set_edit_element(IrdaApp::EditElement value) {
    element = value;
}

IrdaApp::EditElement IrdaApp::get_edit_element(void) {
    return element;
}

void IrdaApp::set_edit_action(IrdaApp::EditAction value) {
    action = value;
}

IrdaApp::EditAction IrdaApp::get_edit_action(void) {
    return action;
}

void IrdaApp::set_current_button(int value) {
    current_button = value;
}

int IrdaApp::get_current_button() {
    return current_button;
}

void IrdaApp::notify_success() {
    notification_message(notification, &sequence_success);
}

void IrdaApp::notify_red_blink() {
    notification_message(notification, &sequence_blink_red_10);
}

void IrdaApp::notify_sent_just_learnt() {
    static const NotificationSequence sequence = {
        &message_green_0,
        &message_vibro_on,
        &message_delay_50,
        &message_vibro_off,
        &message_green_255,
        &message_do_not_reset,
        NULL,
    };

    notification_message_block(notification, &sequence);
}

void IrdaApp::notify_click() {
    static const NotificationSequence sequence = {
        &message_click,
        &message_delay_1,
        &message_sound_off,
        NULL,
    };

    notification_message_block(notification, &sequence);
}

void IrdaApp::notify_click_and_green_blink() {
    static const NotificationSequence sequence = {
        &message_click,
        &message_delay_1,
        &message_sound_off,
        &message_green_255,
        &message_delay_10,
        &message_green_0,
        &message_do_not_reset,
        NULL,
    };

    notification_message_block(notification, &sequence);
}

void IrdaApp::notify_blink_green() {
    static const NotificationSequence sequence = {
        &message_green_255,
        &message_delay_10,
        &message_green_0,
        &message_do_not_reset,
        NULL,
    };

    notification_message(notification, &sequence);
}

void IrdaApp::notify_green_on() {
    notification_message(notification, &sequence_set_only_green_255);
}

void IrdaApp::notify_green_off() {
    notification_message(notification, &sequence_reset_green);
}

IrdaWorker* IrdaApp::get_irda_worker() {
    return irda_worker;
}

const IrdaAppSignal& IrdaApp::get_received_signal() const {
    return received_signal;
}

void IrdaApp::set_received_signal(const IrdaAppSignal& signal) {
    received_signal = signal;
}
