#include "../irda_app.h"
#include <file_worker_cpp.h>
#include "irda.h"
#include <memory>

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

    auto signal = app->get_received_signal();

    if(!signal.is_raw()) {
        auto message = &signal.get_message();
        uint8_t adr_digits = ROUND_UP_TO(irda_get_protocol_address_length(message->protocol), 4);
        uint8_t cmd_digits = ROUND_UP_TO(irda_get_protocol_command_length(message->protocol), 4);
        uint8_t max_digits = MAX(adr_digits, cmd_digits);
        max_digits = MIN(max_digits, 7);
        size_t label_x_offset = 63 + (7 - max_digits) * 3;

        app->set_text_store(0, "%s", irda_get_protocol_name(message->protocol));
        app->set_text_store(
            1,
            "A: 0x%0*lX\nC: 0x%0*lX\n",
            adr_digits,
            message->address,
            cmd_digits,
            message->command);

        dialog_ex_set_header(dialog_ex, app->get_text_store(0), 95, 7, AlignCenter, AlignCenter);
        dialog_ex_set_text(
            dialog_ex, app->get_text_store(1), label_x_offset, 34, AlignLeft, AlignCenter);
    } else {
        dialog_ex_set_header(dialog_ex, "Unknown", 95, 10, AlignCenter, AlignCenter);
        app->set_text_store(0, "%d samples", signal.get_raw_signal().timings_cnt);
        dialog_ex_set_text(dialog_ex, app->get_text_store(0), 75, 23, AlignLeft, AlignTop);
    }

    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "Save");
    dialog_ex_set_center_button_text(dialog_ex, "Send");
    dialog_ex_set_icon(dialog_ex, 0, 1, &I_DolphinReadingSuccess_59x63);
    dialog_ex_set_result_callback(dialog_ex, dialog_result_callback);
    dialog_ex_set_context(dialog_ex, app);

    view_manager->switch_to(IrdaAppViewManager::ViewType::DialogEx);
}

bool IrdaAppSceneLearnSuccess::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;
    if(event->type == IrdaAppEvent::Type::Tick) {
        /* Send event every tick to suppress any switching off green light */
        app->notify_green_on();
    }

    if(event->type == IrdaAppEvent::Type::DialogExSelected) {
        switch(event->payload.dialog_ex_result) {
        case DialogExResultLeft:
            app->switch_to_next_scene_without_saving(IrdaApp::Scene::Learn);
            break;
        case DialogExResultCenter: {
            app->notify_sent_just_learnt();
            auto signal = app->get_received_signal();
            signal.transmit();
            break;
        }
        case DialogExResultRight: {
            FileWorkerCpp file_worker;
            if(file_worker.check_errors()) {
                app->switch_to_next_scene(IrdaApp::Scene::LearnEnterName);
            } else {
                app->switch_to_previous_scene();
            }
            break;
        }
        default:
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
