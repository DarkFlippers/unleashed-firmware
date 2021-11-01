#include "flipper.pb.h"
#include "rpc_i.h"
#include "gui.pb.h"
#include <gui/gui_i.h>

typedef struct {
    Rpc* rpc;
    Gui* gui;
} RpcGuiSystem;

void rpc_system_gui_screen_frame_callback(uint8_t* data, size_t size, void* context) {
    furi_assert(data);
    furi_assert(size == 1024);
    furi_assert(context);

    RpcGuiSystem* rpc_gui = context;

    PB_Main* frame = furi_alloc(sizeof(PB_Main));

    frame->which_content = PB_Main_gui_screen_stream_frame_tag;
    frame->command_status = PB_CommandStatus_OK;
    frame->content.gui_screen_stream_frame.data = furi_alloc(PB_BYTES_ARRAY_T_ALLOCSIZE(size));
    uint8_t* buffer = frame->content.gui_screen_stream_frame.data->bytes;
    uint16_t* frame_size_msg = &frame->content.gui_screen_stream_frame.data->size;
    *frame_size_msg = size;
    memcpy(buffer, data, size);

    rpc_send_and_release(rpc_gui->rpc, frame);

    free(frame);
}

void rpc_system_gui_start_screen_stream_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    RpcGuiSystem* rpc_gui = context;

    rpc_send_and_release_empty(rpc_gui->rpc, request->command_id, PB_CommandStatus_OK);

    gui_set_framebuffer_callback(rpc_gui->gui, rpc_system_gui_screen_frame_callback, context);
}

void rpc_system_gui_stop_screen_stream_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    RpcGuiSystem* rpc_gui = context;

    rpc_send_and_release_empty(rpc_gui->rpc, request->command_id, PB_CommandStatus_OK);

    gui_set_framebuffer_callback(rpc_gui->gui, NULL, NULL);
}

void rpc_system_gui_send_input_event_request_process(const PB_Main* request, void* context) {
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

    return rpc_gui;
}

void rpc_system_gui_free(void* ctx) {
    furi_assert(ctx);
    RpcGuiSystem* rpc_gui = ctx;
    furi_assert(rpc_gui->gui);
    gui_set_framebuffer_callback(rpc_gui->gui, NULL, NULL);
    furi_record_close("gui");
    free(rpc_gui);
}