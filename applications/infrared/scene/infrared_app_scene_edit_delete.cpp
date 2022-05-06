#include "../infrared_app.h"
#include "infrared.h"
#include "infrared/scene/infrared_app_scene.h"
#include <string>

static void dialog_result_callback(DialogExResult result, void* context) {
    auto app = static_cast<InfraredApp*>(context);
    InfraredAppEvent event;

    event.type = InfraredAppEvent::Type::DialogExSelected;
    event.payload.dialog_ex_result = result;

    app->get_view_manager()->send_event(&event);
}

void InfraredAppSceneEditDelete::on_enter(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();

    auto remote_manager = app->get_remote_manager();

    if(app->get_edit_element() == InfraredApp::EditElement::Button) {
        auto signal = remote_manager->get_button_data(app->get_current_button());
        dialog_ex_set_header(dialog_ex, "Delete button?", 64, 0, AlignCenter, AlignTop);
        if(!signal.is_raw()) {
            auto message = &signal.get_message();
            app->set_text_store(
                0,
                "%s\n%s\nA=0x%0*lX C=0x%0*lX",
                remote_manager->get_button_name(app->get_current_button()).c_str(),
                infrared_get_protocol_name(message->protocol),
                ROUND_UP_TO(infrared_get_protocol_address_length(message->protocol), 4),
                message->address,
                ROUND_UP_TO(infrared_get_protocol_command_length(message->protocol), 4),
                message->command);
        } else {
            app->set_text_store(
                0,
                "%s\nRAW\n%ld samples",
                remote_manager->get_button_name(app->get_current_button()).c_str(),
                signal.get_raw_signal().timings_cnt);
        }
    } else {
        dialog_ex_set_header(dialog_ex, "Delete remote?", 64, 0, AlignCenter, AlignTop);
        app->set_text_store(
            0,
            "%s\n with %lu buttons",
            remote_manager->get_remote_name().c_str(),
            remote_manager->get_number_of_buttons());
    }

    dialog_ex_set_text(dialog_ex, app->get_text_store(0), 64, 31, AlignCenter, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
    dialog_ex_set_left_button_text(dialog_ex, "Cancel");
    dialog_ex_set_right_button_text(dialog_ex, "Delete");
    dialog_ex_set_result_callback(dialog_ex, dialog_result_callback);
    dialog_ex_set_context(dialog_ex, app);

    view_manager->switch_to(InfraredAppViewManager::ViewId::DialogEx);
}

bool InfraredAppSceneEditDelete::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;

    if(event->type == InfraredAppEvent::Type::DialogExSelected) {
        switch(event->payload.dialog_ex_result) {
        case DialogExResultLeft:
            app->switch_to_previous_scene();
            break;
        case DialogExResultCenter:
            furi_assert(0);
            break;
        case DialogExResultRight: {
            auto remote_manager = app->get_remote_manager();
            bool result = false;
            if(app->get_edit_element() == InfraredApp::EditElement::Remote) {
                result = remote_manager->delete_remote();
            } else {
                result = remote_manager->delete_button(app->get_current_button());
                app->set_current_button(InfraredApp::ButtonNA);
            }

            if(!result) {
                app->search_and_switch_to_previous_scene(
                    {InfraredApp::Scene::RemoteList, InfraredApp::Scene::Start});
            } else {
                app->switch_to_next_scene(InfraredApp::Scene::EditDeleteDone);
            }
            break;
        }
        default:
            break;
        }
    }

    return consumed;
}

void InfraredAppSceneEditDelete::on_exit(InfraredApp*) {
}
