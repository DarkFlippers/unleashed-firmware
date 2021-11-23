#include "flipper.pb.h"
#include "rpc_i.h"
#include "status.pb.h"

void rpc_system_status_ping_process(const PB_Main* msg_request, void* context) {
    if(msg_request->has_next) {
        rpc_send_and_release_empty(
            context, msg_request->command_id, PB_CommandStatus_ERROR_INVALID_PARAMETERS);
        return;
    }

    PB_Main msg_response = PB_Main_init_default;
    msg_response.has_next = false;
    msg_response.command_status = PB_CommandStatus_OK;
    msg_response.command_id = msg_request->command_id;
    msg_response.which_content = PB_Main_ping_response_tag;

    const PB_Status_PingRequest* request = &msg_request->content.ping_request;
    PB_Status_PingResponse* response = &msg_response.content.ping_response;
    if(request->data && (request->data->size > 0)) {
        response->data = furi_alloc(PB_BYTES_ARRAY_T_ALLOCSIZE(request->data->size));
        memcpy(response->data->bytes, request->data->bytes, request->data->size);
        response->data->size = request->data->size;
    }

    rpc_send_and_release(context, &msg_response);
}

void* rpc_system_status_alloc(Rpc* rpc) {
    RpcHandler rpc_handler = {
        .message_handler = rpc_system_status_ping_process,
        .decode_submessage = NULL,
        .context = rpc,
    };

    rpc_add_handler(rpc, PB_Main_ping_request_tag, &rpc_handler);

    return NULL;
}
