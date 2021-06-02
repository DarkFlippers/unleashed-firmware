#include "../irda-app.hpp"
#include <cstdio>

void IrdaAppSceneEditRename::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    TextInput* text_input = view_manager->get_text_input();

    auto remote_manager = app->get_remote_manager();
    if(app->get_edit_element() == IrdaApp::EditElement::Button) {
        auto button_name = remote_manager->get_current_button_name();
        strncpy(app->get_text_store(0), button_name.c_str(), app->get_text_store_size());
    } else {
        auto remote_name = remote_manager->get_current_remote_name();
        strncpy(app->get_text_store(0), remote_name.c_str(), app->get_text_store_size());
    }

    text_input_set_header_text(text_input, "Name the key");
    text_input_set_result_callback(
        text_input,
        IrdaApp::text_input_callback,
        app,
        app->get_text_store(0),
        app->get_text_store_size());

    view_manager->switch_to(IrdaAppViewManager::ViewType::TextInput);
}

bool IrdaAppSceneEditRename::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::TextEditDone) {
        auto remote_manager = app->get_remote_manager();
        if(app->get_edit_element() == IrdaApp::EditElement::Button) {
            remote_manager->rename_button(app->get_text_store(0));
        } else {
            remote_manager->rename_remote(app->get_text_store(0));
        }
        app->switch_to_next_scene_without_saving(IrdaApp::Scene::EditRenameDone);
        consumed = true;
    }

    return consumed;
}

void IrdaAppSceneEditRename::on_exit(IrdaApp* app) {
}
