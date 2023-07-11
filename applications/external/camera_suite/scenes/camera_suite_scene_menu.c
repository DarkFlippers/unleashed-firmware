#include "../camera_suite.h"

enum SubmenuIndex {
    /** Atkinson Dithering Algorithm. */
    SubmenuIndexSceneStyle1 = 10,
    /** Floyd-Steinberg Dithering Algorithm. */
    SubmenuIndexSceneStyle2,
    /** Guide/how-to. */
    SubmenuIndexGuide,
    /** Settings menu. */
    SubmenuIndexSettings,
};

void camera_suite_scene_menu_submenu_callback(void* context, uint32_t index) {
    CameraSuite* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void camera_suite_scene_menu_on_enter(void* context) {
    CameraSuite* app = context;

    submenu_add_item(
        app->submenu,
        "Open Camera",
        SubmenuIndexSceneStyle1,
        camera_suite_scene_menu_submenu_callback,
        app);
    // Staged view for the future.
    // submenu_add_item(
    //     app->submenu,
    //     "Test",
    //     SubmenuIndexSceneStyle2,
    //     camera_suite_scene_menu_submenu_callback,
    //     app);
    submenu_add_item(
        app->submenu, "Guide", SubmenuIndexGuide, camera_suite_scene_menu_submenu_callback, app);
    submenu_add_item(
        app->submenu,
        "Settings",
        SubmenuIndexSettings,
        camera_suite_scene_menu_submenu_callback,
        app);

    submenu_set_selected_item(
        app->submenu, scene_manager_get_scene_state(app->scene_manager, CameraSuiteSceneMenu));

    view_dispatcher_switch_to_view(app->view_dispatcher, CameraSuiteViewIdMenu);
}

bool camera_suite_scene_menu_on_event(void* context, SceneManagerEvent event) {
    CameraSuite* app = context;
    UNUSED(app);
    if(event.type == SceneManagerEventTypeBack) {
        // Exit application.
        scene_manager_stop(app->scene_manager);
        view_dispatcher_stop(app->view_dispatcher);
        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexSceneStyle1) {
            scene_manager_set_scene_state(
                app->scene_manager, CameraSuiteSceneMenu, SubmenuIndexSceneStyle1);
            scene_manager_next_scene(app->scene_manager, CameraSuiteSceneStyle_1);
            return true;
        } else if(event.event == SubmenuIndexSceneStyle2) {
            scene_manager_set_scene_state(
                app->scene_manager, CameraSuiteSceneMenu, SubmenuIndexSceneStyle2);
            scene_manager_next_scene(app->scene_manager, CameraSuiteSceneStyle_2);
            return true;
        } else if(event.event == SubmenuIndexGuide) {
            scene_manager_set_scene_state(
                app->scene_manager, CameraSuiteSceneMenu, SubmenuIndexGuide);
            scene_manager_next_scene(app->scene_manager, CameraSuiteSceneGuide);
            return true;
        } else if(event.event == SubmenuIndexSettings) {
            scene_manager_set_scene_state(
                app->scene_manager, CameraSuiteSceneMenu, SubmenuIndexSettings);
            scene_manager_next_scene(app->scene_manager, CameraSuiteSceneSettings);
            return true;
        }
    }
    return false;
}

void camera_suite_scene_menu_on_exit(void* context) {
    CameraSuite* app = context;
    submenu_reset(app->submenu);
}