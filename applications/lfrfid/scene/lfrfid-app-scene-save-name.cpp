#include "lfrfid-app-scene-save-name.h"
#include <lib/toolbox/random_name.h>

void LfRfidAppSceneSaveName::on_enter(LfRfidApp* app, bool need_restore) {
    const char* key_name = app->worker.key.get_name();

    bool key_name_empty = !strcmp(key_name, "");
    if(key_name_empty) {
        set_random_name(app->text_store.text, app->text_store.text_size);
    } else {
        app->text_store.set("%s", key_name);
    }

    auto text_input = app->view_controller.get<TextInputVM>();
    text_input->set_header_text("Name the card");

    text_input->set_result_callback(
        save_callback,
        app,
        app->text_store.text,
        app->worker.key.get_name_length(),
        key_name_empty);

    app->view_controller.switch_to<TextInputVM>();
}

bool LfRfidAppSceneSaveName::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Next) {
        if(strlen(app->worker.key.get_name())) {
            app->delete_key(&app->worker.key);
        }

        app->worker.key.set_name(app->text_store.text);

        if(app->save_key(&app->worker.key)) {
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::SaveSuccess);
        } else {
            app->scene_controller.search_and_switch_to_previous_scene(
                {LfRfidApp::SceneType::ReadedMenu});
        }
    }

    return consumed;
}

void LfRfidAppSceneSaveName::on_exit(LfRfidApp* app) {
    app->view_controller.get<TextInputVM>()->clean();
}

void LfRfidAppSceneSaveName::save_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Next;
    app->view_controller.send_event(&event);
}
