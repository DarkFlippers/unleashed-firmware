#include "../irda-app.h"
#include "irda.h"
#include "irda/scene/irda-app-scene.h"
#include <string>

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
        auto signal = remote_manager->get_button_data(app->get_current_button());
        dialog_ex_set_header(dialog_ex, "Delete button?", 64, 6, AlignCenter, AlignCenter);
        if(!signal.is_raw()) {
            auto message = &signal.get_message();
            app->set_text_store(
                0,
                "%s\n%s\nA=0x%0*lX C=0x%0*lX",
                remote_manager->get_button_name(app->get_current_button()).c_str(),
                irda_get_protocol_name(message->protocol),
                irda_get_protocol_address_length(message->protocol),
                message->address,
                irda_get_protocol_command_length(message->protocol),
                message->command);
        } else {
            app->set_text_store(
                0,
                "%s\nRAW\n%ld samples",
                remote_manager->get_button_name(app->get_current_button()).c_str(),
                signal.get_raw_signal().timings_cnt);
        }
    } else {
        dialog_ex_set_header(dialog_ex, "Delete remote?", 64, 6, AlignCenter, AlignCenter);
        app->set_text_store(
            0,
            "%s\n with %lu buttons",
            remote_manager->get_remote_name().c_str(),
            remote_manager->get_number_of_buttons());
    }

    dialog_ex_set_text(dialog_ex, app->get_text_store(0), 64, 32, AlignCenter, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
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
            bool result = false;
            if(app->get_edit_element() == IrdaApp::EditElement::Remote) {
                result = remote_manager->delete_remote();
            } else {
                result = remote_manager->delete_button(app->get_current_button());
                app->set_current_button(IrdaApp::ButtonNA);
            }

            if(!result) {
                app->search_and_switch_to_previous_scene(
                    {IrdaApp::Scene::RemoteList, IrdaApp::Scene::Start});
            } else {
                app->switch_to_next_scene(IrdaApp::Scene::EditDeleteDone);
            }
            break;
        }
    }

    return consumed;
}

void IrdaAppSceneEditDelete::on_exit(IrdaApp* app) {
}
