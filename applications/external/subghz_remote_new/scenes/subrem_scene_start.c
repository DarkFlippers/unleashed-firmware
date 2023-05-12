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
        SubmenuIndexOpenMapFile,
        subrem_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu, "Remote", SubmenuIndexOpenView, subrem_scene_start_submenu_callback, app);
    // submenu_add_item(
    //     submenu,
    //     "ISP Programmer",
    //     SubmenuIndexSubGhzRemoteProgrammer,
    //     subrem_scene_start_submenu_callback,
    //     app);
    // submenu_add_item(
    //     submenu,
    //     "Wiring",
    //     SubmenuIndexAvrIsWiring,
    //     subrem_scene_start_submenu_callback,
    //     app);
    // submenu_add_item(
    //     submenu,
    //     "About",
    //     SubmenuIndexSubGhzRemoteAbout,
    //     subrem_scene_start_submenu_callback,
    //     app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, SubRemSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewSubmenu);
}

bool subrem_scene_start_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexOpenMapFile) {
            scene_manager_next_scene(app->scene_manager, SubRemSceneOpenMapFile);
            consumed = true;
        } else if(event.event == SubmenuIndexOpenView) {
            scene_manager_next_scene(app->scene_manager, SubRemSceneRemote);
            consumed = true;
        }
        // } else if(event.event == SubmenuIndexSubGhzRemoteProgrammer) {
        //     scene_manager_set_scene_state(
        //         app->scene_manager, SubRemSceneChipDetect, SubGhzRemoteViewProgrammer);
        //     scene_manager_next_scene(app->scene_manager, SubRemSceneChipDetect);
        //     consumed = true;
        // } else if(event.event == SubmenuIndexSubGhzRemoteReader) {
        //     scene_manager_set_scene_state(
        //         app->scene_manager, SubRemSceneChipDetect, SubGhzRemoteViewReader);
        //     scene_manager_next_scene(app->scene_manager, SubRemSceneChipDetect);
        //     consumed = true;
        // } else if(event.event == SubmenuIndexSubGhzRemoteWriter) {
        //     scene_manager_set_scene_state(
        //         app->scene_manager, SubRemSceneChipDetect, SubGhzRemoteViewWriter);
        //     scene_manager_next_scene(app->scene_manager, SubRemSceneChipDetect);
        //     consumed = true;
        // } else if(event.event == SubmenuIndexAvrIsWiring) {
        //     scene_manager_next_scene(app->scene_manager, SubRemSceneWiring);
        //     consumed = true;
        // }
        scene_manager_set_scene_state(app->scene_manager, SubRemSceneStart, event.event);
    }

    return consumed;
}

void subrem_scene_start_on_exit(void* context) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    submenu_reset(app->submenu);
}
