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
#define NETWORK_MAX_HOST_LENGTH 255   /**< Maximum host name length, bytes */
#define NETWORK_MAX_URL_LENGTH  2048  /**< Maximum request URL length, bytes */
#define NETWORK_MAX_DATA_SIZE   512   /**< Maximum payload size per send, bytes */

typedef struct Network Network;

/** Transport protocol for raw socket connections */
typedef enum {
    NetworkProtocolTcp, /**< TCP stream socket */
    NetworkProtocolUdp, /**< UDP datagram socket */
} NetworkProtocol;

/** HTTP request method */
typedef enum {
    NetworkHttpMethodGet,    /**< GET */
    NetworkHttpMethodPost,   /**< POST */
    NetworkHttpMethodPut,    /**< PUT */
    NetworkHttpMethodPatch,  /**< PATCH */
    NetworkHttpMethodDelete, /**< DELETE */
    NetworkHttpMethodHead,   /**< HEAD */
} NetworkHttpMethod;

/** Connection state reported by the companion device */
typedef enum {
    NetworkStateDisconnected, /**< Connection is closed */
    NetworkStateConnecting,   /**< Connection is being established */
    NetworkStateConnected,    /**< Connection is open */
    NetworkStateError,        /**< Connection failed */
} NetworkState;

/** Network error code reported by the companion device */
typedef enum {
    NetworkErrorNone,               /**< No error */
    NetworkErrorDnsFailed,          /**< Host name resolution failed */
    NetworkErrorTimeout,            /**< Operation timed out */
    NetworkErrorConnectionRefused,  /**< Peer refused the connection */
    NetworkErrorNetworkUnreachable, /**< Network is unreachable */
    NetworkErrorHostUnreachable,    /**< Host is unreachable */
    NetworkErrorInvalidConnection,  /**< Connection id is unknown or already in use */
    NetworkErrorNotConnected,       /**< Connection is not open */
    NetworkErrorSendFailed,         /**< Sending data failed */
    NetworkErrorReceiveFailed,      /**< Receiving data failed */
    NetworkErrorMaxConnections,     /**< Too many concurrent connections */
    NetworkErrorInvalidProtocol,    /**< Unsupported transport protocol */
    NetworkErrorInternal,           /**< Internal companion error */
    NetworkErrorTlsFailed,          /**< TLS handshake or certificate error */
    NetworkErrorInvalidUrl,         /**< Malformed or unsupported URL */
    NetworkErrorFileError,          /**< SD card read/write error during transfer */
} NetworkError;

/** Network event type delivered to the event callback */
typedef enum {
    NetworkEventConnected,     /**< Companion answered a connect or WebSocket open request */
    NetworkEventStateChanged,  /**< Connection state changed asynchronously */
    NetworkEventReceived,      /**< Inbound data arrived (socket, WebSocket frame or HTTP body) */
    NetworkEventSent,          /**< Companion acknowledged a send request */
    NetworkEventClosed,        /**< Companion acknowledged a close request */
    NetworkEventHttpResponse,  /**< HTTP request finished, metadata available */
} NetworkEventType;

/** Network event payload, valid only for the duration of the callback */
typedef struct {
    NetworkEventType type;    /**< Event type */
    uint32_t connection_id;   /**< Connection or request identifier */
    NetworkState state;       /**< Connected/StateChanged: connection state */
    NetworkError error;       /**< Error code, NetworkErrorNone on success */
    const char* resolved_ip;  /**< Connected: resolved peer address, may be NULL */
    const uint8_t* data;      /**< Received: payload, may be NULL */
    size_t size;              /**< Received: byte count; Sent: bytes sent; HttpResponse: body size */
    bool binary;              /**< Received: WebSocket binary frame flag */
    uint32_t http_status;     /**< HttpResponse: HTTP status code */
    const char* http_headers; /**< HttpResponse: response headers, may be NULL */
    bool saved_to_file;       /**< HttpResponse: body was written to SD by the companion */
} NetworkEvent;

/** Network event callback
 *
 * @param      event    Event payload, valid only for the duration of the call
 * @param      context  Callback context
 */
typedef void (*NetworkEventCallback)(const NetworkEvent* event, void* context);

/** HTTP request description
 *
 * The companion performs the request and either streams the body back as
 * NetworkEventReceived chunks (when @c save_path is NULL) or writes it to the
 * SD card itself (when @c save_path is set). NetworkEventHttpResponse always
 * concludes the request.
 */
typedef struct {
    NetworkHttpMethod method; /**< Request method */
    const char* url;          /**< Full request URL, up to NETWORK_MAX_URL_LENGTH */
    const char* headers;      /**< Request headers as "Name: Value\r\n..." or NULL */
    const uint8_t* body;      /**< Inline request body, up to NETWORK_MAX_DATA_SIZE, or NULL */
    size_t body_size;         /**< Inline request body size */
    const char* send_path;    /**< SD path the companion reads the body from, or NULL */
    const char* save_path;    /**< SD path the companion writes the response to, or NULL */
    uint32_t timeout_ms;      /**< Request timeout in milliseconds, 0 for default */
    bool include_headers;     /**< Request that response headers be returned */
} NetworkHttpRequest;

/** Network error description
 *
 * @param      error  Network error code
 *
 * @return     Static description string
 */
static inline const char* network_error_to_string(NetworkError error) {
    switch(error) {
    case NetworkErrorNone:
        return "OK";
    case NetworkErrorDnsFailed:
        return "DNS failed";
    case NetworkErrorTimeout:
        return "Timeout";
    case NetworkErrorConnectionRefused:
        return "Connection refused";
    case NetworkErrorNetworkUnreachable:
        return "Network unreachable";
    case NetworkErrorHostUnreachable:
        return "Host unreachable";
    case NetworkErrorInvalidConnection:
        return "Invalid connection";
    case NetworkErrorNotConnected:
        return "Not connected";
    case NetworkErrorSendFailed:
        return "Send failed";
    case NetworkErrorReceiveFailed:
        return "Receive failed";
    case NetworkErrorMaxConnections:
        return "Too many connections";
    case NetworkErrorInvalidProtocol:
        return "Invalid protocol";
    case NetworkErrorInternal:
        return "Internal error";
    case NetworkErrorTlsFailed:
        return "TLS failed";
    case NetworkErrorInvalidUrl:
        return "Invalid URL";
    case NetworkErrorFileError:
        return "File error";
    default:
        return "Unknown error";
    }
}

/** Connection state description
 *
 * @param      state  Connection state
 *
 * @return     Static description string
 */
static inline const char* network_state_to_string(NetworkState state) {
    switch(state) {
    case NetworkStateDisconnected:
        return "Disconnected";
    case NetworkStateConnecting:
        return "Connecting";
    case NetworkStateConnected:
        return "Connected";
    case NetworkStateError:
        return "Error";
    default:
        return "Unknown";
    }
}

/** Check whether a connection state is terminal
 *
 * @param      state  Connection state
 *
 * @return     true if the connection will not deliver more data
 */
static inline bool network_state_is_terminal(NetworkState state) {
    return state == NetworkStateDisconnected || state == NetworkStateError;
}

/** Open a raw socket connection through the companion device
 *
 * @param      network        Network instance
 * @param      connection_id  Client-assigned connection identifier
 * @param      host           Host name or address, up to NETWORK_MAX_HOST_LENGTH
 * @param      port           Port number
 * @param      protocol       Transport protocol
 * @param      timeout_ms     Connection timeout in milliseconds
 *
 * @return     true if the request was sent to a live companion session
 */
bool network_connect(
    Network* network,
    uint32_t connection_id,
    const char* host,
    uint16_t port,
    NetworkProtocol protocol,
    uint32_t timeout_ms);

/** Send data over an open socket connection
 *
 * @param      network        Network instance
 * @param      connection_id  Connection identifier
 * @param      data           Data buffer
 * @param      size           Data size, up to NETWORK_MAX_DATA_SIZE
 *
 * @return     true if the request was sent to a live companion session
 */
bool network_send(Network* network, uint32_t connection_id, const uint8_t* data, size_t size);

/** Close an open connection
 *
 * @param      network        Network instance
 * @param      connection_id  Connection identifier
 *
 * @return     true if the request was sent to a live companion session
 */
bool network_close(Network* network, uint32_t connection_id);

/** Perform an HTTP/HTTPS request through the companion device
 *
 * @param      network     Network instance
 * @param      request_id  Client-assigned request identifier
 * @param      request     Request description
 *
 * @return     true if the request was sent to a live companion session
 */
bool network_http_request(Network* network, uint32_t request_id, const NetworkHttpRequest* request);

/** Open a WebSocket connection through the companion device
 *
 * @param      network        Network instance
 * @param      connection_id  Client-assigned connection identifier
 * @param      url            WebSocket URL (ws:// or wss://), up to NETWORK_MAX_URL_LENGTH
 * @param      headers        Extra handshake headers as "Name: Value\r\n..." or NULL
 * @param      timeout_ms     Connection timeout in milliseconds
 *
 * @return     true if the request was sent to a live companion session
 */
bool network_websocket_open(
    Network* network,
    uint32_t connection_id,
    const char* url,
    const char* headers,
    uint32_t timeout_ms);

/** Send a frame over an open WebSocket connection
 *
 * @param      network        Network instance
 * @param      connection_id  Connection identifier
 * @param      data           Frame payload
 * @param      size           Payload size, up to NETWORK_MAX_DATA_SIZE
 * @param      binary         true for a binary frame, false for a text frame
 *
 * @return     true if the request was sent to a live companion session
 */
bool network_websocket_send(
    Network* network,
    uint32_t connection_id,
    const uint8_t* data,
    size_t size,
    bool binary);

/** Set the network event callback
 *
 * @param      network   Network instance
 * @param      callback  NetworkEventCallback
 * @param      context   NetworkEventCallback context
 */
void network_set_event_callback(Network* network, NetworkEventCallback callback, void* context);

#ifdef __cplusplus
}
#endif
