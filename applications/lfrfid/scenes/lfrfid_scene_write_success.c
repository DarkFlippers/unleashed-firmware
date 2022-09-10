#include "../lfrfid_i.h"

void lfrfid_scene_write_success_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    popup_set_header(popup, "Successfully\nwritten!", 94, 3, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 6, &I_RFIDDolphinSuccess_108x57);
    popup_set_context(popup, app);
    popup_set_callback(popup, lfrfid_popup_timeout_callback);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
    notification_message_block(app->notifications, &sequence_set_green_255);
}

bool lfrfid_scene_write_success_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    const uint32_t prev_scenes[] = {LfRfidSceneReadKeyMenu, LfRfidSceneSelectKey};

    if((event.type == SceneManagerEventTypeBack) ||
       ((event.type == SceneManagerEventTypeCustom) && (event.event == LfRfidEventPopupClosed))) {
        scene_manager_search_and_switch_to_previous_scene_one_of(
            app->scene_manager, prev_scenes, COUNT_OF(prev_scenes));
        consumed = true;
    }

    return consumed;
}

void lfrfid_scene_write_success_on_exit(void* context) {
    LfRfid* app = context;
    notification_message_block(app->notifications, &sequence_reset_green);
    popup_reset(app->popup);
}
