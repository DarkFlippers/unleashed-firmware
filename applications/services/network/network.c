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
    };
    return network_dispatch(network, NetworkRpcCommandConnect, &request);
}

bool network_send(Network* network, uint32_t connection_id, const uint8_t* data, size_t size) {
    return network_websocket_send(network, connection_id, data, size, false);
}

bool network_websocket_send(
    Network* network,
    uint32_t connection_id,
    const uint8_t* data,
    size_t size,
    bool binary) {
    furi_check(network);
    furi_check(data);
    if(size == 0 || size > NETWORK_MAX_DATA_SIZE) return false;

    const NetworkRpcRequest request = {
        .connection_id = connection_id,
        .data = data,
        .size = size,
        .binary = binary,
    };
    return network_dispatch(network, NetworkRpcCommandSend, &request);
}

bool network_close(Network* network, uint32_t connection_id) {
    furi_check(network);

    const NetworkRpcRequest request = {
        .connection_id = connection_id,
    };
    return network_dispatch(network, NetworkRpcCommandClose, &request);
}

bool network_http_request(Network* network, uint32_t request_id, const NetworkHttpRequest* request) {
    furi_check(network);
    furi_check(request);
    furi_check(request->url);
    if(strlen(request->url) > NETWORK_MAX_URL_LENGTH) return false;
    if(request->body_size > NETWORK_MAX_DATA_SIZE) return false;

    const NetworkRpcRequest rpc = {
        .connection_id = request_id,
        .timeout_ms = request->timeout_ms,
        .data = request->body,
        .size = request->body_size,
        .method = request->method,
        .url = request->url,
        .headers = request->headers,
        .send_path = request->send_path,
        .save_path = request->save_path,
        .include_headers = request->include_headers,
    };
    return network_dispatch(network, NetworkRpcCommandHttpRequest, &rpc);
}

bool network_websocket_open(
    Network* network,
    uint32_t connection_id,
    const char* url,
    const char* headers,
    uint32_t timeout_ms) {
    furi_check(network);
    furi_check(url);
    if(strlen(url) > NETWORK_MAX_URL_LENGTH) return false;

    const NetworkRpcRequest request = {
        .connection_id = connection_id,
        .timeout_ms = timeout_ms,
        .url = url,
        .headers = headers,
    };
    return network_dispatch(network, NetworkRpcCommandWebSocketOpen, &request);
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
