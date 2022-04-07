#include "../infrared_app.h"

void InfraredAppSceneEditRename::on_enter(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    TextInput* text_input = view_manager->get_text_input();
    size_t enter_name_length = 0;

    auto remote_manager = app->get_remote_manager();
    if(app->get_edit_element() == InfraredApp::EditElement::Button) {
        furi_assert(app->get_current_button() != InfraredApp::ButtonNA);
        auto button_name = remote_manager->get_button_name(app->get_current_button());
        char* buffer_str = app->get_text_store(0);
        size_t max_len = InfraredAppRemoteManager::max_button_name_length;
        strncpy(buffer_str, button_name.c_str(), max_len);
        buffer_str[max_len + 1] = 0;
        enter_name_length = max_len;
        text_input_set_header_text(text_input, "Name the key");
    } else {
        auto remote_name = remote_manager->get_remote_name();
        strncpy(app->get_text_store(0), remote_name.c_str(), app->get_text_store_size());
        enter_name_length = InfraredAppRemoteManager::max_remote_name_length;
        text_input_set_header_text(text_input, "Name the remote");

        ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
            app->infrared_directory, app->infrared_extension, remote_name.c_str());
        text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);
    }

    text_input_set_result_callback(
        text_input,
        InfraredApp::text_input_callback,
        app,
        app->get_text_store(0),
        enter_name_length,
        false);

    view_manager->switch_to(InfraredAppViewManager::ViewId::TextInput);
}

bool InfraredAppSceneEditRename::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    if(event->type == InfraredAppEvent::Type::TextEditDone) {
        auto remote_manager = app->get_remote_manager();
        bool result = false;
        if(app->get_edit_element() == InfraredApp::EditElement::Button) {
            result =
                remote_manager->rename_button(app->get_current_button(), app->get_text_store(0));
            app->set_current_button(InfraredApp::ButtonNA);
        } else {
            result = remote_manager->rename_remote(app->get_text_store(0));
        }
        if(!result) {
            app->search_and_switch_to_previous_scene(
                {InfraredApp::Scene::Start, InfraredApp::Scene::RemoteList});
        } else {
            app->switch_to_next_scene_without_saving(InfraredApp::Scene::EditRenameDone);
        }
        consumed = true;
    }

    return consumed;
}

void InfraredAppSceneEditRename::on_exit(InfraredApp* app) {
    TextInput* text_input = app->get_view_manager()->get_text_input();

    void* validator_context = text_input_get_validator_callback_context(text_input);
    text_input_set_validator(text_input, NULL, NULL);

    if(validator_context != NULL) validator_is_file_free((ValidatorIsFile*)validator_context);
}
