#include "flipper.pb.h"
#include "rpc_i.h"
#include "status.pb.h"

#include <furi-hal-info.h>
#include <furi-hal-bootloader.h>
#include <power/power_service/power.h>

void rpc_system_system_ping_process(const PB_Main* msg_request, void* context) {
    furi_assert(msg_request);
    furi_assert(msg_request->which_content == PB_Main_system_ping_request_tag);
    furi_assert(context);
    Rpc* rpc = context;

    if(msg_request->has_next) {
        rpc_send_and_release_empty(
            rpc, msg_request->command_id, PB_CommandStatus_ERROR_INVALID_PARAMETERS);
        return;
    }

    PB_Main msg_response = PB_Main_init_default;
    msg_response.has_next = false;
    msg_response.command_status = PB_CommandStatus_OK;
    msg_response.command_id = msg_request->command_id;
    msg_response.which_content = PB_Main_system_ping_response_tag;

    const PB_System_PingRequest* request = &msg_request->content.system_ping_request;
    PB_System_PingResponse* response = &msg_response.content.system_ping_response;
    if(request->data && (request->data->size > 0)) {
        response->data = furi_alloc(PB_BYTES_ARRAY_T_ALLOCSIZE(request->data->size));
        memcpy(response->data->bytes, request->data->bytes, request->data->size);
        response->data->size = request->data->size;
    }

    rpc_send_and_release(rpc, &msg_response);
}

void rpc_system_system_reboot_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_system_reboot_request_tag);
    furi_assert(context);
    Rpc* rpc = context;

    const int mode = request->content.system_reboot_request.mode;

    if(mode == PB_System_RebootRequest_RebootMode_OS) {
        power_reboot(PowerBootModeNormal);
    } else if(mode == PB_System_RebootRequest_RebootMode_DFU) {
        power_reboot(PowerBootModeDfu);
    } else {
        rpc_send_and_release_empty(
            rpc, request->command_id, PB_CommandStatus_ERROR_INVALID_PARAMETERS);
    }
}

typedef struct {
    Rpc* rpc;
    PB_Main* response;
} RpcSystemSystemDeviceInfoContext;

void rpc_system_system_device_info_callback(
    const char* key,
    const char* value,
    bool last,
    void* context) {
    furi_assert(key);
    furi_assert(value);
    furi_assert(context);
    RpcSystemSystemDeviceInfoContext* ctx = context;

    char* str_key = strdup(key);
    char* str_value = strdup(value);

    ctx->response->has_next = !last;
    ctx->response->content.system_device_info_response.key = str_key;
    ctx->response->content.system_device_info_response.value = str_value;

    rpc_send_and_release(ctx->rpc, ctx->response);
}

void rpc_system_system_device_info_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_system_device_info_request_tag);
    furi_assert(context);
    Rpc* rpc = context;

    PB_Main* response = furi_alloc(sizeof(PB_Main));
    response->command_id = request->command_id;
    response->which_content = PB_Main_system_device_info_response_tag;
    response->command_status = PB_CommandStatus_OK;

    RpcSystemSystemDeviceInfoContext device_info_context = {
        .rpc = rpc,
        .response = response,
    };

    furi_hal_info_get(rpc_system_system_device_info_callback, &device_info_context);

    free(response);
}

void rpc_system_system_factory_reset_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_system_factory_reset_request_tag);
    furi_assert(context);

    furi_hal_bootloader_set_flags(FuriHalBootloaderFlagFactoryReset);
    power_reboot(PowerBootModeNormal);
}

void* rpc_system_system_alloc(Rpc* rpc) {
    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = rpc,
    };

    rpc_handler.message_handler = rpc_system_system_ping_process;
    rpc_add_handler(rpc, PB_Main_system_ping_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_system_reboot_process;
    rpc_add_handler(rpc, PB_Main_system_reboot_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_system_device_info_process;
    rpc_add_handler(rpc, PB_Main_system_device_info_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_system_factory_reset_process;
    rpc_add_handler(rpc, PB_Main_system_factory_reset_request_tag, &rpc_handler);

    return NULL;
}
