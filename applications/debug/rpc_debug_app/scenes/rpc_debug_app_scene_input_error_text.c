#include "../rpc_debug_app.h"

static void rpc_debug_app_scene_input_error_text_result_callback(void* context) {
    RpcDebugApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, RpcDebugAppCustomEventInputErrorText);
}

void rpc_debug_app_scene_input_error_text_on_enter(void* context) {
    RpcDebugApp* app = context;
    strlcpy(app->text_store, "I'm a scary error message!", TEXT_STORE_SIZE);
    text_input_set_header_text(app->text_input, "Enter error text");
    text_input_set_result_callback(
        app->text_input,
        rpc_debug_app_scene_input_error_text_result_callback,
        app,
        app->text_store,
        TEXT_STORE_SIZE,
        true);
    view_dispatcher_switch_to_view(app->view_dispatcher, RpcDebugAppViewTextInput);
}

bool rpc_debug_app_scene_input_error_text_on_event(void* context, SceneManagerEvent event) {
    RpcDebugApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == RpcDebugAppCustomEventInputErrorText) {
            rpc_system_app_set_error_text(app->rpc, app->text_store);
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
        }
    }

    return consumed;
}

void rpc_debug_app_scene_input_error_text_on_exit(void* context) {
    RpcDebugApp* app = context;
    text_input_reset(app->text_input);
}
