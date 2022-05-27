#include "lfrfid_app_scene_save_name.h"
#include "m-string.h"
#include <lib/toolbox/random_name.h>
#include <lib/toolbox/path.h>

void LfRfidAppSceneSaveName::on_enter(LfRfidApp* app, bool /* need_restore */) {
    const char* key_name = app->worker.key.get_name();

    bool key_name_empty = !strcmp(key_name, "");
    if(key_name_empty) {
        string_set_str(app->file_path, app->app_folder);
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

    string_t folder_path;
    string_init(folder_path);

    path_extract_dirname(string_get_cstr(app->file_path), folder_path);

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(string_get_cstr(folder_path), app->app_extension, key_name);
    text_input->set_validator(validator_is_file_callback, validator_is_file);

    string_clear(folder_path);

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
                {LfRfidApp::SceneType::ReadKeyMenu});
        }
    }

    return consumed;
}

void LfRfidAppSceneSaveName::on_exit(LfRfidApp* app) {
    void* validator_context =
        app->view_controller.get<TextInputVM>()->get_validator_callback_context();
    app->view_controller.get<TextInputVM>()->set_validator(NULL, NULL);
    validator_is_file_free((ValidatorIsFile*)validator_context);

    app->view_controller.get<TextInputVM>()->clean();
}

void LfRfidAppSceneSaveName::save_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Next;
    app->view_controller.send_event(&event);
}
