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

void IrdaAppSceneEditDelete::on_enter(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();

    auto remote_manager = app->get_remote_manager();

    if(app->get_edit_element() == IrdaApp::EditElement::Button) {
        auto message = remote_manager->get_button_data(remote_manager->get_current_button());
        dialog_ex_set_header(dialog_ex, "Delete button?", 64, 6, AlignCenter, AlignCenter);
        app->set_text_store(
            0,
            "%s\n%s\nA=0x%0*lX C=0x%0*lX",
            remote_manager->get_current_button_name().c_str(),
            irda_get_protocol_name(message->protocol),
            irda_get_protocol_address_length(message->protocol),
            message->address,
            irda_get_protocol_command_length(message->protocol),
            message->command);
    } else {
        dialog_ex_set_header(dialog_ex, "Delete remote?", 64, 6, AlignCenter, AlignCenter);
        app->set_text_store(
            0,
            "%s\n with %lu buttons",
            remote_manager->get_current_remote_name().c_str(),
            remote_manager->get_current_remote_buttons_number());
    }

    dialog_ex_set_text(dialog_ex, app->get_text_store(0), 64, 32, AlignCenter, AlignCenter);
    dialog_ex_set_icon(dialog_ex, -1, -1, I_ButtonCenter_7x7);
    dialog_ex_set_left_button_text(dialog_ex, "Back");
    dialog_ex_set_right_button_text(dialog_ex, "Delete");
    dialog_ex_set_result_callback(dialog_ex, dialog_result_callback);
    dialog_ex_set_context(dialog_ex, app);

    view_manager->switch_to(IrdaAppViewManager::ViewType::DialogEx);
}

bool IrdaAppSceneEditDelete::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;

    if(event->type == IrdaAppEvent::Type::DialogExSelected) {
        switch(event->payload.dialog_ex_result) {
        case DialogExResultLeft:
            app->switch_to_previous_scene();
            break;
        case DialogExResultCenter:
            furi_assert(0);
            break;
        case DialogExResultRight:
            auto remote_manager = app->get_remote_manager();
            if(app->get_edit_element() == IrdaApp::EditElement::Remote) {
                remote_manager->delete_current_remote();
            } else {
                remote_manager->delete_current_button();
            }

            app->switch_to_next_scene(IrdaApp::Scene::EditDeleteDone);
            break;
        }
    }

    return consumed;
}

void IrdaAppSceneEditDelete::on_exit(IrdaApp* app) {
}
