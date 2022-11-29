#include "../rpc_debug_app.h"

static void rpc_debug_app_scene_start_format_hex(
    const uint8_t* data,
    size_t data_size,
    char* buf,
    size_t buf_size) {
    furi_assert(data);
    furi_assert(buf);

    const size_t byte_width = 3;
    const size_t line_width = 7;

    data_size = MIN(data_size, buf_size / (byte_width + 1));

    for(size_t i = 0; i < data_size; ++i) {
        char* p = buf + (i * byte_width);
        char sep = !((i + 1) % line_width) ? '\n' : ' ';
        snprintf(p, byte_width + 1, "%02X%c", data[i], sep);
    }

    buf[buf_size - 1] = '\0';
}

static void rpc_debug_app_scene_receive_data_exchange_callback(
    const uint8_t* data,
    size_t data_size,
    void* context) {
    RpcDebugApp* app = context;
    if(data) {
        rpc_debug_app_scene_start_format_hex(data, data_size, app->text_store, TEXT_STORE_SIZE);
    } else {
        strncpy(app->text_store, "<Data empty>", TEXT_STORE_SIZE);
    }
    view_dispatcher_send_custom_event(app->view_dispatcher, RpcDebugAppCustomEventRpcDataExchange);
}

void rpc_debug_app_scene_receive_data_exchange_on_enter(void* context) {
    RpcDebugApp* app = context;
    strncpy(app->text_store, "Received data will appear here...", TEXT_STORE_SIZE);

    text_box_set_text(app->text_box, app->text_store);
    text_box_set_font(app->text_box, TextBoxFontHex);

    rpc_system_app_set_data_exchange_callback(
        app->rpc, rpc_debug_app_scene_receive_data_exchange_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, RpcDebugAppViewTextBox);
}

bool rpc_debug_app_scene_receive_data_exchange_on_event(void* context, SceneManagerEvent event) {
    RpcDebugApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == RpcDebugAppCustomEventRpcDataExchange) {
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
    rpc_system_app_set_data_exchange_callback(app->rpc, NULL, NULL);
}
