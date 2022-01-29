#include "ibutton_scene_save_name.h"
#include "../ibutton_app.h"
#include "../ibutton_view_manager.h"
#include "../ibutton_event.h"
#include "../ibutton_key.h"
#include <callback-connector.h>
#include <lib/toolbox/random_name.h>

void iButtonSceneSaveName::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    TextInput* text_input = view_manager->get_text_input();
    auto callback = cbc::obtain_connector(this, &iButtonSceneSaveName::text_input_callback);

    iButtonKey* key = app->get_key();
    const char* key_name = key->get_name();
    bool key_name_empty = !strcmp(key_name, "");

    if(key_name_empty) {
        set_random_name(app->get_text_store(), app->get_text_store_size());
    } else {
        app->set_text_store("%s", key_name);
    }

    text_input_set_header_text(text_input, "Name the key");
    text_input_set_result_callback(
        text_input, callback, app, app->get_text_store(), IBUTTON_KEY_NAME_SIZE, key_name_empty);

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(app->app_folder, app->app_extension);
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewTextInput);
}

bool iButtonSceneSaveName::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeTextEditResult) {
        if(app->save_key(app->get_text_store())) {
            app->switch_to_next_scene(iButtonApp::Scene::SceneSaveSuccess);
        } else {
            app->search_and_switch_to_previous_scene(
                {iButtonApp::Scene::SceneReadedKeyMenu,
                 iButtonApp::Scene::SceneSavedKeyMenu,
                 iButtonApp::Scene::SceneAddType});
        }
        consumed = true;
    }

    return consumed;
}

void iButtonSceneSaveName::on_exit(iButtonApp* app) {
    TextInput* text_input = app->get_view_manager()->get_text_input();

    void* validator_context = text_input_get_validator_callback_context(text_input);
    text_input_set_validator(text_input, NULL, NULL);
    validator_is_file_free((ValidatorIsFile*)validator_context);

    text_input_reset(text_input);
}

void iButtonSceneSaveName::text_input_callback(void* context) {
    iButtonApp* app = static_cast<iButtonApp*>(context);
    iButtonEvent event;

    event.type = iButtonEvent::Type::EventTypeTextEditResult;

    app->get_view_manager()->send_event(&event);
}
