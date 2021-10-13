#include "flipper.pb.h"
#include "furi/record.h"
#include "status.pb.h"
#include "rpc_i.h"
#include <furi.h>
#include <loader/loader.h>

void rpc_system_app_start_process(const PB_Main* request, void* context) {
    Rpc* rpc = context;
    furi_assert(rpc);
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_app_start_tag);
    PB_CommandStatus result = PB_CommandStatus_ERROR_APP_CANT_START;

    Loader* loader = furi_record_open("loader");
    const char* app_name = request->content.app_start.name;
    if(app_name) {
        const char* app_args = request->content.app_start.args;
        LoaderStatus status = loader_start(loader, app_name, app_args);
        if(status == LoaderStatusErrorAppStarted) {
            result = PB_CommandStatus_ERROR_APP_SYSTEM_LOCKED;
        } else if(status == LoaderStatusErrorInternal) {
            result = PB_CommandStatus_ERROR_APP_CANT_START;
        } else if(status == LoaderStatusErrorUnknownApp) {
            result = PB_CommandStatus_ERROR_INVALID_PARAMETERS;
        } else if(status == LoaderStatusOk) {
            result = PB_CommandStatus_OK;
        } else {
            furi_assert(0);
        }
    } else {
        result = PB_CommandStatus_ERROR_INVALID_PARAMETERS;
    }

    furi_record_close("loader");

    rpc_encode_and_send_empty(rpc, request->command_id, result);
}

void rpc_system_app_lock_status_process(const PB_Main* request, void* context) {
    Rpc* rpc = context;
    furi_assert(rpc);
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_app_lock_status_request_tag);

    Loader* loader = furi_record_open("loader");

    PB_Main response = {
        .has_next = false,
        .command_status = PB_CommandStatus_OK,
        .command_id = request->command_id,
        .which_content = PB_Main_app_lock_status_response_tag,
    };

    response.content.app_lock_status_response.locked = loader_is_locked(loader);

    furi_record_close("loader");

    rpc_encode_and_send(rpc, &response);
}

void* rpc_system_app_alloc(Rpc* rpc) {
    furi_assert(rpc);

    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = rpc,
    };

    rpc_handler.message_handler = rpc_system_app_start_process;
    rpc_add_handler(rpc, PB_Main_app_start_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_app_lock_status_process;
    rpc_add_handler(rpc, PB_Main_app_lock_status_request_tag, &rpc_handler);

    return NULL;
}
