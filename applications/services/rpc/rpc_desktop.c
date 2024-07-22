#include "flipper.pb.h"
#include "rpc_i.h"
#include <desktop/desktop.h>

#define TAG "RpcDesktop"

typedef struct {
    RpcSession* session;
    Desktop* desktop;
    FuriPubSub* status_pubsub;
    FuriPubSubSubscription* status_subscription;
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

static void rpc_desktop_on_desktop_pubsub(const void* message, void* context) {
    RpcDesktop* rpc_desktop = context;
    RpcSession* session = rpc_desktop->session;
    const DesktopStatus* status = message;

    PB_Main rpc_message = {
        .command_id = 0,
        .command_status = PB_CommandStatus_OK,
        .has_next = false,
        .which_content = PB_Main_desktop_status_tag,
        .content.desktop_status.locked = status->locked,
    };
    rpc_send_and_release(session, &rpc_message);
}

static void rpc_desktop_on_status_subscribe_request(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    furi_assert(request->which_content == PB_Main_desktop_status_subscribe_request_tag);

    FURI_LOG_D(TAG, "StatusSubscribeRequest");
    RpcDesktop* rpc_desktop = context;
    RpcSession* session = rpc_desktop->session;

    if(rpc_desktop->status_subscription) {
        rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_ERROR);
    } else {
        rpc_desktop->status_subscription = furi_pubsub_subscribe(
            rpc_desktop->status_pubsub, rpc_desktop_on_desktop_pubsub, rpc_desktop);
        rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_OK);
    }
}

static void rpc_desktop_on_status_unsubscribe_request(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    furi_assert(request->which_content == PB_Main_desktop_status_unsubscribe_request_tag);

    FURI_LOG_D(TAG, "StatusUnsubscribeRequest");
    RpcDesktop* rpc_desktop = context;
    RpcSession* session = rpc_desktop->session;

    if(rpc_desktop->status_subscription) {
        furi_pubsub_unsubscribe(rpc_desktop->status_pubsub, rpc_desktop->status_subscription);
        rpc_desktop->status_subscription = NULL;
        rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_OK);
    } else {
        rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_ERROR);
    }
}

void* rpc_desktop_alloc(RpcSession* session) {
    furi_assert(session);

    RpcDesktop* rpc_desktop = malloc(sizeof(RpcDesktop));
    rpc_desktop->desktop = furi_record_open(RECORD_DESKTOP);
    rpc_desktop->status_pubsub = desktop_api_get_status_pubsub(rpc_desktop->desktop);
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

    rpc_handler.message_handler = rpc_desktop_on_status_subscribe_request;
    rpc_add_handler(session, PB_Main_desktop_status_subscribe_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_desktop_on_status_unsubscribe_request;
    rpc_add_handler(session, PB_Main_desktop_status_unsubscribe_request_tag, &rpc_handler);

    return rpc_desktop;
}

void rpc_desktop_free(void* context) {
    furi_assert(context);
    RpcDesktop* rpc_desktop = context;

    if(rpc_desktop->status_subscription) {
        furi_pubsub_unsubscribe(rpc_desktop->status_pubsub, rpc_desktop->status_subscription);
    }

    furi_assert(rpc_desktop->desktop);
    furi_record_close(RECORD_DESKTOP);

    rpc_desktop->session = NULL;
    free(rpc_desktop);
}
