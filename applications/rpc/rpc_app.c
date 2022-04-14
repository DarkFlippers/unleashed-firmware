#include "flipper.pb.h"
#include "furi/record.h"
#include "rpc_i.h"
#include <furi.h>
#include <loader/loader.h>

#define TAG "RpcSystemApp"

static void rpc_system_app_start_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_app_start_request_tag);
    RpcSession* session = (RpcSession*)context;
    furi_assert(session);

    FURI_LOG_D(TAG, "Start");

    PB_CommandStatus result = PB_CommandStatus_ERROR_APP_CANT_START;

    Loader* loader = furi_record_open("loader");
    const char* app_name = request->content.app_start_request.name;
    if(app_name) {
        const char* app_args = request->content.app_start_request.args;
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

    rpc_send_and_release_empty(session, request->command_id, result);
}

static void rpc_system_app_lock_status_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_app_lock_status_request_tag);
    RpcSession* session = (RpcSession*)context;
    furi_assert(session);

    FURI_LOG_D(TAG, "LockStatus");

    Loader* loader = furi_record_open("loader");

    PB_Main response = {
        .has_next = false,
        .command_status = PB_CommandStatus_OK,
        .command_id = request->command_id,
        .which_content = PB_Main_app_lock_status_response_tag,
    };

    response.content.app_lock_status_response.locked = loader_is_locked(loader);

    furi_record_close("loader");

    rpc_send_and_release(session, &response);
    pb_release(&PB_Main_msg, &response);
}

void* rpc_system_app_alloc(RpcSession* session) {
    furi_assert(session);

    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = session,
    };

    rpc_handler.message_handler = rpc_system_app_start_process;
    rpc_add_handler(session, PB_Main_app_start_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_app_lock_status_process;
    rpc_add_handler(session, PB_Main_app_lock_status_request_tag, &rpc_handler);

    return NULL;
}
