#include "rpc_i.h"

#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include <storage.pb.h>
#include <flipper.pb.h>
#include <portmacro.h>

#include <furi.h>

#include <cli/cli.h>
#include <stdint.h>
#include <stdio.h>
#include <stream_buffer.h>
#include <m-string.h>
#include <m-dict.h>

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
    }};

struct RpcSession {
    Rpc* rpc;

    FuriThread* thread;

    RpcHandlerDict_t handlers;
    StreamBufferHandle_t stream;
    PB_Main* decoded_message;
    bool terminate;
    void** system_contexts;
    bool decode_error;

    osMutexId_t callbacks_mutex;
    RpcSendBytesCallback send_bytes_callback;
    RpcBufferIsEmptyCallback buffer_is_empty_callback;
    RpcSessionClosedCallback closed_callback;
    RpcSessionTerminatedCallback terminated_callback;
    void* context;
};

struct Rpc {
    osMutexId_t busy_mutex;
};

static bool content_callback(pb_istream_t* stream, const pb_field_t* field, void** arg);

static void rpc_close_session_process(const PB_Main* request, void* context) {
    furi_assert(request);

    RpcSession* session = (RpcSession*)context;
    furi_assert(session);

    rpc_send_and_release_empty(session, request->command_id, PB_CommandStatus_OK);
    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    if(session->closed_callback) {
        session->closed_callback(session->context);
    } else {
        FURI_LOG_W(TAG, "Session stop isn't processed by transport layer");
    }
    osMutexRelease(session->callbacks_mutex);
}

static size_t rpc_sprintf_msg_file(
    string_t str,
    const char* prefix,
    const PB_Storage_File* msg_file,
    size_t msg_files_size) {
    size_t cnt = 0;

    for(size_t i = 0; i < msg_files_size; ++i, ++msg_file) {
        string_cat_printf(
            str,
            "%s[%c] size: %5ld",
            prefix,
            msg_file->type == PB_Storage_File_FileType_DIR ? 'd' : 'f',
            msg_file->size);

        if(msg_file->name) {
            string_cat_printf(str, " \'%s\'", msg_file->name);
        }

        if(msg_file->data && msg_file->data->size) {
            string_cat_printf(
                str,
                " (%d):\'%.*s%s\'",
                msg_file->data->size,
                MIN(msg_file->data->size, 30),
                msg_file->data->bytes,
                msg_file->data->size > 30 ? "..." : "");
        }

        string_cat_printf(str, "\r\n");
    }

    return cnt;
}

void rpc_print_data(const char* prefix, uint8_t* buffer, size_t size) {
    string_t str;
    string_init(str);
    string_reserve(str, 100 + size * 5);

    string_cat_printf(str, "\r\n%s DEC(%d): {", prefix, size);
    for(size_t i = 0; i < size; ++i) {
        string_cat_printf(str, "%d, ", buffer[i]);
    }
    string_cat_printf(str, "}\r\n");

    printf("%s", string_get_cstr(str));
    string_reset(str);
    string_reserve(str, 100 + size * 3);

    string_cat_printf(str, "%s HEX(%d): {", prefix, size);
    for(size_t i = 0; i < size; ++i) {
        string_cat_printf(str, "%02X", buffer[i]);
    }
    string_cat_printf(str, "}\r\n\r\n");

    printf("%s", string_get_cstr(str));
    string_clear(str);
}

void rpc_print_message(const PB_Main* message) {
    string_t str;
    string_init(str);

    string_cat_printf(
        str,
        "PB_Main: {\r\n\tresult: %d cmd_id: %ld (%s)\r\n",
        message->command_status,
        message->command_id,
        message->has_next ? "has_next" : "last");
    switch(message->which_content) {
    default:
        /* not implemented yet */
        string_cat_printf(str, "\tNOT_IMPLEMENTED (%d) {\r\n", message->which_content);
        break;
    case PB_Main_stop_session_tag:
        string_cat_printf(str, "\tstop_session {\r\n");
        break;
    case PB_Main_app_start_request_tag: {
        string_cat_printf(str, "\tapp_start {\r\n");
        const char* name = message->content.app_start_request.name;
        const char* args = message->content.app_start_request.args;
        if(name) {
            string_cat_printf(str, "\t\tname: %s\r\n", name);
        }
        if(args) {
            string_cat_printf(str, "\t\targs: %s\r\n", args);
        }
        break;
    }
    case PB_Main_app_lock_status_request_tag: {
        string_cat_printf(str, "\tapp_lock_status_request {\r\n");
        break;
    }
    case PB_Main_app_lock_status_response_tag: {
        string_cat_printf(str, "\tapp_lock_status_response {\r\n");
        bool lock_status = message->content.app_lock_status_response.locked;
        string_cat_printf(str, "\t\tlocked: %s\r\n", lock_status ? "true" : "false");
        break;
    }
    case PB_Main_storage_md5sum_request_tag: {
        string_cat_printf(str, "\tmd5sum_request {\r\n");
        const char* path = message->content.storage_md5sum_request.path;
        if(path) {
            string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_storage_md5sum_response_tag: {
        string_cat_printf(str, "\tmd5sum_response {\r\n");
        const char* path = message->content.storage_md5sum_response.md5sum;
        if(path) {
            string_cat_printf(str, "\t\tmd5sum: %s\r\n", path);
        }
        break;
    }
    case PB_Main_system_ping_request_tag:
        string_cat_printf(str, "\tping_request {\r\n");
        break;
    case PB_Main_system_ping_response_tag:
        string_cat_printf(str, "\tping_response {\r\n");
        break;
    case PB_Main_system_device_info_request_tag:
        string_cat_printf(str, "\tdevice_info_request {\r\n");
        break;
    case PB_Main_system_device_info_response_tag:
        string_cat_printf(str, "\tdevice_info_response {\r\n");
        string_cat_printf(
            str,
            "\t\t%s: %s\r\n",
            message->content.system_device_info_response.key,
            message->content.system_device_info_response.value);
        break;
    case PB_Main_storage_mkdir_request_tag:
        string_cat_printf(str, "\tmkdir {\r\n");
        break;
    case PB_Main_storage_delete_request_tag: {
        string_cat_printf(str, "\tdelete {\r\n");
        const char* path = message->content.storage_delete_request.path;
        if(path) {
            string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_empty_tag:
        string_cat_printf(str, "\tempty {\r\n");
        break;
    case PB_Main_storage_info_request_tag: {
        string_cat_printf(str, "\tinfo_request {\r\n");
        const char* path = message->content.storage_info_request.path;
        if(path) {
            string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_storage_info_response_tag: {
        string_cat_printf(str, "\tinfo_response {\r\n");
        string_cat_printf(
            str, "\t\ttotal_space: %lu\r\n", message->content.storage_info_response.total_space);
        string_cat_printf(
            str, "\t\tfree_space: %lu\r\n", message->content.storage_info_response.free_space);
        break;
    }
    case PB_Main_storage_stat_request_tag: {
        string_cat_printf(str, "\tstat_request {\r\n");
        const char* path = message->content.storage_stat_request.path;
        if(path) {
            string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_storage_stat_response_tag: {
        string_cat_printf(str, "\tstat_response {\r\n");
        if(message->content.storage_stat_response.has_file) {
            const PB_Storage_File* msg_file = &message->content.storage_stat_response.file;
            rpc_sprintf_msg_file(str, "\t\t\t", msg_file, 1);
        }
        break;
    }
    case PB_Main_storage_list_request_tag: {
        string_cat_printf(str, "\tlist_request {\r\n");
        const char* path = message->content.storage_list_request.path;
        if(path) {
            string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_storage_read_request_tag: {
        string_cat_printf(str, "\tread_request {\r\n");
        const char* path = message->content.storage_read_request.path;
        if(path) {
            string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        break;
    }
    case PB_Main_storage_write_request_tag: {
        string_cat_printf(str, "\twrite_request {\r\n");
        const char* path = message->content.storage_write_request.path;
        if(path) {
            string_cat_printf(str, "\t\tpath: %s\r\n", path);
        }
        if(message->content.storage_write_request.has_file) {
            const PB_Storage_File* msg_file = &message->content.storage_write_request.file;
            rpc_sprintf_msg_file(str, "\t\t\t", msg_file, 1);
        }
        break;
    }
    case PB_Main_storage_read_response_tag:
        string_cat_printf(str, "\tread_response {\r\n");
        if(message->content.storage_read_response.has_file) {
            const PB_Storage_File* msg_file = &message->content.storage_read_response.file;
            rpc_sprintf_msg_file(str, "\t\t\t", msg_file, 1);
        }
        break;
    case PB_Main_storage_list_response_tag: {
        const PB_Storage_File* msg_file = message->content.storage_list_response.file;
        size_t msg_file_count = message->content.storage_list_response.file_count;
        string_cat_printf(str, "\tlist_response {\r\n");
        rpc_sprintf_msg_file(str, "\t\t", msg_file, msg_file_count);
        break;
    }
    case PB_Main_storage_rename_request_tag: {
        string_cat_printf(str, "\trename_request {\r\n");
        string_cat_printf(
            str, "\t\told_path: %s\r\n", message->content.storage_rename_request.old_path);
        string_cat_printf(
            str, "\t\tnew_path: %s\r\n", message->content.storage_rename_request.new_path);
        break;
    }
    case PB_Main_gui_start_screen_stream_request_tag:
        string_cat_printf(str, "\tstart_screen_stream {\r\n");
        break;
    case PB_Main_gui_stop_screen_stream_request_tag:
        string_cat_printf(str, "\tstop_screen_stream {\r\n");
        break;
    case PB_Main_gui_screen_frame_tag:
        string_cat_printf(str, "\tscreen_frame {\r\n");
        break;
    case PB_Main_gui_send_input_event_request_tag:
        string_cat_printf(str, "\tsend_input_event {\r\n");
        string_cat_printf(
            str, "\t\tkey: %d\r\n", message->content.gui_send_input_event_request.key);
        string_cat_printf(
            str, "\t\type: %d\r\n", message->content.gui_send_input_event_request.type);
        break;
    case PB_Main_gui_start_virtual_display_request_tag:
        string_cat_printf(str, "\tstart_virtual_display {\r\n");
        break;
    case PB_Main_gui_stop_virtual_display_request_tag:
        string_cat_printf(str, "\tstop_virtual_display {\r\n");
        break;
    }
    string_cat_printf(str, "\t}\r\n}\r\n");
    printf("%s", string_get_cstr(str));

    string_clear(str);
}

void rpc_session_set_context(RpcSession* session, void* context) {
    furi_assert(session);

    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    session->context = context;
    osMutexRelease(session->callbacks_mutex);
}

void rpc_session_set_close_callback(RpcSession* session, RpcSessionClosedCallback callback) {
    furi_assert(session);

    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    session->closed_callback = callback;
    osMutexRelease(session->callbacks_mutex);
}

void rpc_session_set_send_bytes_callback(RpcSession* session, RpcSendBytesCallback callback) {
    furi_assert(session);

    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    session->send_bytes_callback = callback;
    osMutexRelease(session->callbacks_mutex);
}

void rpc_session_set_buffer_is_empty_callback(
    RpcSession* session,
    RpcBufferIsEmptyCallback callback) {
    furi_assert(session);

    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    session->buffer_is_empty_callback = callback;
    osMutexRelease(session->callbacks_mutex);
}

void rpc_session_set_terminated_callback(
    RpcSession* session,
    RpcSessionTerminatedCallback callback) {
    furi_assert(session);

    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    session->terminated_callback = callback;
    osMutexRelease(session->callbacks_mutex);
}

/* Doesn't forbid using rpc_feed_bytes() after session close - it's safe.
 * Because any bytes received in buffer will be flushed before next session.
 * If bytes get into stream buffer before it's get epmtified and this
 * command is gets processed - it's safe either. But case of it is quite
 * odd: client sends close request and sends command after.
 */
size_t
    rpc_session_feed(RpcSession* session, uint8_t* encoded_bytes, size_t size, TickType_t timeout) {
    furi_assert(session);
    size_t bytes_sent = xStreamBufferSend(session->stream, encoded_bytes, size, timeout);

    furi_thread_flags_set(furi_thread_get_id(session->thread), RpcEvtNewData);

    return bytes_sent;
}

size_t rpc_session_get_available_size(RpcSession* session) {
    furi_assert(session);
    return xStreamBufferSpacesAvailable(session->stream);
}

bool rpc_pb_stream_read(pb_istream_t* istream, pb_byte_t* buf, size_t count) {
    RpcSession* session = istream->state;
    furi_assert(session);
    furi_assert(istream->bytes_left);

    uint32_t flags = 0;
    size_t bytes_received = 0;

    while(1) {
        bytes_received +=
            xStreamBufferReceive(session->stream, buf + bytes_received, count - bytes_received, 0);
        if(xStreamBufferIsEmpty(session->stream)) {
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
            flags = furi_thread_flags_wait(RPC_ALL_EVENTS, osFlagsWaitAny, osWaitForever);
            if(flags & RpcEvtDisconnect) {
                if(xStreamBufferIsEmpty(session->stream)) {
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

#if SRV_RPC_DEBUG
    rpc_print_data("INPUT", buf, bytes_received);
#endif

    return (count == bytes_received);
}

static bool content_callback(pb_istream_t* stream, const pb_field_t* field, void** arg) {
    furi_assert(stream);
    RpcSession* session = stream->state;
    furi_assert(session);

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
            .bytes_left = RPC_MAX_MESSAGE_SIZE, /* max incoming message size */
        };

        bool message_decode_failed = false;

        if(pb_decode_ex(&istream, &PB_Main_msg, session->decoded_message, PB_DECODE_DELIMITED)) {
#if SRV_RPC_DEBUG
            FURI_LOG_I(TAG, "INPUT:");
            rpc_print_message(session->decoded_message);
#endif
            RpcHandler* handler =
                RpcHandlerDict_get(session->handlers, session->decoded_message->which_content);

            if(handler && handler->message_handler) {
                furi_check(osMutexAcquire(rpc->busy_mutex, osWaitForever) == osOK);
                handler->message_handler(session->decoded_message, handler->context);
                furi_check(osMutexRelease(rpc->busy_mutex) == osOK);
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
            xStreamBufferReset(session->stream);
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
                osMutexAcquire(session->callbacks_mutex, osWaitForever);
                if(session->closed_callback) {
                    session->closed_callback(session->context);
                }
                osMutexRelease(session->callbacks_mutex);
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

static void rpc_session_free_callback(FuriThreadState thread_state, void* context) {
    furi_assert(context);

    RpcSession* session = (RpcSession*)context;

    if(thread_state == FuriThreadStateStopped) {
        for(size_t i = 0; i < COUNT_OF(rpc_systems); ++i) {
            if(rpc_systems[i].free) {
                rpc_systems[i].free(session->system_contexts[i]);
            }
        }
        free(session->system_contexts);
        free(session->decoded_message);
        RpcHandlerDict_clear(session->handlers);
        vStreamBufferDelete(session->stream);

        osMutexAcquire(session->callbacks_mutex, osWaitForever);
        if(session->terminated_callback) {
            session->terminated_callback(session->context);
        }
        osMutexRelease(session->callbacks_mutex);

        osMutexDelete(session->callbacks_mutex);
        furi_thread_free(session->thread);
        free(session);
    }
}

RpcSession* rpc_session_open(Rpc* rpc) {
    furi_assert(rpc);

    RpcSession* session = malloc(sizeof(RpcSession));
    session->callbacks_mutex = osMutexNew(NULL);
    session->stream = xStreamBufferCreate(RPC_BUFFER_SIZE, 1);
    session->rpc = rpc;
    session->terminate = false;
    session->decode_error = false;
    RpcHandlerDict_init(session->handlers);

    session->decoded_message = malloc(sizeof(PB_Main));
    session->decoded_message->cb_content.funcs.decode = content_callback;
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

    session->thread = furi_thread_alloc();
    furi_thread_set_name(session->thread, "RpcSessionWorker");
    furi_thread_set_stack_size(session->thread, 2048);
    furi_thread_set_context(session->thread, session);
    furi_thread_set_callback(session->thread, rpc_session_worker);

    furi_thread_set_state_context(session->thread, session);
    furi_thread_set_state_callback(session->thread, rpc_session_free_callback);

    furi_thread_start(session->thread);

    return session;
}

void rpc_session_close(RpcSession* session) {
    furi_assert(session);
    furi_assert(session->rpc);

    rpc_session_set_send_bytes_callback(session, NULL);
    rpc_session_set_close_callback(session, NULL);
    rpc_session_set_buffer_is_empty_callback(session, NULL);
    furi_thread_flags_set(furi_thread_get_id(session->thread), RpcEvtDisconnect);
}

int32_t rpc_srv(void* p) {
    UNUSED(p);
    Rpc* rpc = malloc(sizeof(Rpc));

    rpc->busy_mutex = osMutexNew(NULL);

    Cli* cli = furi_record_open("cli");
    cli_add_command(
        cli, "start_rpc_session", CliCommandFlagParallelSafe, rpc_cli_command_start_session, rpc);

    furi_record_create("rpc", rpc);

    return 0;
}

void rpc_add_handler(RpcSession* session, pb_size_t message_tag, RpcHandler* handler) {
    furi_assert(RpcHandlerDict_get(session->handlers, message_tag) == NULL);

    RpcHandlerDict_set_at(session->handlers, message_tag, *handler);
}

void rpc_send(RpcSession* session, PB_Main* message) {
    furi_assert(session);
    furi_assert(message);

    pb_ostream_t ostream = PB_OSTREAM_SIZING;

#if SRV_RPC_DEBUG
    FURI_LOG_I(TAG, "OUTPUT:");
    rpc_print_message(message);
#endif

    bool result = pb_encode_ex(&ostream, &PB_Main_msg, message, PB_ENCODE_DELIMITED);
    furi_check(result && ostream.bytes_written);

    uint8_t* buffer = malloc(ostream.bytes_written);
    ostream = pb_ostream_from_buffer(buffer, ostream.bytes_written);

    pb_encode_ex(&ostream, &PB_Main_msg, message, PB_ENCODE_DELIMITED);

#if SRV_RPC_DEBUG
    rpc_print_data("OUTPUT", buffer, ostream.bytes_written);
#endif

    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    if(session->send_bytes_callback) {
        session->send_bytes_callback(session->context, buffer, ostream.bytes_written);
    }
    osMutexRelease(session->callbacks_mutex);

    free(buffer);
}

void rpc_send_and_release(RpcSession* session, PB_Main* message) {
    rpc_send(session, message);
    pb_release(&PB_Main_msg, message);
}

void rpc_send_and_release_empty(RpcSession* session, uint32_t command_id, PB_CommandStatus status) {
    PB_Main message = {
        .command_id = command_id,
        .command_status = status,
        .has_next = false,
        .which_content = PB_Main_empty_tag,
    };
    rpc_send_and_release(session, &message);
    pb_release(&PB_Main_msg, &message);
}
