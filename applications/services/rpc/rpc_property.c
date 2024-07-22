#include <flipper.pb.h>
#include <furi_hal.h>
#include <furi_hal_info.h>
#include <furi_hal_power.h>
#include <core/core_defines.h>

#include "rpc_i.h"

#define TAG "RpcProperty"

#define PROPERTY_CATEGORY_DEVICE_INFO "devinfo"
#define PROPERTY_CATEGORY_POWER_INFO  "pwrinfo"
#define PROPERTY_CATEGORY_POWER_DEBUG "pwrdebug"

typedef struct {
    RpcSession* session;
    PB_Main* response;
    FuriString* subkey;
} RpcPropertyContext;

static void
    rpc_system_property_get_callback(const char* key, const char* value, bool last, void* context) {
    furi_assert(key);
    furi_assert(value);
    furi_assert(context);
    furi_assert(key);
    furi_assert(value);

    RpcPropertyContext* ctx = context;
    RpcSession* session = ctx->session;
    PB_Main* response = ctx->response;

    if(!strncmp(key, furi_string_get_cstr(ctx->subkey), furi_string_size(ctx->subkey))) {
        response->content.system_device_info_response.key = strdup(key);
        response->content.system_device_info_response.value = strdup(value);
        rpc_send_and_release(session, response);
    }

    if(last) {
        rpc_send_and_release_empty(session, response->command_id, PB_CommandStatus_OK);
    }
}

static void rpc_system_property_get_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_property_get_request_tag);

    FURI_LOG_D(TAG, "GetProperty");

    RpcSession* session = (RpcSession*)context;
    furi_assert(session);

    FuriString* topkey = furi_string_alloc();
    FuriString* subkey = furi_string_alloc_set_str(request->content.property_get_request.key);

    const size_t sep_idx = furi_string_search_char(subkey, '.');

    if(sep_idx == FURI_STRING_FAILURE) {
        furi_string_swap(topkey, subkey);
    } else {
        furi_string_set_n(topkey, subkey, 0, sep_idx);
        furi_string_right(subkey, sep_idx + 1);
    }

    PB_Main* response = malloc(sizeof(PB_Main));

    response->command_id = request->command_id;
    response->command_status = PB_CommandStatus_OK;
    response->has_next = true;
    response->which_content = PB_Main_property_get_response_tag;

    RpcPropertyContext property_context = {
        .session = session,
        .response = response,
        .subkey = subkey,
    };

    if(!furi_string_cmp(topkey, PROPERTY_CATEGORY_DEVICE_INFO)) {
        furi_hal_info_get(rpc_system_property_get_callback, '.', &property_context);
    } else if(!furi_string_cmp(topkey, PROPERTY_CATEGORY_POWER_INFO)) {
        furi_hal_power_info_get(rpc_system_property_get_callback, '.', &property_context);
    } else if(!furi_string_cmp(topkey, PROPERTY_CATEGORY_POWER_DEBUG)) {
        furi_hal_power_debug_get(rpc_system_property_get_callback, &property_context);
    } else {
        rpc_send_and_release_empty(
            session, request->command_id, PB_CommandStatus_ERROR_INVALID_PARAMETERS);
    }

    furi_string_free(subkey);
    furi_string_free(topkey);

    free(response);
}

void* rpc_system_property_alloc(RpcSession* session) {
    furi_assert(session);

    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = session,
    };

    rpc_handler.message_handler = rpc_system_property_get_process;
    rpc_add_handler(session, PB_Main_property_get_request_tag, &rpc_handler);
    return NULL;
}
