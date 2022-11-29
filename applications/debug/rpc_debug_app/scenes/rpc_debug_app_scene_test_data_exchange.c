#include "../rpc_debug_app.h"

typedef enum {
    SubmenuIndexSendData,
    SubmenuIndexReceiveData,
} SubmenuIndex;

static void
    rpc_debug_app_scene_test_data_exchange_submenu_callback(void* context, uint32_t index) {
    RpcDebugApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void rpc_debug_app_scene_test_data_exchange_on_enter(void* context) {
    RpcDebugApp* app = context;
    Submenu* submenu = app->submenu;

    submenu_add_item(
        submenu,
        "Send Data",
        SubmenuIndexSendData,
        rpc_debug_app_scene_test_data_exchange_submenu_callback,
        app);
    submenu_add_item(
        submenu,
        "Receive Data",
        SubmenuIndexReceiveData,
        rpc_debug_app_scene_test_data_exchange_submenu_callback,
        app);

    submenu_set_selected_item(submenu, SubmenuIndexSendData);
    view_dispatcher_switch_to_view(app->view_dispatcher, RpcDebugAppViewSubmenu);
}

bool rpc_debug_app_scene_test_data_exchange_on_event(void* context, SceneManagerEvent event) {
    RpcDebugApp* app = context;
    SceneManager* scene_manager = app->scene_manager;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const uint32_t submenu_index = event.event;
        if(submenu_index == SubmenuIndexSendData) {
            scene_manager_next_scene(scene_manager, RpcDebugAppSceneInputDataExchange);
            consumed = true;
        } else if(submenu_index == SubmenuIndexReceiveData) {
            scene_manager_next_scene(scene_manager, RpcDebugAppSceneReceiveDataExchange);
            consumed = true;
        }
    }

    return consumed;
}

void rpc_debug_app_scene_test_data_exchange_on_exit(void* context) {
    RpcDebugApp* app = context;
    submenu_reset(app->submenu);
}
