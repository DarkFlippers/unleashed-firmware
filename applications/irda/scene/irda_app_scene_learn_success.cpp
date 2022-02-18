#include <gui/modules/dialog_ex.h>
#include <file_worker_cpp.h>
#include <memory>
#include <dolphin/dolphin.h>

#include "../irda_app.h"
#include "irda.h"

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

    DOLPHIN_DEED(DolphinDeedIrLearnSuccess);
    app->notify_green_on();

    irda_worker_tx_set_get_signal_callback(
        app->get_irda_worker(), irda_worker_tx_get_signal_steady_callback, app);
    irda_worker_tx_set_signal_sent_callback(
        app->get_irda_worker(), IrdaApp::signal_sent_callback, app);

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
    dialog_ex_enable_extended_events(dialog_ex);

    view_manager->switch_to(IrdaAppViewManager::ViewType::DialogEx);
}

bool IrdaAppSceneLearnSuccess::on_event(IrdaApp* app, IrdaAppEvent* event) {
    bool consumed = false;
    if(event->type == IrdaAppEvent::Type::Tick) {
        /* Send event every tick to suppress any switching off green light */
        if(!button_pressed) {
            app->notify_green_on();
        }
    }

    if(event->type == IrdaAppEvent::Type::DialogExSelected) {
        switch(event->payload.dialog_ex_result) {
        case DialogExResultLeft:
            consumed = true;
            if(!button_pressed) {
                app->switch_to_next_scene_without_saving(IrdaApp::Scene::Learn);
            }
            break;
        case DialogExResultRight: {
            consumed = true;
            FileWorkerCpp file_worker;
            if(!button_pressed) {
                if(file_worker.check_errors()) {
                    app->switch_to_next_scene(IrdaApp::Scene::LearnEnterName);
                } else {
                    app->switch_to_previous_scene();
                }
            }
            break;
        }
        case DialogExPressCenter:
            if(!button_pressed) {
                button_pressed = true;
                app->notify_click_and_green_blink();

                auto signal = app->get_received_signal();
                if(signal.is_raw()) {
                    irda_worker_set_raw_signal(
                        app->get_irda_worker(),
                        signal.get_raw_signal().timings,
                        signal.get_raw_signal().timings_cnt);
                } else {
                    irda_worker_set_decoded_signal(app->get_irda_worker(), &signal.get_message());
                }

                irda_worker_tx_start(app->get_irda_worker());
            }
            break;
        case DialogExReleaseCenter:
            if(button_pressed) {
                button_pressed = false;
                irda_worker_tx_stop(app->get_irda_worker());
                app->notify_green_off();
            }
            break;
        default:
            break;
        }
    }

    if(event->type == IrdaAppEvent::Type::Back) {
        if(!button_pressed) {
            app->switch_to_next_scene(IrdaApp::Scene::AskBack);
        }
        consumed = true;
    }

    return consumed;
}

void IrdaAppSceneLearnSuccess::on_exit(IrdaApp* app) {
    IrdaAppViewManager* view_manager = app->get_view_manager();
    DialogEx* dialog_ex = view_manager->get_dialog_ex();
    dialog_ex_reset(dialog_ex);
    app->notify_green_off();
    irda_worker_tx_set_get_signal_callback(app->get_irda_worker(), nullptr, nullptr);
    irda_worker_tx_set_signal_sent_callback(app->get_irda_worker(), nullptr, nullptr);
}
