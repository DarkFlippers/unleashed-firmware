#include "rpc_i.h"

#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include <storage.pb.h>
#include <flipper.pb.h>

#include <furi.h>

#include <cli/cli.h>
#include <stdint.h>
#include <stdio.h>
#include <m-dict.h>

#include <bt/bt_service/bt.h>

#define TAG "RpcSrv"

typedef enum {
    RpcEvtNewData = (1 << 0),
    RpcEvtDisconnect = (1 << 1),
} RpcEvtFlags;

#define RPC_ALL_EVENTS (RpcEvtNewData | RpcEvtDisconnect)

DICT_DEF2(RpcHandlerDict, pb_size_t, M_DEFAULT_OPLIST, RpcHandler, M_POD_OPLIST)

typedef struct {
    RpcSystemAlloc alloc;
    RpcSystemFree free;
    void* context;
} RpcSystemCallbacks;

static RpcSystemCallbacks rpc_systems[] = {
    {
        .alloc = rpc_system_system_alloc,
        .free = NULL,
    },
    {
        .alloc = rpc_system_storage_alloc,
        .free = rpc_system_storage_free,
    },
    {
        .alloc = rpc_system_app_alloc,
        .free = rpc_system_app_free,
    },
    {
        .alloc = rpc_system_gui_alloc,
        .free = rpc_system_gui_free,
    },
    {
        .alloc = rpc_system_gpio_alloc,
        .free = NULL,
    },
    {
        .alloc = rpc_system_property_alloc,
        .free = NULL,
    },
    {
        .alloc = rpc_desktop_alloc,
        .free = rpc_desktop_free,
    },
};

struct RpcSession {
    Rpc* rpc;

    FuriThread* thread;

    RpcHandlerDict_t handlers;
    FuriStreamBuffer* stream;
    PB_Main* decoded_message;
    bool terminate;
    void** system_contexts;
    bool decode_error;

    FuriMutex* callbacks_mutex;
    RpcSendBytesCallback send_bytes_callback;
    RpcBufferIsEmptyCallback buffer_is_empty_callback;
    RpcSessionClosedCallback closed_callback;
    RpcSessionTerminatedCallback terminated_callback;
    RpcOwner owner;
    void* context;
};

struct Rpc {
    FuriMutex* busy_mutex;
};

RpcOwner rpc_session_get_owner(RpcSession* session) {
    furi_check(session);
    return session->owner;
}

static void rpc_close_session_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);

    RpcSession* session = (RpcSession*)context;

    rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_OK);
    furi_mutex_acquire(session->callbacks_mutex, FuriWaitForever);
    if(session->closed_callback) {
        session->closed_callback(session->context);
    } else {
        FURI_LOG_W(TAG, "Session stop isn't processed by transport layer");
    }
    furi_mutex_release(session->callbacks_mutex);
}

void rpc_session_set_context(RpcSession* session, void* context) {
    furi_check(session);

    furi_mutex_acquire(session->callbacks_mutex, FuriWaitForever);
    session->context = context;
    furi_mutex_release(session->callbacks_mutex);
}

void rpc_session_set_close_callback(RpcSession* session, RpcSessionClosedCallback callback) {
    furi_check(session);

    furi_mutex_acquire(session->callbacks_mutex, FuriWaitForever);
    session->closed_callback = callback;
    furi_mutex_release(session->callbacks_mutex);
}

void rpc_session_set_send_bytes_callback(RpcSession* session, RpcSendBytesCallback callback) {
    furi_check(session);

    furi_mutex_acquire(session->callbacks_mutex, FuriWaitForever);
    session->send_bytes_callback = callback;
    furi_mutex_release(session->callbacks_mutex);
}

void rpc_session_set_buffer_is_empty_callback(
    RpcSession* session,
    RpcBufferIsEmptyCallback callback) {
    furi_check(session);

    furi_mutex_acquire(session->callbacks_mutex, FuriWaitForever);
    session->buffer_is_empty_callback = callback;
    furi_mutex_release(session->callbacks_mutex);
}

void rpc_session_set_terminated_callback(
    RpcSession* session,
    RpcSessionTerminatedCallback callback) {
    furi_check(session);

    furi_mutex_acquire(session->callbacks_mutex, FuriWaitForever);
    session->terminated_callback = callback;
    furi_mutex_release(session->callbacks_mutex);
}

/* Doesn't forbid using rpc_feed_bytes() after session close - it's safe.
 * Because any bytes received in buffer will be flushed before next session.
 * If bytes get into stream buffer before it's get epmtified and this
 * command is gets processed - it's safe either. But case of it is quite
 * odd: client sends close request and sends command after.
 */
size_t rpc_session_feed(
    RpcSession* session,
    const uint8_t* encoded_bytes,
    size_t size,
    uint32_t timeout) {
    furi_check(session);
    furi_check(encoded_bytes);

    if(!size) return 0;

    size_t bytes_sent = furi_stream_buffer_send(session->stream, encoded_bytes, size, timeout);

    furi_thread_flags_set(furi_thread_get_id(session->thread), RpcEvtNewData);

    return bytes_sent;
}

size_t rpc_session_get_available_size(RpcSession* session) {
    furi_check(session);
    return furi_stream_buffer_spaces_available(session->stream);
}

bool rpc_pb_stream_read(pb_istream_t* istream, pb_byte_t* buf, size_t count) {
    furi_assert(istream);
    furi_assert(buf);
    RpcSession* session = istream->state;
    furi_assert(session);
    furi_assert(istream->bytes_left);

    if(session->terminate) {
        return false;
    }

    uint32_t flags = 0;
    size_t bytes_received = 0;

    while(1) {
        bytes_received += furi_stream_buffer_receive(
            session->stream, buf + bytes_received, count - bytes_received, 0);
        if(furi_stream_buffer_is_empty(session->stream)) {
            if(session->buffer_is_empty_callback) {
                session->buffer_is_empty_callback(session->context);
            }
        }
        if(session->decode_error) {
            /* never go out till RPC_EVENT_DISCONNECT come */
            bytes_received = 0;
        }
        if(count == bytes_received) {
            break;
        } else {
            flags = furi_thread_flags_wait(RPC_ALL_EVENTS, FuriFlagWaitAny, FuriWaitForever);
            if(flags & RpcEvtDisconnect) {
                if(furi_stream_buffer_is_empty(session->stream)) {
                    session->terminate = true;
                    istream->bytes_left = 0;
                    bytes_received = 0;
                    break;
                } else {
                    /* Save disconnect flag and continue reading buffer */
                    furi_thread_flags_set(furi_thread_get_id(session->thread), RpcEvtDisconnect);
                }
            } else if(flags & RpcEvtNewData) {
                // Just wake thread up
            }
        }
    }

#ifdef SRV_RPC_DEBUG
    rpc_debug_print_data("INPUT", buf, bytes_received);
#endif

    return count == bytes_received;
}

static bool rpc_pb_content_callback(pb_istream_t* stream, const pb_field_t* field, void** arg) {
    furi_assert(stream);
    RpcSession* session = stream->state;
    furi_assert(session);
    furi_assert(field);

    RpcHandler* handler = RpcHandlerDict_get(session->handlers, field->tag);

    if(handler && handler->decode_submessage) {
        handler->decode_submessage(stream, field, arg);
    }

    return true;
}

static int32_t rpc_session_worker(void* context) {
    furi_assert(context);
    RpcSession* session = (RpcSession*)context;
    Rpc* rpc = session->rpc;

    FURI_LOG_D(TAG, "Session started");

    while(1) {
        pb_istream_t istream = {
            .callback = rpc_pb_stream_read,
            .state = session,
            .errmsg = NULL,
            .bytes_left = SIZE_MAX,
        };

        bool message_decode_failed = false;

        if(pb_decode_ex(&istream, &PB_Main_msg, session->decoded_message, PB_DECODE_DELIMITED)) {
#ifdef SRV_RPC_DEBUG
            FURI_LOG_I(TAG, "INPUT:");
            rpc_debug_print_message(session->decoded_message);
#endif
            RpcHandler* handler =
                RpcHandlerDict_get(session->handlers, session->decoded_message->which_content);

            if(handler && handler->message_handler) {
                furi_check(furi_mutex_acquire(rpc->busy_mutex, FuriWaitForever) == FuriStatusOk);
                handler->message_handler(session->decoded_message, handler->context);
                furi_check(furi_mutex_release(rpc->busy_mutex) == FuriStatusOk);
            } else if(session->decoded_message->which_content == 0) {
                /* Receiving zeroes means message is 0-length, which
                 * is valid for proto3: all fields are filled with default values.
                 * 0 - is default value for which_content field.
                 * Mark it as decode error, because there is no content message
                 * in Main message with tag 0.
                 */
                message_decode_failed = true;
            } else if(!handler && !session->terminate) {
                FURI_LOG_E(
                    TAG,
                    "Message(%d) decoded, but not implemented",
                    session->decoded_message->which_content);
                rpc_send_and_release_empty(
                    session,
                    session->decoded_message->command_id,
                    PB_CommandStatus_ERROR_NOT_IMPLEMENTED);
            }
        } else {
            message_decode_failed = true;
        }

        if(message_decode_failed) {
            furi_stream_buffer_reset(session->stream);
            if(!session->terminate) {
                /* Protobuf can't determine start and end of message.
                 * Handle this by adding varint at beginning
                 * of a message (PB_ENCODE_DELIMITED). But decoding fail
                 * means we can't be sure next bytes are varint for next
                 * message, so the only way to close session.
                 * RPC itself can't make decision to close session. It has
                 * to notify:
                 * 1) down layer (transport)
                 * 2) other side (companion app)
                 * Who are responsible to handle RPC session lifecycle.
                 * Companion receives 2 messages: ERROR_DECODE and session_closed.
                 */
                FURI_LOG_E(TAG, "Decode failed, error: \'%.128s\'", PB_GET_ERROR(&istream));
                session->decode_error = true;
                rpc_send_and_release_empty(session, 0, PB_CommandStatus_ERROR_DECODE);
                furi_mutex_acquire(session->callbacks_mutex, FuriWaitForever);
                if(session->closed_callback) {
                    session->closed_callback(session->context);
                }
                furi_mutex_release(session->callbacks_mutex);

                if(session->owner == RpcOwnerBle) {
                    // Disconnect BLE session
                    FURI_LOG_E("RPC", "BLE session closed due to a decode error");
                    Bt* bt = furi_record_open(RECORD_BT);
                    bt_profile_restore_default(bt);
                    furi_record_close(RECORD_BT);
                    FURI_LOG_E("RPC", "Finished disconnecting the BLE session");
                }
            }
        }

        pb_release(&PB_Main_msg, session->decoded_message);

        if(session->terminate) {
            FURI_LOG_D(TAG, "Session terminated");
            break;
        }
    }

    return 0;
}

static void rpc_session_thread_pending_callback(void* context, uint32_t arg) {
    UNUSED(arg);
    RpcSession* session = (RpcSession*)context;

    for(size_t i = 0; i < COUNT_OF(rpc_systems); ++i) {
        if(rpc_systems[i].free) {
            (rpc_systems[i].free)(session->system_contexts[i]);
        }
    }
    free(session->system_contexts);
    free(session->decoded_message);
    RpcHandlerDict_clear(session->handlers);
    furi_stream_buffer_free(session->stream);

    furi_mutex_acquire(session->callbacks_mutex, FuriWaitForever);
    if(session->terminated_callback) {
        session->terminated_callback(session->context);
    }
    furi_mutex_release(session->callbacks_mutex);

    furi_mutex_free(session->callbacks_mutex);
    furi_thread_join(session->thread);
    furi_thread_free(session->thread);
    free(session);
}

static void
    rpc_session_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    UNUSED(thread);
    if(state == FuriThreadStateStopped) {
        furi_timer_pending_callback(rpc_session_thread_pending_callback, context, 0);
    }
}

RpcSession* rpc_session_open(Rpc* rpc, RpcOwner owner) {
    furi_check(rpc);

    RpcSession* session = malloc(sizeof(RpcSession));
    session->callbacks_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    session->stream = furi_stream_buffer_alloc(RPC_BUFFER_SIZE, 1);
    session->rpc = rpc;
    session->terminate = false;
    session->decode_error = false;
    session->owner = owner;
    RpcHandlerDict_init(session->handlers);

    session->decoded_message = malloc(sizeof(PB_Main));
    session->decoded_message->cb_content.funcs.decode = rpc_pb_content_callback;
    session->decoded_message->cb_content.arg = session;

    session->system_contexts = malloc(COUNT_OF(rpc_systems) * sizeof(void*));
    for(size_t i = 0; i < COUNT_OF(rpc_systems); ++i) {
        session->system_contexts[i] = rpc_systems[i].alloc(session);
    }

    RpcHandler rpc_handler = {
        .message_handler = rpc_close_session_process,
        .decode_submessage = NULL,
        .context = session,
    };
    rpc_add_handler(session, PB_Main_stop_session_tag, &rpc_handler);

    session->thread = furi_thread_alloc_ex("RpcSessionWorker", 3072, rpc_session_worker, session);

    furi_thread_set_state_context(session->thread, session);
    furi_thread_set_state_callback(session->thread, rpc_session_thread_state_callback);

    furi_thread_start(session->thread);

    return session;
}

void rpc_session_close(RpcSession* session) {
    furi_check(session);
    furi_check(session->rpc);

    rpc_session_set_send_bytes_callback(session, NULL);
    rpc_session_set_close_callback(session, NULL);
    rpc_session_set_buffer_is_empty_callback(session, NULL);
    furi_thread_flags_set(furi_thread_get_id(session->thread), RpcEvtDisconnect);
}

void rpc_on_system_start(void* p) {
    UNUSED(p);
    Rpc* rpc = malloc(sizeof(Rpc));

    rpc->busy_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(
        cli, "start_rpc_session", CliCommandFlagParallelSafe, rpc_cli_command_start_session, rpc);

    furi_record_create(RECORD_RPC, rpc);
}

void rpc_add_handler(RpcSession* session, pb_size_t message_tag, RpcHandler* handler) {
    furi_assert(RpcHandlerDict_get(session->handlers, message_tag) == NULL);

    RpcHandlerDict_set_at(session->handlers, message_tag, *handler);
}

void rpc_send(RpcSession* session, PB_Main* message) {
    furi_assert(session);
    furi_assert(message);

    pb_ostream_t ostream = PB_OSTREAM_SIZING;

#ifdef SRV_RPC_DEBUG
    FURI_LOG_I(TAG, "OUTPUT:");
    rpc_debug_print_message(message);
#endif

    bool result = pb_encode_ex(&ostream, &PB_Main_msg, message, PB_ENCODE_DELIMITED);
    furi_check(result && ostream.bytes_written);

    uint8_t* buffer = malloc(ostream.bytes_written);
    ostream = pb_ostream_from_buffer(buffer, ostream.bytes_written);

    pb_encode_ex(&ostream, &PB_Main_msg, message, PB_ENCODE_DELIMITED);

#ifdef SRV_RPC_DEBUG
    rpc_debug_print_data("OUTPUT", buffer, ostream.bytes_written);
#endif

    furi_mutex_acquire(session->callbacks_mutex, FuriWaitForever);
    if(session->send_bytes_callback) {
        session->send_bytes_callback(session->context, buffer, ostream.bytes_written);
    }
    furi_mutex_release(session->callbacks_mutex);

    free(buffer);
}

void rpc_send_and_release(RpcSession* session, PB_Main* message) {
    rpc_send(session, message);
    pb_release(&PB_Main_msg, message);
}

void rpc_send_and_release_empty(RpcSession* session, uint32_t command_id, PB_CommandStatus status) {
    furi_assert(session);

    PB_Main message = {
        .command_id = command_id,
        .command_status = status,
        .has_next = false,
        .which_content = PB_Main_empty_tag,
    };

    rpc_send_and_release(session, &message);
    pb_release(&PB_Main_msg, &message);
}
