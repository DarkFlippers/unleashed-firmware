#pragma once

#include "network.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NetworkRpcCommandConnect,
    NetworkRpcCommandSend,
    NetworkRpcCommandClose,
} NetworkRpcCommand;

typedef struct {
    uint32_t connection_id;
    const char* host;
    uint16_t port;
    NetworkProtocol protocol;
    uint32_t timeout_ms;
    const uint8_t* data;
    size_t size;
} NetworkRpcRequest;

typedef void (*NetworkRpcSend)(
    NetworkRpcCommand command,
    const NetworkRpcRequest* request,
    void* context);

void network_set_rpc_bridge(Network* network, NetworkRpcSend send, void* context);

void network_on_event(Network* network, const NetworkEvent* event);

#ifdef __cplusplus
}
#endif
