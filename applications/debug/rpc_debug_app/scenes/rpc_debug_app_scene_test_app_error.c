#include "../rpc_debug_app.h"

typedef enum {
    SubmenuIndexSetErrorCode,
    SubmenuIndexSetErrorText,
} SubmenuIndex;

static void rpc_debug_app_scene_test_app_error_submenu_callback(void* context, uint32_t index) {
    RpcDebugApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void rpc_debug_app_scene_test_app_error_on_enter(void* context) {
    RpcDebugApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Set Error Code",
        SubmenuIndexSetErrorCode,
        rpc_debug_app_scene_test_app_error_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Set Error Text",
        SubmenuIndexSetErrorText,
        rpc_debug_app_scene_test_app_error_submenu_callback,
        app);

    submenu_set_selected_item(submenu, SubmenuIndexSetErrorCode);
    view_dispatcher_switch_to_view(app->view_dispatcher, RpcDebugAppViewSubmenu);
}

bool rpc_debug_app_scene_test_app_error_on_event(void* context, SceneManagerEvent event) {
    RpcDebugApp* app = context;
    SceneManager* scene_manager = app->scene_manager;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const uint32_t submenu_index = event.event;
        if(submenu_index == SubmenuIndexSetErrorCode) {
            scene_manager_next_scene(scene_manager, RpcDebugAppSceneInputErrorCode);
            consumed = true;
        } else if(submenu_index == SubmenuIndexSetErrorText) {
            scene_manager_next_scene(scene_manager, RpcDebugAppSceneInputErrorText);
            consumed = true;
        }
    }

    return consumed;
}

void rpc_debug_app_scene_test_app_error_on_exit(void* context) {
    RpcDebugApp* app = context;
    submenu_reset(app->submenu);
}
