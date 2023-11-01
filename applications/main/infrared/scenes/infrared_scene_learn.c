#include "../infrared_app_i.h"
#include <dolphin/dolphin.h>

void infrared_scene_learn_on_enter(void* context) {
    InfraredApp* infrared = context;
    Popup* popup = infrared->popup;
    InfraredWorker* worker = infrared->worker;

    infrared_worker_rx_set_received_signal_callback(
        worker, infrared_signal_received_callback, context);
    infrared_worker_rx_start(worker);
    infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStartRead);

    popup_set_icon(popup, 0, 32, &I_InfraredLearnShort_128x31);
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignCenter);
    popup_set_text(
        popup, "Point the remote at IR port\nand push the button", 5, 10, AlignLeft, AlignCenter);
    popup_set_callback(popup, NULL);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewPopup);
}

bool infrared_scene_learn_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypeSignalReceived) {
            infrared_play_notification_message(infrared, InfraredNotificationMessageSuccess);
            scene_manager_next_scene(infrared->scene_manager, InfraredSceneLearnSuccess);
            dolphin_deed(DolphinDeedIrLearnSuccess);
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_learn_on_exit(void* context) {
    InfraredApp* infrared = context;
    Popup* popup = infrared->popup;
    infrared_worker_rx_set_received_signal_callback(infrared->worker, NULL, NULL);
    infrared_worker_rx_stop(infrared->worker);
    infrared_play_notification_message(infrared, InfraredNotificationMessageBlinkStop);
    popup_set_icon(popup, 0, 0, NULL);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignCenter);
}
