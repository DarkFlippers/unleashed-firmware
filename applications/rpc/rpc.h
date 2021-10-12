#pragma once
#include <stddef.h>
#include <stdint.h>
#include "cmsis_os.h"

typedef struct Rpc Rpc;
typedef struct RpcSession RpcSession;

typedef void (*RpcSendBytesCallback)(void* context, uint8_t* bytes, size_t bytes_len);

RpcSession* rpc_open_session(Rpc* rpc);
void rpc_close_session(RpcSession* session);
/* WARN: can't call RPC API within RpcSendBytesCallback */
void rpc_set_send_bytes_callback(RpcSession* session, RpcSendBytesCallback callback, void* context);
size_t
    rpc_feed_bytes(RpcSession* session, uint8_t* encoded_bytes, size_t size, TickType_t timeout);
