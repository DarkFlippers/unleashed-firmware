#include "../rpc_debug_app.h"

#include <lib/toolbox/strint.h>

static bool rpc_debug_app_scene_input_error_code_validator_callback(
    const char* text,
    FuriString* error,
    void* context) {
    UNUSED(context);

    for(; *text; ++text) {
        const char c = *text;
        if(c < '0' || c > '9') {
            furi_string_printf(error, "%s", "Please enter\na number!");
            return false;
        }
    }

    return true;
}

static void rpc_debug_app_scene_input_error_code_result_callback(void* context) {
    RpcDebugApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, RpcDebugAppCustomEventInputErrorCode);
}

void rpc_debug_app_scene_input_error_code_on_enter(void* context) {
    RpcDebugApp* app = context;
    strlcpy(app->text_store, "666", TEXT_STORE_SIZE);
    text_input_set_header_text(app->text_input, "Enter error code");
    text_input_set_validator(
        app->text_input, rpc_debug_app_scene_input_error_code_validator_callback, NULL);
    text_input_set_result_callback(
        app->text_input,
        rpc_debug_app_scene_input_error_code_result_callback,
        app,
        app->text_store,
        TEXT_STORE_SIZE,
        true);
    view_dispatcher_switch_to_view(app->view_dispatcher, RpcDebugAppViewTextInput);
}

bool rpc_debug_app_scene_input_error_code_on_event(void* context, SceneManagerEvent event) {
    RpcDebugApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == RpcDebugAppCustomEventInputErrorCode) {
            uint32_t error_code;
            if(strint_to_uint32(app->text_store, NULL, &error_code, 10) == StrintParseNoError) {
                rpc_system_app_set_error_code(app->rpc, error_code);
            }
            scene_manager_previous_scene(app->scene_manager);
            consumed = true;
        }
    }

    return consumed;
}

void rpc_debug_app_scene_input_error_code_on_exit(void* context) {
    RpcDebugApp* app = context;
    text_input_reset(app->text_input);
    text_input_set_validator(app->text_input, NULL, NULL);
}
