#include "../rpc_debug_app.h"

void rpc_debug_app_scene_start_dummy_on_enter(void* context) {
    RpcDebugApp* app = context;
    widget_add_text_box_element(
        app->widget,
        0,
        0,
        128,
        64,
        AlignCenter,
        AlignCenter,
        "This application\nis meant to be run\nin \e#RPC\e# mode.",
        false);
    view_dispatcher_switch_to_view(app->view_dispatcher, RpcDebugAppViewWidget);
}

bool rpc_debug_app_scene_start_dummy_on_event(void* context, SceneManagerEvent event) {
    RpcDebugApp* app = context;
    UNUSED(app);
    UNUSED(event);

    bool consumed = false;
    return consumed;
}

void rpc_debug_app_scene_start_dummy_on_exit(void* context) {
    RpcDebugApp* app = context;
    widget_reset(app->widget);
}
