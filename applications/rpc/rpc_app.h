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

typedef bool (*RpcAppSystemCallback)(RpcAppSystemEvent event, const char* arg, void* context);

typedef struct RpcAppSystem RpcAppSystem;

void rpc_system_app_set_callback(RpcAppSystem* rpc_app, RpcAppSystemCallback callback, void* ctx);

void rpc_system_app_send_started(RpcAppSystem* rpc_app);

void rpc_system_app_send_exited(RpcAppSystem* rpc_app);

#ifdef __cplusplus
}
#endif
