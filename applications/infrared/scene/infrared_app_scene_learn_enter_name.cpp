#include "../infrared_app.h"
#include "gui/modules/text_input.h"

void InfraredAppSceneLearnEnterName::on_enter(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    TextInput* text_input = view_manager->get_text_input();

    auto signal = app->get_received_signal();

    if(!signal.is_raw()) {
        auto message = &signal.get_message();
        app->set_text_store(
            0,
            "%.4s_%0*lX",
            infrared_get_protocol_name(message->protocol),
            ROUND_UP_TO(infrared_get_protocol_command_length(message->protocol), 4),
            message->command);
    } else {
        auto raw_signal = signal.get_raw_signal();
        app->set_text_store(0, "RAW_%d", raw_signal.timings_cnt);
    }

    text_input_set_header_text(text_input, "Name the button");
    text_input_set_result_callback(
        text_input,
        InfraredApp::text_input_callback,
        app,
        app->get_text_store(0),
        InfraredAppRemoteManager::max_button_name_length,
        true);

    view_manager->switch_to(InfraredAppViewManager::ViewId::TextInput);
}

bool InfraredAppSceneLearnEnterName::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    if(event->type == InfraredAppEvent::Type::TextEditDone) {
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
                {InfraredApp::Scene::Start, InfraredApp::Scene::RemoteList});
        } else {
            app->switch_to_next_scene_without_saving(InfraredApp::Scene::LearnDone);
        }
    }
    return consumed;
}

void InfraredAppSceneLearnEnterName::on_exit(InfraredApp*) {
}
