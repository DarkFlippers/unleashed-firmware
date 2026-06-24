/**
 * @file network.h
 * Network: companion internet proxy API
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_NETWORK          "network"
#define NETWORK_MAX_HOST_LENGTH 255
#define NETWORK_MAX_URL_LENGTH  2048
#define NETWORK_MAX_DATA_SIZE   512

typedef struct Network Network;

typedef enum {
    NetworkProtocolTcp,
    NetworkProtocolUdp,
} NetworkProtocol;

typedef enum {
    NetworkHttpMethodGet,
    NetworkHttpMethodPost,
    NetworkHttpMethodPut,
    NetworkHttpMethodPatch,
    NetworkHttpMethodDelete,
    NetworkHttpMethodHead,
} NetworkHttpMethod;

typedef enum {
    NetworkStateDisconnected,
    NetworkStateConnecting,
    NetworkStateConnected,
    NetworkStateError,
} NetworkState;

typedef enum {
    NetworkErrorNone,
    NetworkErrorDnsFailed,
    NetworkErrorTimeout,
    NetworkErrorConnectionRefused,
    NetworkErrorNetworkUnreachable,
    NetworkErrorHostUnreachable,
    NetworkErrorInvalidConnection,
    NetworkErrorNotConnected,
    NetworkErrorSendFailed,
    NetworkErrorReceiveFailed,
    NetworkErrorMaxConnections,
    NetworkErrorInvalidProtocol,
    NetworkErrorInternal,
    NetworkErrorTlsFailed,
    NetworkErrorInvalidUrl,
    NetworkErrorFileError,
} NetworkError;

typedef enum {
    NetworkEventConnected,
    NetworkEventStateChanged,
    NetworkEventReceived,
    NetworkEventSent,
    NetworkEventClosed,
    NetworkEventHttpResponse,
} NetworkEventType;

typedef struct {
    NetworkEventType type;
    uint32_t connection_id;
    NetworkState state;
    NetworkError error;
    const char* resolved_ip;
    const uint8_t* data;
    size_t size;
    bool binary;
    uint32_t http_status;
    const char* http_headers;
    bool saved_to_file;
} NetworkEvent;

typedef void (*NetworkEventCallback)(const NetworkEvent* event, void* context);

typedef struct {
    NetworkHttpMethod method;
    const char* url;
    const char* headers;
    const uint8_t* body;
    size_t body_size;
    const char* send_path;
    const char* save_path;
    uint32_t timeout_ms;
    bool include_headers;
} NetworkHttpRequest;

const char* network_error_to_string(NetworkError error);

const char* network_state_to_string(NetworkState state);

static inline bool network_state_is_terminal(NetworkState state) {
    return state == NetworkStateDisconnected || state == NetworkStateError;
}

bool network_connect(
    Network* network,
    uint32_t connection_id,
    const char* host,
    uint16_t port,
    NetworkProtocol protocol,
    uint32_t timeout_ms);

bool network_send(Network* network, uint32_t connection_id, const uint8_t* data, size_t size);

bool network_close(Network* network, uint32_t connection_id);

bool network_http_request(Network* network, uint32_t request_id, const NetworkHttpRequest* request);

bool network_websocket_open(
    Network* network,
    uint32_t connection_id,
    const char* url,
    const char* headers,
    uint32_t timeout_ms);

bool network_websocket_send(
    Network* network,
    uint32_t connection_id,
    const uint8_t* data,
    size_t size,
    bool binary);

void network_set_event_callback(Network* network, NetworkEventCallback callback, void* context);

#ifdef __cplusplus
}
#endif
