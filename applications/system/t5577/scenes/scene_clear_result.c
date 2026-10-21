#include "../t5577_app.h"
#include <t5577_icons.h>

void t5577_app_scene_clear_result_on_enter(void* context) {
    T5577App* app = context;

    popup_reset(app->popup);

    if(app->cleared) {
        popup_set_header(app->popup, "Cleared!", 75, 10, AlignLeft, AlignTop);
        popup_set_icon(app->popup, 0, 9, &I_DolphinSuccess_91x55);
        notification_message(app->notifications, &sequence_success);
    } else {
        popup_set_header(app->popup, "Not cleared", 64, 4, AlignCenter, AlignTop);
        // A tag that never answered and a tag whose password is not in the list end a sweep the
        // same way and cannot be told apart from here. The two that can are worth saying: a
        // dictionary that was never there, and one that stopped part way.
        const char* why = "No password matched,\nor no tag answered";

        if(app->dict_failed) {
            why = "Password list stopped:\ncheck the SD card";
        } else if(app->candidates == 0) {
            why = "No password list:\ncheck the SD card";
        }

        popup_set_text(app->popup, why, 64, 22, AlignCenter, AlignTop);
        notification_message(app->notifications, &sequence_error);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewPopup);
}

bool t5577_app_scene_clear_result_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Back goes past the sweep rather than into it: that scene writes the tag on entry.
    if(event.type != SceneManagerEventTypeBack) return false;

    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);

    return true;
}

void t5577_app_scene_clear_result_on_exit(void* context) {
    T5577App* app = context;
    popup_reset(app->popup);
}
