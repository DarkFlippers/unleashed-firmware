#pragma once
#include "rpc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RpcAppEventSessionClose,
    RpcAppEventAppExit,
    RpcAppEventLoadFile,
    RpcAppEventButtonPress,
    RpcAppEventButtonRelease,
} RpcAppSystemEvent;

typedef void (*RpcAppSystemCallback)(RpcAppSystemEvent event, void* context);

typedef struct RpcAppSystem RpcAppSystem;

void rpc_system_app_set_callback(RpcAppSystem* rpc_app, RpcAppSystemCallback callback, void* ctx);

void rpc_system_app_send_started(RpcAppSystem* rpc_app);

void rpc_system_app_send_exited(RpcAppSystem* rpc_app);

const char* rpc_system_app_get_data(RpcAppSystem* rpc_app);

void rpc_system_app_confirm(RpcAppSystem* rpc_app, RpcAppSystemEvent event, bool result);

#ifdef __cplusplus
}
#endif
