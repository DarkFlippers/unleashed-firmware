#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RPC_BUFFER_SIZE (1024)

#define RECORD_RPC "rpc"

/** Rpc interface. Used for opening session only. */
typedef struct Rpc Rpc;
/** Rpc session interface */
typedef struct RpcSession RpcSession;

/** Callback to send to client any data (e.g. response to command) */
typedef void (*RpcSendBytesCallback)(void* context, uint8_t* bytes, size_t bytes_len);
/** Callback to notify client that buffer is empty */
typedef void (*RpcBufferIsEmptyCallback)(void* context);
/** Callback to notify transport layer that close_session command
 * is received. Any other actions lays on transport layer.
 * No destruction or session close performed. */
typedef void (*RpcSessionClosedCallback)(void* context);
/** Callback to notify transport layer that session was closed
 * and all operations were finished */
typedef void (*RpcSessionTerminatedCallback)(void* context);

/** RPC owner */
typedef enum {
    RpcOwnerUnknown = 0,
    RpcOwnerBle,
    RpcOwnerUsb,
    RpcOwnerUart,
    RpcOwnerCount,
} RpcOwner;

/** Get RPC session owner
 *
 * @param   session     pointer to RpcSession descriptor
 * @return              session owner
 */
RpcOwner rpc_session_get_owner(RpcSession* session);

/** Open RPC session
 *
 * USAGE:
 * 1) rpc_session_open();
 * 2) rpc_session_set_context();
 * 3) rpc_session_set_send_bytes_callback();
 * 4) rpc_session_set_close_callback();
 * 5) while(1) {
 *      rpc_session_feed();
 *    }
 * 6) rpc_session_close();
 *
 *
 * @param   rpc     instance
 * @param   owner   owner of session
 * @return          pointer to RpcSession descriptor, or
 *                  NULL if RPC is busy and can't open session now
 */
RpcSession* rpc_session_open(Rpc* rpc, RpcOwner owner);

/** Close RPC session
 * It is guaranteed that no callbacks will be called
 * as soon as session is closed. So no need in setting
 * callbacks to NULL after session close.
 *
 * @param   session     pointer to RpcSession descriptor
 */
void rpc_session_close(RpcSession* session);

/** Set session context for callbacks to pass
 *
 * @param   session     pointer to RpcSession descriptor
 * @param   context     context to pass to callbacks
 */
void rpc_session_set_context(RpcSession* session, void* context);

/** Set callback to send bytes to client
 *  WARN: It's forbidden to call RPC API within RpcSendBytesCallback
 *
 * @param   session     pointer to RpcSession descriptor
 * @param   callback    callback to send bytes to client (can be NULL)
 */
void rpc_session_set_send_bytes_callback(RpcSession* session, RpcSendBytesCallback callback);

/** Set callback to notify that buffer is empty
 *
 * @param   session     pointer to RpcSession descriptor
 * @param   callback    callback to notify client that buffer is empty (can be NULL)
 * @param   context     context to pass to callback
 */
void rpc_session_set_buffer_is_empty_callback(
    RpcSession* session,
    RpcBufferIsEmptyCallback callback);

/** Set callback to be called when RPC command to close session is received
 *  WARN: It's forbidden to call RPC API within RpcSessionClosedCallback
 *
 * @param   session     pointer to RpcSession descriptor
 * @param   callback    callback to inform about RPC close session command (can be NULL)
 */
void rpc_session_set_close_callback(RpcSession* session, RpcSessionClosedCallback callback);

/** Set callback to be called when RPC session is closed
 *
 * @param   session     pointer to RpcSession descriptor
 * @param   callback    callback to inform about RPC session state
 */
void rpc_session_set_terminated_callback(
    RpcSession* session,
    RpcSessionTerminatedCallback callback);

/** Give bytes to RPC service to decode them and perform command
 *
 * @param   session     pointer to RpcSession descriptor
 * @param   buffer      buffer to provide to RPC service
 * @param   size        size of buffer
 * @param   timeout     max timeout to wait till all buffer will be consumed
 *
 * @return              actually consumed bytes
 */
size_t rpc_session_feed(RpcSession* session, const uint8_t* buffer, size_t size, uint32_t timeout);

/** Get available size of RPC buffer
 *
 * @param   session     pointer to RpcSession descriptor
 *
 * @return              bytes available in buffer
 */
size_t rpc_session_get_available_size(RpcSession* session);

#ifdef __cplusplus
}
#endif
