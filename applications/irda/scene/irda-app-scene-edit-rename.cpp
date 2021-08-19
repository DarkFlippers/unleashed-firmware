#include "../irda-app.h"

void IrdaAppSceneEditRename::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    TextInput* text_input = view_manager->get_text_input();
    size_t enter_name_length = 0;

    auto remote_manager = app->get_remote_manager();
    if(app->get_edit_element() == IrdaApp::EditElement::Button) {
        furi_assert(app->get_current_button() != IrdaApp::ButtonNA);
        auto button_name = remote_manager->get_button_name(app->get_current_button());
        char* buffer_str = app->get_text_store(0);
        size_t max_len = IrdaAppRemoteManager::max_button_name_length;
        strncpy(buffer_str, button_name.c_str(), max_len);
        buffer_str[max_len + 1] = 0;
        enter_name_length = max_len;
        text_input_set_header_text(text_input, "Name the key");
    } else {
        auto remote_name = remote_manager->get_remote_name();
        strncpy(app->get_text_store(0), remote_name.c_str(), app->get_text_store_size());
        enter_name_length = IrdaAppRemoteManager::max_remote_name_length;
        text_input_set_header_text(text_input, "Name the remote");
    }

    text_input_set_result_callback(
        text_input,
        IrdaApp::text_input_callback,
        app,
        app->get_text_store(0),
        enter_name_length,
        false);

    view_manager->switch_to(IrdaAppViewManager::ViewType::TextInput);
}

bool IrdaAppSceneEditRename::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::TextEditDone) {
        auto remote_manager = app->get_remote_manager();
        bool result = false;
        if(app->get_edit_element() == IrdaApp::EditElement::Button) {
            result =
                remote_manager->rename_button(app->get_current_button(), app->get_text_store(0));
            app->set_current_button(IrdaApp::ButtonNA);
        } else {
            result = remote_manager->rename_remote(app->get_text_store(0));
        }
        if(!result) {
            app->search_and_switch_to_previous_scene(
                {IrdaApp::Scene::Start, IrdaApp::Scene::RemoteList});
        } else {
            app->switch_to_next_scene_without_saving(IrdaApp::Scene::EditRenameDone);
        }
        consumed = true;
    }

    return consumed;
}

void IrdaAppSceneEditRename::on_exit(IrdaApp* app) {
}
