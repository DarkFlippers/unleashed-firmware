#include "flipper.pb.h"
#include "rpc_i.h"
#include <desktop/desktop.h>
#include "desktop.pb.h"

#define TAG "RpcDesktop"

typedef struct {
    RpcSession* session;
    Desktop* desktop;
} RpcDesktop;

static void rpc_desktop_on_is_locked_request(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    furi_assert(request->which_content == PB_Main_desktop_is_locked_request_tag);

    FURI_LOG_D(TAG, "IsLockedRequest");
    RpcDesktop* rpc_desktop = context;
    RpcSession* session = rpc_desktop->session;

    PB_CommandStatus ret = desktop_api_is_locked(rpc_desktop->desktop) ? PB_CommandStatus_OK :
                                                                         PB_CommandStatus_ERROR;

    rpc_send_and_release_empty(session, request->command_id, ret);
}

static void rpc_desktop_on_unlock_request(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    furi_assert(request->which_content == PB_Main_desktop_unlock_request_tag);

    FURI_LOG_D(TAG, "UnlockRequest");
    RpcDesktop* rpc_desktop = context;
    RpcSession* session = rpc_desktop->session;

    desktop_api_unlock(rpc_desktop->desktop);

    rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_OK);
}

void* rpc_desktop_alloc(RpcSession* session) {
    furi_assert(session);

    RpcDesktop* rpc_desktop = malloc(sizeof(RpcDesktop));
    rpc_desktop->desktop = furi_record_open(RECORD_DESKTOP);
    rpc_desktop->session = session;

    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = rpc_desktop,
    };

    rpc_handler.message_handler = rpc_desktop_on_is_locked_request;
    rpc_add_handler(session, PB_Main_desktop_is_locked_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_desktop_on_unlock_request;
    rpc_add_handler(session, PB_Main_desktop_unlock_request_tag, &rpc_handler);

    return rpc_desktop;
}

void rpc_desktop_free(void* context) {
    furi_assert(context);
    RpcDesktop* rpc_desktop = context;

    furi_assert(rpc_desktop->desktop);
    furi_record_close(RECORD_DESKTOP);

    rpc_desktop->session = NULL;
    free(rpc_desktop);
}