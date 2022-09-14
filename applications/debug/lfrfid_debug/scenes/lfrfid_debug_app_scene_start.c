#include "../lfrfid_debug_i.h"

typedef enum {
    SubmenuIndexTune,
} SubmenuIndex;

static void lfrfid_debug_scene_start_submenu_callback(void* context, uint32_t index) {
    LfRfidDebug* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void lfrfid_debug_scene_start_on_enter(void* context) {
    LfRfidDebug* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu, "Tune", SubmenuIndexTune, lfrfid_debug_scene_start_submenu_callback, app);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(app->scene_manager, LfRfidDebugSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidDebugViewSubmenu);
}

bool lfrfid_debug_scene_start_on_event(void* context, SceneManagerEvent event) {
    LfRfidDebug* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexTune) {
            scene_manager_next_scene(app->scene_manager, LfRfidDebugSceneTune);
            consumed = true;
        }
    }

    return consumed;
}

void lfrfid_debug_scene_start_on_exit(void* context) {
    LfRfidDebug* app = context;

    submenu_reset(app->submenu);
}
