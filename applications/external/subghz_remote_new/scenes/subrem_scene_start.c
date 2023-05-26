#include "../subghz_remote_app_i.h"
#include "../helpers/subrem_custom_event.h"

void subrem_scene_start_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void subrem_scene_start_on_enter(void* context) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    Submenu* submenu = app->submenu;
#if FURI_DEBUG
    submenu_add_item(
        submenu,
        "Remote_Debug",
        SubmenuIndexSubRemRemoteView,
        subrem_scene_start_submenu_callback,
        app);
#endif
    // submenu_add_item(
    //     submenu,
    //     "About",
    //     SubmenuIndexSubGhzRemoteAbout,
    //     subrem_scene_start_submenu_callback,
    //     app);

    // TODO: set scene state in subrem alloc
    // submenu_set_selected_item(
    //     submenu, scene_manager_get_scene_state(app->scene_manager, SubRemSceneStart));
    submenu_set_selected_item(submenu, 0);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewSubmenu);
}

bool subrem_scene_start_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    bool consumed = false;
    UNUSED(app);
    if(event.type == SceneManagerEventTypeCustom) {
        // } else if(event.event == SubmenuIndexSubRemAbout) {
        //     scene_manager_next_scene(app->scene_manager, SubRemSceneAbout);
        //     consumed = true;
        // }
    }

    return consumed;
}

void subrem_scene_start_on_exit(void* context) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    submenu_reset(app->submenu);
}
