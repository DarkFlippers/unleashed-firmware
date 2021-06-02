#include "../irda-app.hpp"
#include "gui/modules/text_input.h"
#include <callback-connector.h>
#include <string>
#include <stdio.h>

void IrdaAppSceneLearnEnterName::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    TextInput* text_input = view_manager->get_text_input();

    auto receiver = app->get_receiver();
    auto message = receiver->get_last_message();

    app->set_text_store(
        0,
        "%.4s_%0*lX",
        irda_get_protocol_name(message->protocol),
        irda_get_protocol_command_length(message->protocol),
        message->command);

    text_input_set_header_text(text_input, "Name the key");
    text_input_set_result_callback(
        text_input,
        IrdaApp::text_input_callback,
        app,
        app->get_text_store(0),
        app->get_text_store_size());

    view_manager->switch_to(IrdaAppViewManager::ViewType::TextInput);
}

bool IrdaAppSceneLearnEnterName::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::TextEditDone) {
        auto remote_manager = app->get_remote_manager();
        auto receiver = app->get_receiver();
        if(app->get_learn_new_remote()) {
            remote_manager->add_remote_with_button(
                app->get_text_store(0), receiver->get_last_message());
        } else {
            remote_manager->add_button(app->get_text_store(0), receiver->get_last_message());
        }

        app->switch_to_next_scene_without_saving(IrdaApp::Scene::LearnDone);
    }
    return consumed;
}

void IrdaAppSceneLearnEnterName::on_exit(IrdaApp* app) {
}
