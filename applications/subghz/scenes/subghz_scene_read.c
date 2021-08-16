#include "../subghz_i.h"

#define GUBGHZ_READ_CUSTOM_EVENT (10UL)

void subghz_read_protocol_callback(SubGhzProtocolCommon* parser, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    subghz->protocol_result = parser;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, GUBGHZ_READ_CUSTOM_EVENT);
}
void subghz_scene_read_callback(DialogExResult result, void* context) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, result);
}

const void subghz_scene_read_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    DialogEx* dialog_ex = subghz->dialog_ex;

    dialog_ex_set_header(dialog_ex, "SubGhz 433.92", 36, 6, AlignLeft, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 10, 12, &I_RFIDDolphinReceive_97x61);

    //Start CC1101 rx
    subghz_begin(FuriHalSubGhzPresetOokAsync);
    subghz_rx(433920000);

    furi_hal_subghz_start_async_rx(subghz_worker_rx_callback, subghz->worker);
    subghz_worker_start(subghz->worker);
    subghz_protocol_enable_dump(subghz->protocol, subghz_read_protocol_callback, subghz);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewDialogEx);
}

const bool subghz_scene_read_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GUBGHZ_READ_CUSTOM_EVENT) {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiver);
            notification_message(subghz->notifications, &sequence_success);
            return true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        notification_message(subghz->notifications, &sequence_blink_blue_10);
        return true;
    }
    return false;
}

const void subghz_scene_read_on_exit(void* context) {
    SubGhz* subghz = context;

    // Stop CC1101
    subghz_worker_stop(subghz->worker);
    furi_hal_subghz_stop_async_rx();
    subghz_end();

    DialogEx* dialog_ex = subghz->dialog_ex;
    dialog_ex_set_header(dialog_ex, NULL, 0, 0, AlignCenter, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
}
