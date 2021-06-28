#include "lfrfid-app-scene-save-name.h"
#include "../helpers/rfid-name-generator.h"

void LfRfidAppSceneSaveName::on_enter(LfRfidApp* app, bool need_restore) {
    const char* key_name = app->worker.key.get_name();

    if(strcmp(key_name, "") == 0) {
        rfid_generate_random_name(app->text_store.text, app->text_store.text_size);
    } else {
        app->text_store.set("%s", key_name);
    }

    auto text_input = app->view_controller.get<TextInputVM>();
    text_input->set_header_text("Name the card");

    text_input->set_result_callback(
        save_callback, app, app->text_store.text, LFRFID_KEY_NAME_SIZE);

    app->view_controller.switch_to<TextInputVM>();
}

bool LfRfidAppSceneSaveName::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Next) {
        /*if(app->save_key(app->get_text_store())) {
            app->switch_to_next_scene(iButtonApp::Scene::SceneSaveSuccess);
        } else {
            app->search_and_switch_to_previous_scene(
                {iButtonApp::Scene::SceneReadedKeyMenu,
                 iButtonApp::Scene::SceneSavedKeyMenu,
                 iButtonApp::Scene::SceneAddType});
        }*/
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
