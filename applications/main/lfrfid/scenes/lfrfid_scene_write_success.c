#include "../lfrfid_i.h"

void lfrfid_scene_write_success_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    popup_set_header(popup, "Success!", 75, 10, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 9, &I_DolphinSuccess_91x55);
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

    if(event.type == SceneManagerEventTypeBack || event.type == SceneManagerEventTypeCustom) {
        if(!scene_manager_search_and_switch_to_previous_scene(
               app->scene_manager, LfRfidSceneReadKeyMenu)) {
            scene_manager_search_and_switch_to_another_scene(
                app->scene_manager, LfRfidSceneSelectKey);
        }
        consumed = true;
    }

    return consumed;
}

void lfrfid_scene_write_success_on_exit(void* context) {
    LfRfid* app = context;
    notification_message_block(app->notifications, &sequence_reset_green);
    popup_reset(app->popup);
}
