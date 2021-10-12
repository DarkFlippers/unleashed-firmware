#include "flipper.pb.h"
#include "rpc_i.h"
#include "status.pb.h"

void rpc_system_status_ping_process(const PB_Main* msg_request, void* context) {
    PB_Main msg_response = PB_Main_init_default;
    msg_response.has_next = false;
    msg_response.command_status = PB_CommandStatus_OK;
    msg_response.command_id = msg_request->command_id;
    msg_response.which_content = PB_Main_ping_response_tag;

    rpc_encode_and_send(context, &msg_response);
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
