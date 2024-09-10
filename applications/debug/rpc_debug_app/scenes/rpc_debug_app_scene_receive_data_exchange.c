#include "../rpc_debug_app.h"

void rpc_debug_app_scene_receive_data_exchange_on_enter(void* context) {
    RpcDebugApp* app = context;
    strlcpy(app->text_store, "Received data will appear here...", TEXT_STORE_SIZE);

    text_box_set_text(app->text_box, app->text_store);
    text_box_set_font(app->text_box, TextBoxFontHex);

    view_dispatcher_switch_to_view(app->view_dispatcher, RpcDebugAppViewTextBox);
}

bool rpc_debug_app_scene_receive_data_exchange_on_event(void* context, SceneManagerEvent event) {
    RpcDebugApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == RpcDebugAppCustomEventRpcDataExchange) {
            rpc_system_app_confirm(app->rpc, true);
            notification_message(app->notifications, &sequence_blink_cyan_100);
            notification_message(app->notifications, &sequence_display_backlight_on);
            text_box_set_text(app->text_box, app->text_store);
            consumed = true;
        }
    }

    return consumed;
}

void rpc_debug_app_scene_receive_data_exchange_on_exit(void* context) {
    RpcDebugApp* app = context;
    text_box_reset(app->text_box);
}
