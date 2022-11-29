#include "../rpc_debug_app.h"

static void rpc_debug_app_scene_input_data_exchange_result_callback(void* context) {
    RpcDebugApp* app = context;
    view_dispatcher_send_custom_event(
        app->view_dispatcher, RpcDebugAppCustomEventInputDataExchange);
}

void rpc_debug_app_scene_input_data_exchange_on_enter(void* context) {
    RpcDebugApp* app = context;
    byte_input_set_header_text(app->byte_input, "Enter data to exchange");
    byte_input_set_result_callback(
        app->byte_input,
        rpc_debug_app_scene_input_data_exchange_result_callback,
        NULL,
        app,
        app->data_store,
        DATA_STORE_SIZE);
    view_dispatcher_switch_to_view(app->view_dispatcher, RpcDebugAppViewByteInput);
}

bool rpc_debug_app_scene_input_data_exchange_on_event(void* context, SceneManagerEvent event) {
    RpcDebugApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == RpcDebugAppCustomEventInputDataExchange) {
            rpc_system_app_exchange_data(app->rpc, app->data_store, DATA_STORE_SIZE);
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
        }
    }

    return consumed;
}

void rpc_debug_app_scene_input_data_exchange_on_exit(void* context) {
    RpcDebugApp* app = context;
    UNUSED(app);
}
