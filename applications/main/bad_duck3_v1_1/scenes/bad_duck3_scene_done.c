#include "../bad_duck3_app_i.h"

void bad_duck3_scene_done_on_enter(void* context) {
    BadDuck3App* app = context;

    popup_reset(app->popup);
    popup_set_icon(app->popup, 36, 5, &I_DolphinDone_80x58);
    popup_set_header(app->popup, "Done!", 20, 22, AlignLeft, AlignBottom);
    popup_set_timeout(app->popup, 1500);
    popup_set_context(app->popup, app);
    popup_set_callback(app->popup, NULL);
    popup_enable_timeout(app->popup);

    view_dispatcher_switch_to_view(app->view_dispatcher, BadDuck3AppViewPopup);
}

bool bad_duck3_scene_done_on_event(void* context, SceneManagerEvent event) {
    BadDuck3App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, BadDuck3SceneConfig);
    }

    return consumed;
}

void bad_duck3_scene_done_on_exit(void* context) {
    BadDuck3App* app = context;
    popup_reset(app->popup);
}
