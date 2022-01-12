#include "flipper.pb.h"
#include "rpc_i.h"
#include "gui.pb.h"
#include <gui/gui_i.h>

#define TAG "RpcGui"

typedef struct {
    Rpc* rpc;
    Gui* gui;
    ViewPort* virtual_display_view_port;
    uint8_t* virtual_display_buffer;
    bool virtual_display_not_empty;
} RpcGuiSystem;

static void
    rpc_system_gui_screen_stream_frame_callback(uint8_t* data, size_t size, void* context) {
    furi_assert(data);
    furi_assert(size == 1024);
    furi_assert(context);

    RpcGuiSystem* rpc_gui = context;

    PB_Main* frame = furi_alloc(sizeof(PB_Main));

    frame->which_content = PB_Main_gui_screen_frame_tag;
    frame->command_status = PB_CommandStatus_OK;
    frame->content.gui_screen_frame.data = furi_alloc(PB_BYTES_ARRAY_T_ALLOCSIZE(size));
    uint8_t* buffer = frame->content.gui_screen_frame.data->bytes;
    uint16_t* frame_size_msg = &frame->content.gui_screen_frame.data->size;
    *frame_size_msg = size;
    memcpy(buffer, data, size);

    rpc_send_and_release(rpc_gui->rpc, frame);

    free(frame);
}

static void rpc_system_gui_start_screen_stream_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    RpcGuiSystem* rpc_gui = context;

    rpc_send_and_release_empty(rpc_gui->rpc, request->command_id, PB_CommandStatus_OK);

    gui_set_framebuffer_callback(
        rpc_gui->gui, rpc_system_gui_screen_stream_frame_callback, context);
}

static void rpc_system_gui_stop_screen_stream_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    RpcGuiSystem* rpc_gui = context;

    gui_set_framebuffer_callback(rpc_gui->gui, NULL, NULL);

    rpc_send_and_release_empty(rpc_gui->rpc, request->command_id, PB_CommandStatus_OK);
}

static void
    rpc_system_gui_send_input_event_request_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_gui_send_input_event_request_tag);
    furi_assert(context);
    RpcGuiSystem* rpc_gui = context;

    InputEvent event;

    bool invalid = false;

    switch(request->content.gui_send_input_event_request.key) {
    case PB_Gui_InputKey_UP:
        event.key = InputKeyUp;
        break;
    case PB_Gui_InputKey_DOWN:
        event.key = InputKeyDown;
        break;
    case PB_Gui_InputKey_RIGHT:
        event.key = InputKeyRight;
        break;
    case PB_Gui_InputKey_LEFT:
        event.key = InputKeyLeft;
        break;
    case PB_Gui_InputKey_OK:
        event.key = InputKeyOk;
        break;
    case PB_Gui_InputKey_BACK:
        event.key = InputKeyBack;
        break;
    default:
        // Invalid key
        invalid = true;
        break;
    }

    switch(request->content.gui_send_input_event_request.type) {
    case PB_Gui_InputType_PRESS:
        event.type = InputTypePress;
        break;
    case PB_Gui_InputType_RELEASE:
        event.type = InputTypeRelease;
        break;
    case PB_Gui_InputType_SHORT:
        event.type = InputTypeShort;
        break;
    case PB_Gui_InputType_LONG:
        event.type = InputTypeLong;
        break;
    case PB_Gui_InputType_REPEAT:
        event.type = InputTypeRepeat;
        break;
    default:
        // Invalid type
        invalid = true;
        break;
    }

    if(invalid) {
        rpc_send_and_release_empty(
            rpc_gui->rpc, request->command_id, PB_CommandStatus_ERROR_INVALID_PARAMETERS);
        return;
    }

    FuriPubSub* input_events = furi_record_open("input_events");
    furi_check(input_events);
    furi_pubsub_publish(input_events, &event);
    furi_record_close("input_events");
    rpc_send_and_release_empty(rpc_gui->rpc, request->command_id, PB_CommandStatus_OK);
}

static void rpc_system_gui_virtual_display_render_callback(Canvas* canvas, void* context) {
    furi_assert(canvas);
    furi_assert(context);
    RpcGuiSystem* rpc_gui = context;

    if(!rpc_gui->virtual_display_not_empty) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignCenter, "Virtual Display");
        canvas_draw_str_aligned(canvas, 64, 36, AlignCenter, AlignCenter, "Waiting for frames...");
        return;
    }

    canvas_draw_xbm(canvas, 0, 0, canvas->width, canvas->height, rpc_gui->virtual_display_buffer);
}

static void rpc_system_gui_start_virtual_display_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    RpcGuiSystem* rpc_gui = context;

    if(rpc_gui->virtual_display_view_port) {
        rpc_send_and_release_empty(
            rpc_gui->rpc,
            request->command_id,
            PB_CommandStatus_ERROR_VIRTUAL_DISPLAY_ALREADY_STARTED);
        return;
    }

    // TODO: consider refactoring
    // Using display framebuffer size as an XBM buffer size is like comparing apples and oranges
    // Glad they both are 1024 for now
    size_t buffer_size = canvas_get_buffer_size(rpc_gui->gui->canvas);
    rpc_gui->virtual_display_buffer = furi_alloc(buffer_size);

    if(request->content.gui_start_virtual_display_request.has_first_frame) {
        size_t buffer_size = canvas_get_buffer_size(rpc_gui->gui->canvas);
        memcpy(
            rpc_gui->virtual_display_buffer,
            request->content.gui_start_virtual_display_request.first_frame.data->bytes,
            buffer_size);
        rpc_gui->virtual_display_not_empty = true;
    }

    rpc_gui->virtual_display_view_port = view_port_alloc();
    view_port_draw_callback_set(
        rpc_gui->virtual_display_view_port,
        rpc_system_gui_virtual_display_render_callback,
        rpc_gui);
    gui_add_view_port(rpc_gui->gui, rpc_gui->virtual_display_view_port, GuiLayerFullscreen);

    rpc_send_and_release_empty(rpc_gui->rpc, request->command_id, PB_CommandStatus_OK);
}

static void rpc_system_gui_stop_virtual_display_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    RpcGuiSystem* rpc_gui = context;

    if(!rpc_gui->virtual_display_view_port) {
        rpc_send_and_release_empty(
            rpc_gui->rpc, request->command_id, PB_CommandStatus_ERROR_VIRTUAL_DISPLAY_NOT_STARTED);
        return;
    }

    gui_remove_view_port(rpc_gui->gui, rpc_gui->virtual_display_view_port);
    view_port_free(rpc_gui->virtual_display_view_port);
    free(rpc_gui->virtual_display_buffer);
    rpc_gui->virtual_display_view_port = NULL;
    rpc_gui->virtual_display_not_empty = false;

    rpc_send_and_release_empty(rpc_gui->rpc, request->command_id, PB_CommandStatus_OK);
}

static void rpc_system_gui_virtual_display_frame_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    RpcGuiSystem* rpc_gui = context;

    if(!rpc_gui->virtual_display_view_port) {
        FURI_LOG_W(TAG, "Virtual display is not started, ignoring incoming frame packet");
        return;
    }

    size_t buffer_size = canvas_get_buffer_size(rpc_gui->gui->canvas);
    memcpy(
        rpc_gui->virtual_display_buffer,
        request->content.gui_screen_frame.data->bytes,
        buffer_size);
    rpc_gui->virtual_display_not_empty = true;
    view_port_update(rpc_gui->virtual_display_view_port);
}

void* rpc_system_gui_alloc(Rpc* rpc) {
    furi_assert(rpc);

    RpcGuiSystem* rpc_gui = furi_alloc(sizeof(RpcGuiSystem));
    rpc_gui->gui = furi_record_open("gui");
    rpc_gui->rpc = rpc;

    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = rpc_gui,
    };

    rpc_handler.message_handler = rpc_system_gui_start_screen_stream_process;
    rpc_add_handler(rpc, PB_Main_gui_start_screen_stream_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_gui_stop_screen_stream_process;
    rpc_add_handler(rpc, PB_Main_gui_stop_screen_stream_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_gui_send_input_event_request_process;
    rpc_add_handler(rpc, PB_Main_gui_send_input_event_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_gui_start_virtual_display_process;
    rpc_add_handler(rpc, PB_Main_gui_start_virtual_display_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_gui_stop_virtual_display_process;
    rpc_add_handler(rpc, PB_Main_gui_stop_virtual_display_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_gui_virtual_display_frame_process;
    rpc_add_handler(rpc, PB_Main_gui_screen_frame_tag, &rpc_handler);

    return rpc_gui;
}

void rpc_system_gui_free(void* ctx) {
    furi_assert(ctx);
    RpcGuiSystem* rpc_gui = ctx;
    furi_assert(rpc_gui->gui);

    if(rpc_gui->virtual_display_view_port) {
        gui_remove_view_port(rpc_gui->gui, rpc_gui->virtual_display_view_port);
        view_port_free(rpc_gui->virtual_display_view_port);
        free(rpc_gui->virtual_display_buffer);
        rpc_gui->virtual_display_view_port = NULL;
        rpc_gui->virtual_display_not_empty = false;
    }

    gui_set_framebuffer_callback(rpc_gui->gui, NULL, NULL);
    furi_record_close("gui");
    free(rpc_gui);
}
