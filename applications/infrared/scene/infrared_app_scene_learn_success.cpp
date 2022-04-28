#include <gui/modules/dialog_ex.h>
#include <memory>
#include <dolphin/dolphin.h>

#include "../infrared_app.h"
#include "infrared.h"

static void dialog_result_callback(DialogExResult result, void* context) {
    auto app = static_cast<InfraredApp*>(context);
    InfraredAppEvent event;

    event.type = InfraredAppEvent::Type::DialogExSelected;
    event.payload.dialog_ex_result = result;

    app->get_view_manager()->send_event(&event);
}

void InfraredAppSceneLearnSuccess::on_enter(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();

    DOLPHIN_DEED(DolphinDeedIrLearnSuccess);
    app->notify_green_on();

    infrared_worker_tx_set_get_signal_callback(
        app->get_infrared_worker(), infrared_worker_tx_get_signal_steady_callback, app);
    infrared_worker_tx_set_signal_sent_callback(
        app->get_infrared_worker(), InfraredApp::signal_sent_callback, app);

    auto signal = app->get_received_signal();

    if(!signal.is_raw()) {
        auto message = &signal.get_message();
        uint8_t adr_digits =
            ROUND_UP_TO(infrared_get_protocol_address_length(message->protocol), 4);
        uint8_t cmd_digits =
            ROUND_UP_TO(infrared_get_protocol_command_length(message->protocol), 4);
        uint8_t max_digits = MAX(adr_digits, cmd_digits);
        max_digits = MIN(max_digits, 7);
        size_t label_x_offset = 63 + (7 - max_digits) * 3;

        app->set_text_store(0, "%s", infrared_get_protocol_name(message->protocol));
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
    dialog_ex_enable_extended_events(dialog_ex);

    view_manager->switch_to(InfraredAppViewManager::ViewId::DialogEx);
}

bool InfraredAppSceneLearnSuccess::on_event(InfraredApp* app, InfraredAppEvent* event) {
    bool consumed = false;
    if(event->type == InfraredAppEvent::Type::Tick) {
        /* Send event every tick to suppress any switching off green light */
        if(!button_pressed) {
            app->notify_green_on();
        }
    }

    if(event->type == InfraredAppEvent::Type::DialogExSelected) {
        switch(event->payload.dialog_ex_result) {
        case DialogExResultLeft:
            consumed = true;
            if(!button_pressed) {
                app->switch_to_next_scene_without_saving(InfraredApp::Scene::Learn);
            }
            break;
        case DialogExResultRight: {
            consumed = true;
            if(!button_pressed) {
                app->switch_to_next_scene(InfraredApp::Scene::LearnEnterName);
            }
            break;
        }
        case DialogExPressCenter:
            if(!button_pressed) {
                button_pressed = true;

                auto signal = app->get_received_signal();
                if(signal.is_raw()) {
                    infrared_worker_set_raw_signal(
                        app->get_infrared_worker(),
                        signal.get_raw_signal().timings,
                        signal.get_raw_signal().timings_cnt);
                } else {
                    infrared_worker_set_decoded_signal(
                        app->get_infrared_worker(), &signal.get_message());
                }

                infrared_worker_tx_start(app->get_infrared_worker());
            }
            break;
        case DialogExReleaseCenter:
            if(button_pressed) {
                button_pressed = false;
                infrared_worker_tx_stop(app->get_infrared_worker());
                app->notify_green_off();
            }
            break;
        default:
            break;
        }
    }

    if(event->type == InfraredAppEvent::Type::Back) {
        if(!button_pressed) {
            app->switch_to_next_scene(InfraredApp::Scene::AskBack);
        }
        consumed = true;
    }

    return consumed;
}

void InfraredAppSceneLearnSuccess::on_exit(InfraredApp* app) {
    InfraredAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();
    dialog_ex_reset(dialog_ex);
    app->notify_green_off();
    infrared_worker_tx_set_get_signal_callback(app->get_infrared_worker(), nullptr, nullptr);
    infrared_worker_tx_set_signal_sent_callback(app->get_infrared_worker(), nullptr, nullptr);
}
