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
    submenu_add_item(
        submenu,
        "Open Map File",
        SubmenuIndexSubRemOpenMapFile,
        subrem_scene_start_submenu_callback,
        app);
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
#ifndef SUBREM_LIGHT
    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, SubRemSceneStart));
#endif
    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDSubmenu);
}

bool subrem_scene_start_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexSubRemOpenMapFile) {
#ifndef SUBREM_LIGHT
            scene_manager_set_scene_state(
                app->scene_manager, SubRemSceneStart, SubmenuIndexSubRemOpenMapFile);
#endif
            scene_manager_next_scene(app->scene_manager, SubRemSceneOpenMapFile);
            consumed = true;
        }
        // } else if(event.event == SubmenuIndexSubRemAbout) {
        //     scene_manager_next_scene(app->scene_manager, SubRemSceneAbout);
        //     consumed = true;
        // }
#if FURI_DEBUG
        else if(event.event == SubmenuIndexSubRemRemoteView) {
            scene_manager_next_scene(app->scene_manager, SubRemSceneRemote);
            consumed = true;
        }
#endif
    }

    return consumed;
}

void subrem_scene_start_on_exit(void* context) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    submenu_reset(app->submenu);
}
