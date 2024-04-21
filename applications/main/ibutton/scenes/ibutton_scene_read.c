#include "../ibutton_i.h"
#include <dolphin/dolphin.h>

static void ibutton_scene_read_callback(void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventWorkerRead);
}

void ibutton_scene_read_on_enter(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;
    iButtonKey* key = ibutton->key;
    iButtonWorker* worker = ibutton->worker;

    popup_set_header(popup, "Reading", 95, 26, AlignCenter, AlignBottom);
    popup_set_text(popup, "Connect key\nwith pogo pins", 95, 30, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 10, &I_DolphinWait_59x54);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);

    ibutton_worker_read_set_callback(worker, ibutton_scene_read_callback, ibutton);
    ibutton_worker_read_start(worker, key);

    ibutton_notification_message(ibutton, iButtonNotificationMessageReadStart);
}

bool ibutton_scene_read_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventWorkerRead) {
            if(ibutton_protocols_is_valid(ibutton->protocols, ibutton->key)) {
                ibutton_notification_message(ibutton, iButtonNotificationMessageSuccess);
                scene_manager_next_scene(scene_manager, iButtonSceneReadSuccess);

                dolphin_deed(DolphinDeedIbuttonReadSuccess);

            } else {
                scene_manager_next_scene(scene_manager, iButtonSceneReadError);
            }
        }
    }

    return consumed;
}

void ibutton_scene_read_on_exit(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;
    ibutton_worker_stop(ibutton->worker);
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);

    ibutton_notification_message(ibutton, iButtonNotificationMessageBlinkStop);
}
