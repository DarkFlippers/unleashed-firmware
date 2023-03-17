#include "../pocsag_pager_app_i.h"

typedef enum {
    SubmenuIndexPOCSAGPagerReceiver,
    SubmenuIndexPOCSAGPagerAbout,
} SubmenuIndex;

void pocsag_pager_scene_start_submenu_callback(void* context, uint32_t index) {
    POCSAGPagerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void pocsag_pager_scene_start_on_enter(void* context) {
    UNUSED(context);
    POCSAGPagerApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Receive messages",
        SubmenuIndexPOCSAGPagerReceiver,
        pocsag_pager_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "About",
        SubmenuIndexPOCSAGPagerAbout,
        pocsag_pager_scene_start_submenu_callback,
        app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, POCSAGPagerSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, POCSAGPagerViewSubmenu);
}

bool pocsag_pager_scene_start_on_event(void* context, SceneManagerEvent event) {
    POCSAGPagerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexPOCSAGPagerAbout) {
            scene_manager_next_scene(app->scene_manager, POCSAGPagerSceneAbout);
            consumed = true;
        } else if(event.event == SubmenuIndexPOCSAGPagerReceiver) {
            scene_manager_next_scene(app->scene_manager, POCSAGPagerSceneReceiver);
            consumed = true;
        }
        scene_manager_set_scene_state(app->scene_manager, POCSAGPagerSceneStart, event.event);
    }

    return consumed;
}

void pocsag_pager_scene_start_on_exit(void* context) {
    POCSAGPagerApp* app = context;
    submenu_reset(app->submenu);
}
