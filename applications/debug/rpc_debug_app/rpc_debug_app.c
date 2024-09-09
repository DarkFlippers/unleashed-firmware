#include "rpc_debug_app.h"
#include <core/log.h>

#include <string.h>

static bool rpc_debug_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    RpcDebugApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool rpc_debug_app_back_event_callback(void* context) {
    furi_assert(context);
    RpcDebugApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void rpc_debug_app_tick_event_callback(void* context) {
    furi_assert(context);
    RpcDebugApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

static void
    rpc_debug_app_format_hex(const uint8_t* data, size_t data_size, char* buf, size_t buf_size) {
    if(data == NULL || data_size == 0) {
        strlcpy(buf, "<Data empty>", buf_size);
        return;
    }

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

static void rpc_debug_app_rpc_command_callback(const RpcAppSystemEvent* event, void* context) {
    furi_assert(context);
    RpcDebugApp* app = context;
    furi_assert(app->rpc);

    if(event->type == RpcAppEventTypeSessionClose) {
        scene_manager_stop(app->scene_manager);
        view_dispatcher_stop(app->view_dispatcher);
        rpc_system_app_set_callback(app->rpc, NULL, NULL);
        app->rpc = NULL;
    } else if(event->type == RpcAppEventTypeAppExit) {
        scene_manager_stop(app->scene_manager);
        view_dispatcher_stop(app->view_dispatcher);
        rpc_system_app_confirm(app->rpc, true);
    } else if(event->type == RpcAppEventTypeDataExchange) {
        furi_assert(event->data.type == RpcAppSystemEventDataTypeBytes);

        rpc_debug_app_format_hex(
            event->data.bytes.ptr, event->data.bytes.size, app->text_store, TEXT_STORE_SIZE);

        view_dispatcher_send_custom_event(
            app->view_dispatcher, RpcDebugAppCustomEventRpcDataExchange);
    } else {
        rpc_system_app_confirm(app->rpc, false);
    }
}

static bool rpc_debug_app_rpc_init_rpc(RpcDebugApp* app, const char* args) {
    bool ret = false;
    if(args && strlen(args)) {
        uint32_t rpc = 0;
        if(sscanf(args, "RPC %lX", &rpc) == 1) {
            app->rpc = (RpcAppSystem*)rpc;
            rpc_system_app_set_callback(app->rpc, rpc_debug_app_rpc_command_callback, app);
            rpc_system_app_send_started(app->rpc);
            ret = true;
        }
    }
    return ret;
}

static RpcDebugApp* rpc_debug_app_alloc(void) {
    RpcDebugApp* app = malloc(sizeof(RpcDebugApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->scene_manager = scene_manager_alloc(&rpc_debug_app_scene_handlers, app);
    app->view_dispatcher = view_dispatcher_alloc();

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, rpc_debug_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, rpc_debug_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, rpc_debug_app_tick_event_callback, 100);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, RpcDebugAppViewWidget, widget_get_view(app->widget));
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, RpcDebugAppViewSubmenu, submenu_get_view(app->submenu));
    app->text_box = text_box_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, RpcDebugAppViewTextBox, text_box_get_view(app->text_box));
    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, RpcDebugAppViewTextInput, text_input_get_view(app->text_input));
    app->byte_input = byte_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, RpcDebugAppViewByteInput, byte_input_get_view(app->byte_input));

    return app;
}

static void rpc_debug_app_free(RpcDebugApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, RpcDebugAppViewByteInput);
    view_dispatcher_remove_view(app->view_dispatcher, RpcDebugAppViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, RpcDebugAppViewTextBox);
    view_dispatcher_remove_view(app->view_dispatcher, RpcDebugAppViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, RpcDebugAppViewWidget);

    free(app->byte_input);
    free(app->text_input);
    free(app->text_box);
    free(app->submenu);
    free(app->widget);

    free(app->scene_manager);
    free(app->view_dispatcher);

    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;
    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    if(app->rpc) {
        rpc_system_app_set_callback(app->rpc, NULL, NULL);
        rpc_system_app_send_exited(app->rpc);
        app->rpc = NULL;
    }

    free(app);
}

int32_t rpc_debug_app(void* args) {
    RpcDebugApp* app = rpc_debug_app_alloc();

    if(rpc_debug_app_rpc_init_rpc(app, args)) {
        notification_message(app->notifications, &sequence_display_backlight_on);
        scene_manager_next_scene(app->scene_manager, RpcDebugAppSceneStart);
    } else {
        scene_manager_next_scene(app->scene_manager, RpcDebugAppSceneStartDummy);
    }

    view_dispatcher_run(app->view_dispatcher);

    rpc_debug_app_free(app);
    return 0;
}
