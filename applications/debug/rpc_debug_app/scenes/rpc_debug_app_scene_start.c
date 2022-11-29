#include "../rpc_debug_app.h"

enum SubmenuIndex {
    SubmenuIndexTestAppError,
    SubmenuIndexTestDataExchange,
};

static void rpc_debug_app_scene_start_submenu_callback(void* context, uint32_t index) {
    RpcDebugApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void rpc_debug_app_scene_start_on_enter(void* context) {
    RpcDebugApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Test App Error",
        SubmenuIndexTestAppError,
        rpc_debug_app_scene_start_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Test Data Exchange",
        SubmenuIndexTestDataExchange,
        rpc_debug_app_scene_start_submenu_callback,
        app);

    submenu_set_selected_item(submenu, SubmenuIndexTestAppError);
    view_dispatcher_switch_to_view(app->view_dispatcher, RpcDebugAppViewSubmenu);
}

bool rpc_debug_app_scene_start_on_event(void* context, SceneManagerEvent event) {
    RpcDebugApp* app = context;
    SceneManager* scene_manager = app->scene_manager;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const uint32_t submenu_index = event.event;
        if(submenu_index == SubmenuIndexTestAppError) {
            scene_manager_next_scene(scene_manager, RpcDebugAppSceneTestAppError);
            consumed = true;
        } else if(submenu_index == SubmenuIndexTestDataExchange) {
            scene_manager_next_scene(scene_manager, RpcDebugAppSceneTestDataExchange);
            consumed = true;
        }
    }

    return consumed;
}

void rpc_debug_app_scene_start_on_exit(void* context) {
    RpcDebugApp* app = context;
    submenu_reset(app->submenu);
}
