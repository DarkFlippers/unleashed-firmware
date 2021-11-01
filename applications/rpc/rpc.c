#include "rpc_i.h"
#include <pb.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <status.pb.h>
#include <storage.pb.h>
#include <flipper.pb.h>
#include <cmsis_os.h>
#include <cmsis_os2.h>
#include <portmacro.h>
#include <furi.h>
#include <cli/cli.h>
#include <stdint.h>
#include <stdio.h>
#include <stream_buffer.h>
#include <m-string.h>
#include <m-dict.h>

#define RPC_TAG "RPC"

#define RPC_EVENT_NEW_DATA (1 << 0)
#define RPC_EVENT_DISCONNECT (1 << 1)
#define RPC_EVENTS_ALL (RPC_EVENT_DISCONNECT | RPC_EVENT_NEW_DATA)

#define DEBUG_PRINT 0

DICT_DEF2(RpcHandlerDict, pb_size_t, M_DEFAULT_OPLIST, RpcHandler, M_POD_OPLIST)

typedef struct {
    RpcSystemAlloc alloc;
    RpcSystemFree free;
    void* context;
} RpcSystemCallbacks;

static RpcSystemCallbacks rpc_systems[] = {
    {
        .alloc = rpc_system_status_alloc,
        .free = NULL,
    },
    {
        .alloc = rpc_system_storage_alloc,
        .free = rpc_system_storage_free,
    },
    {
        .alloc = rpc_system_app_alloc,
        .free = NULL,
    },
    {
        .alloc = rpc_system_gui_alloc,
        .free = rpc_system_gui_free,
    },
};

struct RpcSession {
    RpcSendBytesCallback send_bytes_callback;
    RpcSessionClosedCallback closed_callback;
    void* context;
    osMutexId_t callbacks_mutex;
    Rpc* rpc;
    bool terminate;
    void** system_contexts;
};

struct Rpc {
    bool busy;
    osMutexId_t busy_mutex;
    RpcSession session;
    osEventFlagsId_t events;
    StreamBufferHandle_t stream;
    RpcHandlerDict_t handlers;
    PB_Main* decoded_message;
};

static bool content_callback(pb_istream_t* stream, const pb_field_t* field, void** arg);

static void rpc_close_session_process(const PB_Main* msg_request, void* context) {
    furi_assert(msg_request);
    furi_assert(context);

    Rpc* rpc = context;
    rpc_send_and_release_empty(rpc, msg_request->command_id, PB_CommandStatus_OK);

    osMutexAcquire(rpc->session.callbacks_mutex, osWaitForever);
    if(rpc->session.closed_callback) {
        rpc->session.closed_callback(rpc->session.context);
    }
    osMutexRelease(rpc->session.callbacks_mutex);
}

static size_t rpc_sprintf_msg_file(
    string_t str,
    const char* prefix,
    const PB_Storage_File* msg_file,
    size_t msg_files_size) {
    size_t cnt = 0;

    for(int i = 0; i < msg_files_size; ++i, ++msg_file) {
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
    for(int i = 0; i < size; ++i) {
        string_cat_printf(str, "%d, ", buffer[i]);
    }
    string_cat_printf(str, "}\r\n");

    printf("%s", string_get_cstr(str));
    string_clean(str);
    string_reserve(str, 100 + size * 3);

    string_cat_printf(str, "%s HEX(%d): {", prefix, size);
    for(int i = 0; i < size; ++i) {
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
    case PB_Main_ping_request_tag:
        string_cat_printf(str, "\tping_request {\r\n");
        break;
    case PB_Main_ping_response_tag:
        string_cat_printf(str, "\tping_response {\r\n");
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
    }
    case PB_Main_gui_start_screen_stream_request_tag:
        string_cat_printf(str, "\tstart_screen_stream {\r\n");
        break;
    case PB_Main_gui_stop_screen_stream_request_tag:
        string_cat_printf(str, "\tstop_screen_stream {\r\n");
        break;
    case PB_Main_gui_screen_stream_frame_tag:
        string_cat_printf(str, "\tscreen_stream_frame {\r\n");
        break;
    case PB_Main_gui_send_input_event_request_tag:
        string_cat_printf(str, "\tsend_input_event {\r\n");
        string_cat_printf(
            str, "\t\tkey: %d\r\n", message->content.gui_send_input_event_request.key);
        string_cat_printf(
            str, "\t\type: %d\r\n", message->content.gui_send_input_event_request.type);
        break;
    }
    string_cat_printf(str, "\t}\r\n}\r\n");
    printf("%s", string_get_cstr(str));

    string_clear(str);
}

static Rpc* rpc_alloc(void) {
    Rpc* rpc = furi_alloc(sizeof(Rpc));
    rpc->busy_mutex = osMutexNew(NULL);
    rpc->busy = false;
    rpc->events = osEventFlagsNew(NULL);
    rpc->stream = xStreamBufferCreate(256, 1);

    rpc->decoded_message = furi_alloc(sizeof(PB_Main));
    rpc->decoded_message->cb_content.funcs.decode = content_callback;
    rpc->decoded_message->cb_content.arg = rpc;

    RpcHandlerDict_init(rpc->handlers);

    return rpc;
}

RpcSession* rpc_session_open(Rpc* rpc) {
    furi_assert(rpc);
    bool result = false;
    furi_check(osMutexAcquire(rpc->busy_mutex, osWaitForever) == osOK);
    if(rpc->busy) {
        result = false;
    } else {
        rpc->busy = true;
        result = true;
    }
    furi_check(osMutexRelease(rpc->busy_mutex) == osOK);

    if(result) {
        RpcSession* session = &rpc->session;
        session->callbacks_mutex = osMutexNew(NULL);
        session->rpc = rpc;
        session->terminate = false;
        xStreamBufferReset(rpc->stream);

        session->system_contexts = furi_alloc(COUNT_OF(rpc_systems) * sizeof(void*));
        for(int i = 0; i < COUNT_OF(rpc_systems); ++i) {
            session->system_contexts[i] = rpc_systems[i].alloc(rpc);
        }

        RpcHandler rpc_handler = {
            .message_handler = rpc_close_session_process,
            .decode_submessage = NULL,
            .context = rpc,
        };
        rpc_add_handler(rpc, PB_Main_stop_session_tag, &rpc_handler);

        FURI_LOG_D(RPC_TAG, "Session started\r\n");
    }

    return result ? &rpc->session : NULL; /* support 1 open session for now */
}

void rpc_session_close(RpcSession* session) {
    furi_assert(session);
    furi_assert(session->rpc);
    furi_assert(session->rpc->busy);

    rpc_session_set_send_bytes_callback(session, NULL);
    rpc_session_set_close_callback(session, NULL);
    osEventFlagsSet(session->rpc->events, RPC_EVENT_DISCONNECT);
}

static void rpc_free_session(RpcSession* session) {
    furi_assert(session);

    for(int i = 0; i < COUNT_OF(rpc_systems); ++i) {
        if(rpc_systems[i].free) {
            rpc_systems[i].free(session->system_contexts[i]);
        }
    }
    free(session->system_contexts);
    osMutexDelete(session->callbacks_mutex);
    RpcHandlerDict_clean(session->rpc->handlers);

    session->context = NULL;
    session->closed_callback = NULL;
    session->send_bytes_callback = NULL;
}

void rpc_session_set_context(RpcSession* session, void* context) {
    furi_assert(session);
    furi_assert(session->rpc);
    furi_assert(session->rpc->busy);

    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    session->context = context;
    osMutexRelease(session->callbacks_mutex);
}

void rpc_session_set_close_callback(RpcSession* session, RpcSessionClosedCallback callback) {
    furi_assert(session);
    furi_assert(session->rpc);
    furi_assert(session->rpc->busy);

    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    session->closed_callback = callback;
    osMutexRelease(session->callbacks_mutex);
}

void rpc_session_set_send_bytes_callback(RpcSession* session, RpcSendBytesCallback callback) {
    furi_assert(session);
    furi_assert(session->rpc);
    furi_assert(session->rpc->busy);

    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    session->send_bytes_callback = callback;
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
    Rpc* rpc = session->rpc;
    furi_assert(rpc->busy);

    size_t bytes_sent = xStreamBufferSend(rpc->stream, encoded_bytes, size, timeout);
    osEventFlagsSet(rpc->events, RPC_EVENT_NEW_DATA);

    return bytes_sent;
}

bool rpc_pb_stream_read(pb_istream_t* istream, pb_byte_t* buf, size_t count) {
    Rpc* rpc = istream->state;
    uint32_t flags = 0;
    size_t bytes_received = 0;

    furi_assert(istream->bytes_left);

    while(1) {
        bytes_received +=
            xStreamBufferReceive(rpc->stream, buf + bytes_received, count - bytes_received, 0);
        if(count == bytes_received) {
            break;
        } else {
            flags = osEventFlagsWait(rpc->events, RPC_EVENTS_ALL, 0, osWaitForever);
            if(flags & RPC_EVENT_DISCONNECT) {
                if(xStreamBufferIsEmpty(rpc->stream)) {
                    rpc->session.terminate = true;
                    istream->bytes_left = 0;
                    bytes_received = 0;
                    break;
                } else {
                    /* Save disconnect flag and continue reading buffer */
                    osEventFlagsSet(rpc->events, RPC_EVENT_DISCONNECT);
                }
            }
        }
    }

#if DEBUG_PRINT
    rpc_print_data("INPUT", buf, bytes_received);
#endif

    return (count == bytes_received);
}

void rpc_send_and_release(Rpc* rpc, PB_Main* message) {
    furi_assert(rpc);
    furi_assert(message);
    RpcSession* session = &rpc->session;
    pb_ostream_t ostream = PB_OSTREAM_SIZING;

#if DEBUG_PRINT
    FURI_LOG_I(RPC_TAG, "OUTPUT:");
    rpc_print_message(message);
#endif

    bool result = pb_encode_ex(&ostream, &PB_Main_msg, message, PB_ENCODE_DELIMITED);
    furi_check(result && ostream.bytes_written);

    uint8_t* buffer = furi_alloc(ostream.bytes_written);
    ostream = pb_ostream_from_buffer(buffer, ostream.bytes_written);

    pb_encode_ex(&ostream, &PB_Main_msg, message, PB_ENCODE_DELIMITED);

#if DEBUG_PRINT
    rpc_print_data("OUTPUT", buffer, ostream.bytes_written);
#endif

    osMutexAcquire(session->callbacks_mutex, osWaitForever);
    if(session->send_bytes_callback) {
        session->send_bytes_callback(session->context, buffer, ostream.bytes_written);
    }
    osMutexRelease(session->callbacks_mutex);

    free(buffer);
    pb_release(&PB_Main_msg, message);
}

static bool content_callback(pb_istream_t* stream, const pb_field_t* field, void** arg) {
    furi_assert(stream);
    Rpc* rpc = *arg;

    RpcHandler* handler = RpcHandlerDict_get(rpc->handlers, field->tag);

    if(handler && handler->decode_submessage) {
        handler->decode_submessage(stream, field, arg);
    }

    return true;
}

int32_t rpc_srv(void* p) {
    Rpc* rpc = rpc_alloc();
    furi_record_create("rpc", rpc);

    Cli* cli = furi_record_open("cli");

    cli_add_command(
        cli, "start_rpc_session", CliCommandFlagParallelSafe, rpc_cli_command_start_session, rpc);

    while(1) {
        pb_istream_t istream = {
            .callback = rpc_pb_stream_read,
            .state = rpc,
            .errmsg = NULL,
            .bytes_left = 1024, /* max incoming message size */
        };

        if(pb_decode_ex(&istream, &PB_Main_msg, rpc->decoded_message, PB_DECODE_DELIMITED)) {
#if DEBUG_PRINT
            FURI_LOG_I(RPC_TAG, "INPUT:");
            rpc_print_message(rpc->decoded_message);
#endif
            RpcHandler* handler =
                RpcHandlerDict_get(rpc->handlers, rpc->decoded_message->which_content);

            if(handler && handler->message_handler) {
                handler->message_handler(rpc->decoded_message, handler->context);
            } else if(!handler && !rpc->session.terminate) {
                FURI_LOG_E(
                    RPC_TAG, "Unhandled message, tag: %d", rpc->decoded_message->which_content);
            }
        } else {
            xStreamBufferReset(rpc->stream);
            if(!rpc->session.terminate) {
                FURI_LOG_E(RPC_TAG, "Decode failed, error: \'%.128s\'", PB_GET_ERROR(&istream));
            }
        }

        pb_release(&PB_Main_msg, rpc->decoded_message);

        if(rpc->session.terminate) {
            FURI_LOG_D(RPC_TAG, "Session terminated");
            osEventFlagsClear(rpc->events, RPC_EVENTS_ALL);
            rpc_free_session(&rpc->session);
            rpc->busy = false;
        }
    }
    return 0;
}

void rpc_add_handler(Rpc* rpc, pb_size_t message_tag, RpcHandler* handler) {
    furi_assert(RpcHandlerDict_get(rpc->handlers, message_tag) == NULL);

    RpcHandlerDict_set_at(rpc->handlers, message_tag, *handler);
}

void rpc_send_and_release_empty(Rpc* rpc, uint32_t command_id, PB_CommandStatus status) {
    PB_Main message = {
        .command_id = command_id,
        .command_status = status,
        .has_next = false,
        .which_content = PB_Main_empty_tag,
    };
    rpc_send_and_release(rpc, &message);
    pb_release(&PB_Main_msg, &message);
}
