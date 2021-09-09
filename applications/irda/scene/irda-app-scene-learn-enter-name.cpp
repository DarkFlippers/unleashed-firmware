#include "../irda-app.h"
#include "gui/modules/text_input.h"

void IrdaAppSceneLearnEnterName::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    TextInput* text_input = view_manager->get_text_input();

    auto signal = app->get_received_signal();

    if(!signal.is_raw()) {
        auto message = &signal.get_message();
        app->set_text_store(
            0,
            "%.4s_%0*lX",
            irda_get_protocol_name(message->protocol),
            ROUND_UP_TO(irda_get_protocol_command_length(message->protocol), 4),
            message->command);
    } else {
        auto raw_signal = signal.get_raw_signal();
        app->set_text_store(0, "RAW_%d", raw_signal.timings_cnt);
    }

    text_input_set_header_text(text_input, "Name the key");
    text_input_set_result_callback(
        text_input,
        IrdaApp::text_input_callback,
        app,
        app->get_text_store(0),
        IrdaAppRemoteManager::max_button_name_length,
        false);

    view_manager->switch_to(IrdaAppViewManager::ViewType::TextInput);
}

bool IrdaAppSceneLearnEnterName::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::TextEditDone) {
        auto remote_manager = app->get_remote_manager();
        bool result = false;
        if(app->get_learn_new_remote()) {
            result = remote_manager->add_remote_with_button(
                app->get_text_store(0), app->get_received_signal());
        } else {
            result =
                remote_manager->add_button(app->get_text_store(0), app->get_received_signal());
        }

        if(!result) {
            app->search_and_switch_to_previous_scene(
                {IrdaApp::Scene::Start, IrdaApp::Scene::RemoteList});
        } else {
            app->switch_to_next_scene_without_saving(IrdaApp::Scene::LearnDone);
        }
    }
    return consumed;
}

void IrdaAppSceneLearnEnterName::on_exit(IrdaApp* app) {
}
