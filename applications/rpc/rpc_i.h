#pragma once
#include "rpc.h"
#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <flipper.pb.h>
#include <cli/cli.h>

typedef void* (*RpcSystemAlloc)(Rpc*);
typedef void (*RpcSystemFree)(void*);
typedef void (*PBMessageHandler)(const PB_Main* msg_request, void* context);

typedef struct {
    bool (*decode_submessage)(pb_istream_t* stream, const pb_field_t* field, void** arg);
    PBMessageHandler message_handler;
    void* context;
} RpcHandler;

void rpc_send_and_release(Rpc* rpc, PB_Main* main_message);
void rpc_send_and_release_empty(Rpc* rpc, uint32_t command_id, PB_CommandStatus status);
void rpc_add_handler(Rpc* rpc, pb_size_t message_tag, RpcHandler* handler);

void* rpc_system_status_alloc(Rpc* rpc);
void* rpc_system_storage_alloc(Rpc* rpc);
void rpc_system_storage_free(void* ctx);
void* rpc_system_app_alloc(Rpc* rpc);

void rpc_print_message(const PB_Main* message);
void rpc_cli_command_start_session(Cli* cli, string_t args, void* context);
