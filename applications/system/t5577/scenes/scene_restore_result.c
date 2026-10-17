#include "../t5577_app.h"
#include <t5577_icons.h>

void t5577_app_scene_restore_result_on_enter(void* context) {
    T5577App* app = context;

    const bool answered = app->config.clock != T5577ReadClockUnsupported;
    const bool restored = answered && app->bad == 0;

    popup_reset(app->popup);

    if(restored) {
        popup_set_header(app->popup, "Written!", 75, 10, AlignLeft, AlignTop);
        popup_set_icon(app->popup, 0, 9, &I_DolphinSuccess_91x55);
        notification_message(app->notifications, &sequence_success);
    } else {
        popup_set_header(app->popup, "Not written", 64, 4, AlignCenter, AlignTop);

        // Which of the two it is matters: a tag that went quiet is a tag whose configuration or
        // password is not what the check expected, where differing blocks are writes that the
        // chip refused - a locked block, or the wrong password.
        if(answered) {
            snprintf(
                app->progress,
                sizeof(app->progress),
                "%u of %u blocks\ndiffer",
                app->bad,
                app->done);
        } else {
            snprintf(
                app->progress,
                sizeof(app->progress),
                "Wrote %u blocks,\ntag went quiet",
                app->done);
        }
        popup_set_text(app->popup, app->progress, 64, 22, AlignCenter, AlignTop);
        notification_message(app->notifications, &sequence_error);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewPopup);
}

bool t5577_app_scene_restore_result_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Back leaves the write behind rather than returning into it: the restore scene writes the
    // tag on entry, so stepping back into it would write the tag again.
    if(event.type != SceneManagerEventTypeBack) return false;

    if(!scene_manager_search_and_switch_to_previous_scene(
           app->scene_manager, T5577AppSceneFileMenu)) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);
    }

    return true;
}

void t5577_app_scene_restore_result_on_exit(void* context) {
    T5577App* app = context;
    popup_reset(app->popup);
}
