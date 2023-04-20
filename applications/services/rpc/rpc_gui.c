#include "flipper.pb.h"
#include "rpc_i.h"
#include "gui.pb.h"
#include <gui/gui_i.h>
#include <assets_icons.h>

#define TAG "RpcGui"

typedef enum {
    RpcGuiWorkerFlagTransmit = (1 << 0),
    RpcGuiWorkerFlagExit = (1 << 1),
} RpcGuiWorkerFlag;

#define RpcGuiWorkerFlagAny (RpcGuiWorkerFlagTransmit | RpcGuiWorkerFlagExit)

#define RPC_GUI_INPUT_RESET (0u)

typedef struct {
    RpcSession* session;
    Gui* gui;

    // Receive part
    ViewPort* virtual_display_view_port;
    uint8_t* virtual_display_buffer;

    // Transmit
    PB_Main* transmit_frame;
    FuriThread* transmit_thread;

    bool virtual_display_not_empty;
    bool is_streaming;

    uint32_t input_key_counter[InputKeyMAX];
    uint32_t input_counter;

    ViewPort* rpc_session_active_viewport;
} RpcGuiSystem;

static const PB_Gui_ScreenOrientation rpc_system_gui_screen_orientation_map[] = {
    [CanvasOrientationHorizontal] = PB_Gui_ScreenOrientation_HORIZONTAL,
    [CanvasOrientationHorizontalFlip] = PB_Gui_ScreenOrientation_HORIZONTAL_FLIP,
    [CanvasOrientationVertical] = PB_Gui_ScreenOrientation_VERTICAL,
    [CanvasOrientationVerticalFlip] = PB_Gui_ScreenOrientation_VERTICAL_FLIP,
};

static void rpc_system_gui_screen_stream_frame_callback(
    uint8_t* data,
    size_t size,
    CanvasOrientation orientation,
    void* context) {
    furi_assert(data);
    furi_assert(context);

    RpcGuiSystem* rpc_gui = (RpcGuiSystem*)context;
    uint8_t* buffer = rpc_gui->transmit_frame->content.gui_screen_frame.data->bytes;

    furi_assert(size == rpc_gui->transmit_frame->content.gui_screen_frame.data->size);

    memcpy(buffer, data, size);
    rpc_gui->transmit_frame->content.gui_screen_frame.orientation =
        rpc_system_gui_screen_orientation_map[orientation];

    furi_thread_flags_set(furi_thread_get_id(rpc_gui->transmit_thread), RpcGuiWorkerFlagTransmit);
}

static int32_t rpc_system_gui_screen_stream_frame_transmit_thread(void* context) {
    furi_assert(context);

    RpcGuiSystem* rpc_gui = (RpcGuiSystem*)context;

    uint32_t transmit_time = 0;
    while(true) {
        uint32_t flags =
            furi_thread_flags_wait(RpcGuiWorkerFlagAny, FuriFlagWaitAny, FuriWaitForever);

        if(flags & RpcGuiWorkerFlagTransmit) {
            transmit_time = furi_get_tick();
            rpc_send(rpc_gui->session, rpc_gui->transmit_frame);
            transmit_time = furi_get_tick() - transmit_time;

            // Guaranteed bandwidth reserve
            uint32_t extra_delay = transmit_time / 20;
            if(extra_delay > 500) extra_delay = 500;
            if(extra_delay) furi_delay_tick(extra_delay);
        }

        if(flags & RpcGuiWorkerFlagExit) {
            break;
        }
    }

    return 0;
}

static void rpc_system_gui_start_screen_stream_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    FURI_LOG_D(TAG, "StartScreenStream");

    RpcGuiSystem* rpc_gui = context;
    RpcSession* session = rpc_gui->session;
    furi_assert(session);

    if(rpc_gui->is_streaming) {
        rpc_send_and_release_empty(
            session, request->command_id, PB_CommandStatus_ERROR_VIRTUAL_DISPLAY_ALREADY_STARTED);
    } else {
        rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_OK);

        rpc_gui->is_streaming = true;
        size_t framebuffer_size = gui_get_framebuffer_size(rpc_gui->gui);
        // Reusable Frame
        rpc_gui->transmit_frame = malloc(sizeof(PB_Main));
        rpc_gui->transmit_frame->which_content = PB_Main_gui_screen_frame_tag;
        rpc_gui->transmit_frame->command_status = PB_CommandStatus_OK;
        rpc_gui->transmit_frame->content.gui_screen_frame.data =
            malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(framebuffer_size));
        rpc_gui->transmit_frame->content.gui_screen_frame.data->size = framebuffer_size;
        // Transmission thread for async TX
        rpc_gui->transmit_thread = furi_thread_alloc_ex(
            "GuiRpcWorker", 1024, rpc_system_gui_screen_stream_frame_transmit_thread, rpc_gui);
        furi_thread_start(rpc_gui->transmit_thread);
        // GUI framebuffer callback
        gui_add_framebuffer_callback(
            rpc_gui->gui, rpc_system_gui_screen_stream_frame_callback, context);
    }
}

static void rpc_system_gui_stop_screen_stream_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    FURI_LOG_D(TAG, "StopScreenStream");

    RpcGuiSystem* rpc_gui = context;
    RpcSession* session = rpc_gui->session;
    furi_assert(session);

    if(rpc_gui->is_streaming) {
        rpc_gui->is_streaming = false;
        // Remove GUI framebuffer callback
        gui_remove_framebuffer_callback(
            rpc_gui->gui, rpc_system_gui_screen_stream_frame_callback, context);
        // Stop and release worker thread
        furi_thread_flags_set(furi_thread_get_id(rpc_gui->transmit_thread), RpcGuiWorkerFlagExit);
        furi_thread_join(rpc_gui->transmit_thread);
        furi_thread_free(rpc_gui->transmit_thread);
        // Release frame
        pb_release(&PB_Main_msg, rpc_gui->transmit_frame);
        free(rpc_gui->transmit_frame);
        rpc_gui->transmit_frame = NULL;
    }

    rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_OK);
}

static void
    rpc_system_gui_send_input_event_request_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_gui_send_input_event_request_tag);
    furi_assert(context);

    FURI_LOG_D(TAG, "SendInputEvent");

    RpcGuiSystem* rpc_gui = context;
    RpcSession* session = rpc_gui->session;
    furi_assert(session);

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
            session, request->command_id, PB_CommandStatus_ERROR_INVALID_PARAMETERS);
        return;
    }

    // Event sequence shenanigans
    event.sequence_source = INPUT_SEQUENCE_SOURCE_SOFTWARE;
    if(event.type == InputTypePress) {
        rpc_gui->input_counter++;
        if(rpc_gui->input_counter == RPC_GUI_INPUT_RESET) rpc_gui->input_counter++;
        rpc_gui->input_key_counter[event.key] = rpc_gui->input_counter;
    }
    if(rpc_gui->input_key_counter[event.key] == RPC_GUI_INPUT_RESET) {
        FURI_LOG_W(TAG, "Out of sequence input event: key %d, type %d,", event.key, event.type);
    }
    event.sequence_counter = rpc_gui->input_key_counter[event.key];
    if(event.type == InputTypeRelease) {
        rpc_gui->input_key_counter[event.key] = RPC_GUI_INPUT_RESET;
    }

    // Submit event
    FuriPubSub* input_events = furi_record_open(RECORD_INPUT_EVENTS);
    furi_check(input_events);
    furi_pubsub_publish(input_events, &event);
    furi_record_close(RECORD_INPUT_EVENTS);
    rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_OK);
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

    FURI_LOG_D(TAG, "StartVirtualDisplay");

    RpcGuiSystem* rpc_gui = context;
    RpcSession* session = rpc_gui->session;
    furi_assert(session);

    if(rpc_gui->virtual_display_view_port) {
        rpc_send_and_release_empty(
            session, request->command_id, PB_CommandStatus_ERROR_VIRTUAL_DISPLAY_ALREADY_STARTED);
        return;
    }

    // TODO: consider refactoring
    // Using display framebuffer size as an XBM buffer size is like comparing apples and oranges
    // Glad they both are 1024 for now
    size_t buffer_size = canvas_get_buffer_size(rpc_gui->gui->canvas);
    rpc_gui->virtual_display_buffer = malloc(buffer_size);

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

    rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_OK);
}

static void rpc_system_gui_stop_virtual_display_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    FURI_LOG_D(TAG, "StopVirtualDisplay");

    RpcGuiSystem* rpc_gui = context;
    RpcSession* session = rpc_gui->session;
    furi_assert(session);

    if(!rpc_gui->virtual_display_view_port) {
        rpc_send_and_release_empty(
            session, request->command_id, PB_CommandStatus_ERROR_VIRTUAL_DISPLAY_NOT_STARTED);
        return;
    }

    gui_remove_view_port(rpc_gui->gui, rpc_gui->virtual_display_view_port);
    view_port_free(rpc_gui->virtual_display_view_port);
    free(rpc_gui->virtual_display_buffer);
    rpc_gui->virtual_display_view_port = NULL;
    rpc_gui->virtual_display_not_empty = false;

    rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_OK);
}

static void rpc_system_gui_virtual_display_frame_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    FURI_LOG_D(TAG, "VirtualDisplayFrame");

    RpcGuiSystem* rpc_gui = context;
    RpcSession* session = rpc_gui->session;
    furi_assert(session);

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

    (void)session;
}

static void rpc_active_session_icon_draw_callback(Canvas* canvas, void* context) {
    UNUSED(context);
    furi_assert(canvas);
    canvas_draw_icon(canvas, 0, 0, &I_Rpc_active_7x8);
}

void* rpc_system_gui_alloc(RpcSession* session) {
    furi_assert(session);

    RpcGuiSystem* rpc_gui = malloc(sizeof(RpcGuiSystem));
    rpc_gui->gui = furi_record_open(RECORD_GUI);
    rpc_gui->session = session;

    // Active session icon
    rpc_gui->rpc_session_active_viewport = view_port_alloc();
    view_port_set_width(rpc_gui->rpc_session_active_viewport, icon_get_width(&I_Rpc_active_7x8));
    view_port_draw_callback_set(
        rpc_gui->rpc_session_active_viewport, rpc_active_session_icon_draw_callback, session);
    if(rpc_session_get_owner(rpc_gui->session) != RpcOwnerBle) {
        view_port_enabled_set(rpc_gui->rpc_session_active_viewport, true);
    } else {
        view_port_enabled_set(rpc_gui->rpc_session_active_viewport, false);
    }
    gui_add_view_port(rpc_gui->gui, rpc_gui->rpc_session_active_viewport, GuiLayerStatusBarLeft);

    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = rpc_gui,
    };

    rpc_handler.message_handler = rpc_system_gui_start_screen_stream_process;
    rpc_add_handler(session, PB_Main_gui_start_screen_stream_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_gui_stop_screen_stream_process;
    rpc_add_handler(session, PB_Main_gui_stop_screen_stream_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_gui_send_input_event_request_process;
    rpc_add_handler(session, PB_Main_gui_send_input_event_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_gui_start_virtual_display_process;
    rpc_add_handler(session, PB_Main_gui_start_virtual_display_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_gui_stop_virtual_display_process;
    rpc_add_handler(session, PB_Main_gui_stop_virtual_display_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_gui_virtual_display_frame_process;
    rpc_add_handler(session, PB_Main_gui_screen_frame_tag, &rpc_handler);

    return rpc_gui;
}

void rpc_system_gui_free(void* context) {
    furi_assert(context);
    RpcGuiSystem* rpc_gui = context;
    furi_assert(rpc_gui->gui);

    if(rpc_gui->virtual_display_view_port) {
        gui_remove_view_port(rpc_gui->gui, rpc_gui->virtual_display_view_port);
        view_port_free(rpc_gui->virtual_display_view_port);
        free(rpc_gui->virtual_display_buffer);
        rpc_gui->virtual_display_view_port = NULL;
        rpc_gui->virtual_display_not_empty = false;
    }

    gui_remove_view_port(rpc_gui->gui, rpc_gui->rpc_session_active_viewport);
    view_port_free(rpc_gui->rpc_session_active_viewport);

    if(rpc_gui->is_streaming) {
        rpc_gui->is_streaming = false;
        // Remove GUI framebuffer callback
        gui_remove_framebuffer_callback(
            rpc_gui->gui, rpc_system_gui_screen_stream_frame_callback, context);
        // Stop and release worker thread
        furi_thread_flags_set(furi_thread_get_id(rpc_gui->transmit_thread), RpcGuiWorkerFlagExit);
        furi_thread_join(rpc_gui->transmit_thread);
        furi_thread_free(rpc_gui->transmit_thread);
        // Release frame
        pb_release(&PB_Main_msg, rpc_gui->transmit_frame);
        free(rpc_gui->transmit_frame);
        rpc_gui->transmit_frame = NULL;
    }
    furi_record_close(RECORD_GUI);
    free(rpc_gui);
}