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

#define RECORD_NETWORK "network"

typedef struct Network Network;

/** Transport protocol */
typedef enum {
    NetworkProtocolTcp,
    NetworkProtocolUdp,
} NetworkProtocol;

/** Connection state reported by the companion device */
typedef enum {
    NetworkStateDisconnected,
    NetworkStateConnecting,
    NetworkStateConnected,
    NetworkStateError,
} NetworkState;

/** Network error code reported by the companion device */
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
} NetworkError;

/** Network event type */
typedef enum {
    NetworkEventConnected, /**< Companion answered a connect request */
    NetworkEventStateChanged, /**< Connection state changed asynchronously */
    NetworkEventReceived, /**< Inbound data arrived */
    NetworkEventSent, /**< Companion acknowledged a send request */
    NetworkEventClosed, /**< Companion acknowledged a close request */
} NetworkEventType;

/** Network event payload */
typedef struct {
    NetworkEventType type;
    uint32_t connection_id;
    NetworkState state; /**< Connected/StateChanged */
    NetworkError error;
    const char* resolved_ip; /**< Connected, may be NULL */
    const uint8_t* data; /**< Received, may be NULL */
    size_t size; /**< Received bytes count, or Sent bytes count */
} NetworkEvent;

/** Network event callback
 *
 * @param      event    Event payload, valid only for the duration of the call
 * @param      context  Callback context
 */
typedef void (*NetworkEventCallback)(const NetworkEvent* event, void* context);

#define NETWORK_MAX_HOST_LENGTH 255
#define NETWORK_MAX_DATA_SIZE   512

/** Open a connection through the companion device
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

/** Send data over an open connection
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
