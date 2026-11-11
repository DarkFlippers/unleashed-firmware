#include "../t5577_app.h"
#include <t5577_icons.h>

void t5577_app_scene_wipe_result_on_enter(void* context) {
    T5577App* app = context;

    popup_reset(app->popup);

    if(app->wipe_result == T5577BlockHeld) {
        popup_set_header(app->popup, "Wiped!", 75, 10, AlignLeft, AlignTop);
        popup_set_icon(app->popup, 0, 9, &I_DolphinSuccess_91x55);
        notification_message(app->notifications, &sequence_success);
    } else {
        popup_set_header(app->popup, "Not wiped", 64, 4, AlignCenter, AlignTop);

        // What was measured, then the likeliest cause and what would answer it. A protected tag
        // lands on "no answer" rather than on a mismatch: it does keep broadcasting in regular
        // read mode, but in its own configuration, and a stream at the wrong bit rate never
        // yields a coherent run. Deliberately unsaid: "not a T5577", which cannot be told from a
        // tag out of range, and anything about lock bits, which nothing here can see.
        popup_set_text(
            app->popup,
            (app->wipe_result == T5577BlockNoAnswer) ?
                "No answer.\nProtected? Try\nClear Password" :
            (app->wipe_result == T5577BlockMismatch) ? "The tag answered,\nbut not as wiped" :
                                                       "Could not check\nwhat the tag holds",
            64,
            22,
            AlignCenter,
            AlignTop);
        notification_message(app->notifications, &sequence_error);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, T5577AppViewPopup);
}

bool t5577_app_scene_wipe_result_on_event(void* context, SceneManagerEvent event) {
    T5577App* app = context;

    // Back goes past the wipe rather than into it: that scene writes the tag on entry.
    if(event.type != SceneManagerEventTypeBack) return false;

    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, T5577AppSceneStart);

    return true;
}

void t5577_app_scene_wipe_result_on_exit(void* context) {
    T5577App* app = context;
    popup_reset(app->popup);
}
