#include "network_i.h"

#include <furi.h>
#include <string.h>

struct Network {
    FuriMutex* mutex;

    NetworkRpcSend rpc_send;
    void* rpc_send_context;

    NetworkEventCallback event_callback;
    void* event_context;
};

static Network* network_alloc(void) {
    Network* network = malloc(sizeof(Network));
    network->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    network->rpc_send = NULL;
    network->rpc_send_context = NULL;
    network->event_callback = NULL;
    network->event_context = NULL;
    return network;
}

static bool
    network_dispatch(Network* network, NetworkRpcCommand command, const NetworkRpcRequest* request) {
    furi_mutex_acquire(network->mutex, FuriWaitForever);
    bool sent = false;
    if(network->rpc_send) {
        network->rpc_send(command, request, network->rpc_send_context);
        sent = true;
    }
    furi_mutex_release(network->mutex);
    return sent;
}

bool network_connect(
    Network* network,
    uint32_t connection_id,
    const char* host,
    uint16_t port,
    NetworkProtocol protocol,
    uint32_t timeout_ms) {
    furi_check(network);
    furi_check(host);
    if(strlen(host) > NETWORK_MAX_HOST_LENGTH) return false;

    const NetworkRpcRequest request = {
        .connection_id = connection_id,
        .host = host,
        .port = port,
        .protocol = protocol,
        .timeout_ms = timeout_ms,
        .data = NULL,
        .size = 0,
    };
    return network_dispatch(network, NetworkRpcCommandConnect, &request);
}

bool network_send(Network* network, uint32_t connection_id, const uint8_t* data, size_t size) {
    furi_check(network);
    furi_check(data);
    if(size == 0 || size > NETWORK_MAX_DATA_SIZE) return false;

    const NetworkRpcRequest request = {
        .connection_id = connection_id,
        .host = NULL,
        .port = 0,
        .protocol = NetworkProtocolTcp,
        .timeout_ms = 0,
        .data = data,
        .size = size,
    };
    return network_dispatch(network, NetworkRpcCommandSend, &request);
}

bool network_close(Network* network, uint32_t connection_id) {
    furi_check(network);

    const NetworkRpcRequest request = {
        .connection_id = connection_id,
        .host = NULL,
        .port = 0,
        .protocol = NetworkProtocolTcp,
        .timeout_ms = 0,
        .data = NULL,
        .size = 0,
    };
    return network_dispatch(network, NetworkRpcCommandClose, &request);
}

void network_set_event_callback(Network* network, NetworkEventCallback callback, void* context) {
    furi_check(network);
    furi_mutex_acquire(network->mutex, FuriWaitForever);
    network->event_callback = callback;
    network->event_context = context;
    furi_mutex_release(network->mutex);
}

void network_set_rpc_bridge(Network* network, NetworkRpcSend send, void* context) {
    furi_check(network);
    furi_mutex_acquire(network->mutex, FuriWaitForever);
    network->rpc_send = send;
    network->rpc_send_context = context;
    furi_mutex_release(network->mutex);
}

void network_on_event(Network* network, const NetworkEvent* event) {
    furi_check(network);
    furi_check(event);
    furi_mutex_acquire(network->mutex, FuriWaitForever);
    NetworkEventCallback callback = network->event_callback;
    void* context = network->event_context;
    furi_mutex_release(network->mutex);
    if(callback) {
        callback(event, context);
    }
}

void network_on_system_start(void* p) {
    UNUSED(p);
    furi_record_create(RECORD_NETWORK, network_alloc());
}
