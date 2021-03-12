#include "ibutton-scene-save-name.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include "../ibutton-key.h"
#include <callback-connector.h>

void iButtonSceneSaveName::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    TextInput* text_input = view_manager->get_text_input();
    auto callback = cbc::obtain_connector(this, &iButtonSceneSaveName::text_input_callback);

    iButtonKey* key = app->get_key();
    const char* key_name = key->get_name();

    if(strcmp(key_name, "") == 0) {
        app->generate_random_name(app->get_text_store(), app->get_text_store_size());
    }

    text_input_set_header_text(text_input, "Name the key");
    text_input_set_result_callback(
        text_input, callback, app, app->get_text_store(), app->get_text_store_size());

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewTextInput);
}

bool iButtonSceneSaveName::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeTextEditResult) {
        KeyStore* store = app->get_key_store();
        uint8_t key_index = store->add_key();
        iButtonKey* key = app->get_key();

        store->set_key_type(key_index, key->get_key_type());
        store->set_key_name(key_index, app->get_text_store());
        store->set_key_data(key_index, key->get_data(), key->get_size());

        app->switch_to_next_scene(iButtonApp::Scene::SceneSaveSuccess);
        consumed = true;
    }

    return consumed;
}

void iButtonSceneSaveName::on_exit(iButtonApp* app) {
    TextInput* text_input = app->get_view_manager()->get_text_input();
    text_input_set_header_text(text_input, "");
    text_input_set_result_callback(text_input, NULL, NULL, NULL, 0);
}

void iButtonSceneSaveName::text_input_callback(void* context, char* text) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeTextEditResult;

    app->get_view_manager()->send_event(&event);
}