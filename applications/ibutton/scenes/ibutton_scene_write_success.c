#include "../ibutton_i.h"

static void ibutton_scene_write_success_popup_callback(void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventBack);
    ibutton_notification_message(ibutton, iButtonNotificationMessageGreenOff);
}

void ibutton_scene_write_success_on_enter(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;

    popup_set_icon(popup, 0, 12, &I_iButtonDolphinVerySuccess_108x52);
    popup_set_text(popup, "Successfully written!", 40, 12, AlignLeft, AlignBottom);

    popup_set_callback(popup, ibutton_scene_write_success_popup_callback);
    popup_set_context(popup, ibutton);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewPopup);
    ibutton_notification_message(ibutton, iButtonNotificationMessageSuccess);
    ibutton_notification_message(ibutton, iButtonNotificationMessageGreenOn);
}

bool ibutton_scene_write_success_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventBack) {
            const uint32_t possible_scenes[] = {iButtonSceneReadKeyMenu, iButtonSceneStart};
            scene_manager_search_and_switch_to_previous_scene_one_of(
                ibutton->scene_manager, possible_scenes, COUNT_OF(possible_scenes));
        }
    }

    return consumed;
}

void ibutton_scene_write_success_on_exit(void* context) {
    iButton* ibutton = context;
    Popup* popup = ibutton->popup;

    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);

    popup_disable_timeout(popup);
    popup_set_context(popup, NULL);
    popup_set_callback(popup, NULL);
}
