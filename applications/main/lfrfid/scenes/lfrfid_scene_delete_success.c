#include "../lfrfid_i.h"

void lfrfid_scene_delete_success_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    popup_set_icon(popup, 0, 2, &I_DolphinMafia_119x62);
    popup_set_header(popup, "Deleted", 80, 19, AlignLeft, AlignBottom);
    popup_set_context(popup, app);
    popup_set_callback(popup, lfrfid_popup_timeout_callback);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

bool lfrfid_scene_delete_success_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack || event.type == SceneManagerEventTypeCustom) {
        // Always return to SceneSelectKey from here
        scene_manager_search_and_switch_to_another_scene(app->scene_manager, LfRfidSceneSelectKey);
        consumed = true;
    }

    return consumed;
}

void lfrfid_scene_delete_success_on_exit(void* context) {
    LfRfid* app = context;

    popup_reset(app->popup);
}
