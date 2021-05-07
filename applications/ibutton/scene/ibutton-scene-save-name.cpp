#include "ibutton-scene-save-name.h"
#include "../ibutton-app.h"
#include "../ibutton-view-manager.h"
#include "../ibutton-event.h"
#include "../ibutton-key.h"
#include <callback-connector.h>
#include <filesystem-api.h>

void iButtonSceneSaveName::on_enter(iButtonApp* app) {
    iButtonAppViewManager* view_manager = app->get_view_manager();
    TextInput* text_input = view_manager->get_text_input();
    auto callback = cbc::obtain_connector(this, &iButtonSceneSaveName::text_input_callback);

    iButtonKey* key = app->get_key();
    const char* key_name = key->get_name();

    if(strcmp(key_name, "") == 0) {
        app->generate_random_name(app->get_text_store(), app->get_text_store_size());
    } else {
        app->set_text_store("%s", key_name);
    }

    text_input_set_header_text(text_input, "Name the key");
    text_input_set_result_callback(
        text_input, callback, app, app->get_text_store(), app->get_text_store_size());

    view_manager->switch_to(iButtonAppViewManager::Type::iButtonAppViewTextInput);
}

bool iButtonSceneSaveName::on_event(iButtonApp* app, iButtonEvent* event) {
    bool consumed = false;

    if(event->type == iButtonEvent::Type::EventTypeTextEditResult) {
        iButtonKey* key = app->get_key();
        File key_file;
        string_t key_file_name;
        string_init_set_str(key_file_name, "ibutton/");
        string_cat_str(key_file_name, app->get_text_store());
        uint8_t key_data[IBUTTON_KEY_SIZE + 1];
        key_data[0] = static_cast<uint8_t>(key->get_key_type());
        memcpy(key_data + 1, key->get_data(), IBUTTON_KEY_SIZE);
        // Create ibutton directory if necessary
        app->get_fs_api()->common.mkdir("ibutton");
        bool res = app->get_fs_api()->file.open(
            &key_file, string_get_cstr(key_file_name), FSAM_WRITE, FSOM_CREATE_ALWAYS);
        // TODO process file system errors from file system service
        if(res) {
            res = app->get_fs_api()->file.write(&key_file, key_data, IBUTTON_KEY_SIZE + 1);
            res = app->get_fs_api()->file.close(&key_file);
            app->switch_to_next_scene(iButtonApp::Scene::SceneSaveSuccess);
        } else {
            app->get_sd_ex_api()->check_error(app->get_sd_ex_api()->context);
            app->search_and_switch_to_previous_scene(
                {iButtonApp::Scene::SceneReadedKeyMenu,
                 iButtonApp::Scene::SceneSavedKeyMenu,
                 iButtonApp::Scene::SceneAddType});
        }
        string_clear(key_file_name);
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