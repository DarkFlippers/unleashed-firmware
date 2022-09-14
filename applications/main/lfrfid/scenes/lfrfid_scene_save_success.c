#include "../lfrfid_i.h"
#include <dolphin/dolphin.h>

void lfrfid_scene_save_success_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    DOLPHIN_DEED(DolphinDeedRfidSave);
    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Saved!", 5, 7, AlignLeft, AlignTop);
    popup_set_context(popup, app);
    popup_set_callback(popup, lfrfid_popup_timeout_callback);
    popup_set_timeout(popup, 1500);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

bool lfrfid_scene_save_success_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    const uint32_t prev_scenes[] = {LfRfidSceneReadKeyMenu, LfRfidSceneSelectKey};

    if((event.type == SceneManagerEventTypeBack) ||
       ((event.type == SceneManagerEventTypeCustom) && (event.event == LfRfidEventPopupClosed))) {
        bool result = scene_manager_search_and_switch_to_previous_scene_one_of(
            app->scene_manager, prev_scenes, COUNT_OF(prev_scenes));
        if(!result) {
            scene_manager_search_and_switch_to_another_scene(
                app->scene_manager, LfRfidSceneSelectKey);
        }
        consumed = true;
    }

    return consumed;
}

void lfrfid_scene_save_success_on_exit(void* context) {
    LfRfid* app = context;

    popup_reset(app->popup);
}
