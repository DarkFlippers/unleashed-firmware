#include "../infrared_app_i.h"

static void
    infrared_scene_learn_success_dialog_result_callback(DialogExResult result, void* context) {
    InfraredApp* infrared = context;
    view_dispatcher_send_custom_event(infrared->view_dispatcher, result);
}

void infrared_scene_learn_success_on_enter(void* context) {
    InfraredApp* infrared = context;
    DialogEx* dialog_ex = infrared->dialog_ex;
    InfraredSignal* signal = infrared->current_signal;

    infrared_play_notification_message(infrared, InfraredNotificationMessageGreenOn);

    if(infrared_signal_is_raw(signal)) {
        const InfraredRawSignal* raw = infrared_signal_get_raw_signal(signal);
        dialog_ex_set_header(dialog_ex, "Unknown", 95, 10, AlignCenter, AlignCenter);
        infrared_text_store_set(infrared, 0, "%zu samples", raw->timings_size);
        dialog_ex_set_text(dialog_ex, infrared->text_store[0], 75, 23, AlignLeft, AlignTop);

    } else {
        const InfraredMessage* message = infrared_signal_get_message(signal);
        uint8_t addr_digits =
            ROUND_UP_TO(infrared_get_protocol_address_length(message->protocol), 4);
        uint8_t cmd_digits =
            ROUND_UP_TO(infrared_get_protocol_command_length(message->protocol), 4);
        uint8_t max_digits = MAX(addr_digits, cmd_digits);
        max_digits = MIN(max_digits, 7);
        size_t label_x_offset = 63 + (7 - max_digits) * 3;

        infrared_text_store_set(infrared, 0, "%s", infrared_get_protocol_name(message->protocol));
        infrared_text_store_set(
            infrared,
            1,
            "A: 0x%0*lX\nC: 0x%0*lX\n",
            addr_digits,
            message->address,
            cmd_digits,
            message->command);

        dialog_ex_set_header(dialog_ex, infrared->text_store[0], 95, 7, AlignCenter, AlignCenter);
        dialog_ex_set_text(
            dialog_ex, infrared->text_store[1], label_x_offset, 34, AlignLeft, AlignCenter);
    }

    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "Save");
    dialog_ex_set_center_button_text(dialog_ex, "Send");
    dialog_ex_set_icon(dialog_ex, 0, 1, &I_DolphinReadingSuccess_59x63);
    dialog_ex_set_result_callback(dialog_ex, infrared_scene_learn_success_dialog_result_callback);
    dialog_ex_set_context(dialog_ex, context);
    dialog_ex_enable_extended_events(dialog_ex);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewDialogEx);
}

bool infrared_scene_learn_success_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    const bool is_transmitter_idle = !infrared->app_state.is_transmitting;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        if(is_transmitter_idle) {
            infrared_play_notification_message(infrared, InfraredNotificationMessageGreenOn);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        if(is_transmitter_idle) {
            scene_manager_next_scene(scene_manager, InfraredSceneAskBack);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            if(is_transmitter_idle) {
                scene_manager_next_scene(scene_manager, InfraredSceneAskRetry);
            }
            consumed = true;
        } else if(event.event == DialogExResultRight) {
            if(is_transmitter_idle) {
                scene_manager_next_scene(scene_manager, InfraredSceneLearnEnterName);
            }
            consumed = true;
        } else if(event.event == DialogExPressCenter) {
            infrared_play_notification_message(infrared, InfraredNotificationMessageGreenOff);
            infrared_tx_start(infrared);
            consumed = true;
        } else if(event.event == DialogExReleaseCenter) {
            infrared_tx_stop(infrared);
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_learn_success_on_exit(void* context) {
    InfraredApp* infrared = context;
    dialog_ex_reset(infrared->dialog_ex);
    infrared_play_notification_message(infrared, InfraredNotificationMessageGreenOff);
}
