#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"

/** Rpc interface. Used for opening session only. */
typedef struct Rpc Rpc;
/** Rpc session interface */
typedef struct RpcSession RpcSession;

/** Callback to send to client any data (e.g. response to command) */
typedef void (*RpcSendBytesCallback)(void* context, uint8_t* bytes, size_t bytes_len);
/** Callback to notify transport layer that close_session command
 * is received. Any other actions lays on transport layer.
 * No destruction or session close preformed. */
typedef void (*RpcSessionClosedCallback)(void* context);

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
 * @return          pointer to RpcSession descriptor, or
 *                  NULL if RPC is busy and can't open session now
 */
RpcSession* rpc_session_open(Rpc* rpc);

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

/** Set callback to be called when RPC command to close session is received
 *  WARN: It's forbidden to call RPC API within RpcSessionClosedCallback
 *
 * @param   session     pointer to RpcSession descriptor
 * @param   callback    callback to inform about RPC close session command (can be NULL)
 */
void rpc_session_set_close_callback(RpcSession* session, RpcSessionClosedCallback callback);

/** Give bytes to RPC service to decode them and perform command
 *
 * @param   session     pointer to RpcSession descriptor
 * @param   buffer      buffer to provide to RPC service
 * @param   size        size of buffer
 * @param   timeout     max timeout to wait till all buffer will be consumed
 *
 * @return              actually consumed bytes
 */
size_t rpc_session_feed(RpcSession* session, uint8_t* buffer, size_t size, TickType_t timeout);
