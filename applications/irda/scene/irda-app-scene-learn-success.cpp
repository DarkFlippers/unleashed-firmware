#include "../irda-app.hpp"
#include "irda.h"
#include <string>
#include <stdio.h>

static void dialog_result_callback(DialogExResult result, void* context) {
    auto app = static_cast<IrdaApp*>(context);
    IrdaAppEvent event;

    event.type = IrdaAppEvent::Type::DialogExSelected;
    event.payload.dialog_ex_result = result;

    app->get_view_manager()->send_event(&event);
}

void IrdaAppSceneLearnSuccess::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();

    app->notify_green_on();

    auto receiver = app->get_receiver();
    auto message = receiver->get_last_message();

    app->set_text_store(0, "%s", irda_get_protocol_name(message->protocol));
    app->set_text_store(
        1,
        "A: 0x%0*lX\nC: 0x%0*lX\n",
        irda_get_protocol_address_length(message->protocol),
        message->address,
        irda_get_protocol_command_length(message->protocol),
        message->command);
    dialog_ex_set_header(dialog_ex, app->get_text_store(0), 95, 10, AlignCenter, AlignCenter);
    dialog_ex_set_text(dialog_ex, app->get_text_store(1), 75, 23, AlignLeft, AlignTop);
    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "Save");
    dialog_ex_set_center_button_text(dialog_ex, "Send");
    dialog_ex_set_icon(dialog_ex, 0, 1, I_DolphinExcited_64x63);
    dialog_ex_set_result_callback(dialog_ex, dialog_result_callback);
    dialog_ex_set_context(dialog_ex, app);

    view_manager->switch_to(IrdaAppViewManager::ViewType::DialogEx);
}

bool IrdaAppSceneLearnSuccess::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::DialogExSelected) {
        switch(event->payload.dialog_ex_result) {
        case DialogExResultLeft:
            app->switch_to_next_scene_without_saving(IrdaApp::Scene::Learn);
            break;
        case DialogExResultCenter: {
            app->notify_space_blink();
            auto receiver = app->get_receiver();
            auto message = receiver->get_last_message();
            irda_send(message, 1);
            break;
        }
        case DialogExResultRight:
            auto remote_manager = app->get_remote_manager();
            if(remote_manager->check_fs()) {
                app->switch_to_next_scene(IrdaApp::Scene::LearnEnterName);
            } else {
                app->switch_to_previous_scene();
            }
            break;
        }
    }

    return consumed;
}

void IrdaAppSceneLearnSuccess::on_exit(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();
    dialog_ex_set_center_button_text(dialog_ex, nullptr);
    app->notify_green_off();
}
