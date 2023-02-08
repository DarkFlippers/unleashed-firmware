#include "flipper.pb.h"
#include <core/check.h>
#include <core/record.h>
#include "pb_decode.h"
#include <rpc/rpc.h>
#include "rpc/rpc_i.h"
#include "storage.pb.h"
#include "storage/filesystem_api_defines.h"
#include "storage/storage.h"
#include <furi.h>
#include "../minunit.h"
#include <stdint.h>
#include <pb.h>
#include <pb_encode.h>
#include <m-list.h>
#include <lib/toolbox/md5.h>
#include <lib/toolbox/path.h>
#include <cli/cli.h>
#include <loader/loader.h>
#include <protobuf_version.h>
#include <semphr.h>

LIST_DEF(MsgList, PB_Main, M_POD_OPLIST)
#define M_OPL_MsgList_t() LIST_OPLIST(MsgList)

#define TEST_RPC_SESSIONS 2

/* MinUnit test framework doesn't allow passing context into tests,
 * so we have to use global variables
 */
static Rpc* rpc = NULL;
static uint32_t command_id = 0;

typedef struct {
    RpcSession* session;
    FuriStreamBuffer* output_stream;
    SemaphoreHandle_t close_session_semaphore;
    SemaphoreHandle_t terminate_semaphore;
    TickType_t timeout;
} RpcSessionContext;

static RpcSessionContext rpc_session[TEST_RPC_SESSIONS];

#define TAG "UnitTestsRpc"
#define MAX_RECEIVE_OUTPUT_TIMEOUT 3000
#define MAX_NAME_LENGTH 255
#define MAX_DATA_SIZE 512u // have to be exact as in rpc_storage.c
#define TEST_DIR TEST_DIR_NAME "/"
#define TEST_DIR_NAME EXT_PATH("unit_tests_tmp")
#define MD5SUM_SIZE 16

#define PING_REQUEST 0
#define PING_RESPONSE 1
#define WRITE_REQUEST 0
#define READ_RESPONSE 1

#define DEBUG_PRINT 0

#define BYTES(x) (x), sizeof(x)

#define DISABLE_TEST(code)  \
    do {                    \
        volatile int a = 0; \
        if(a) {             \
            code            \
        }                   \
    } while(0)

static void output_bytes_callback(void* ctx, uint8_t* got_bytes, size_t got_size);
static void clean_directory(Storage* fs_api, const char* clean_dir);
static void
    test_rpc_add_empty_to_list(MsgList_t msg_list, PB_CommandStatus status, uint32_t command_id);
static void test_rpc_encode_and_feed(MsgList_t msg_list, uint8_t session);
static void test_rpc_encode_and_feed_one(PB_Main* request, uint8_t session);
static void test_rpc_compare_messages(PB_Main* result, PB_Main* expected);
static void test_rpc_decode_and_compare(MsgList_t expected_msg_list, uint8_t session);
static void test_rpc_free_msg_list(MsgList_t msg_list);
static void test_rpc_session_close_callback(void* context);
static void test_rpc_session_terminated_callback(void* context);

static void test_rpc_setup(void) {
    furi_check(!rpc);
    furi_check(!(rpc_session[0].session));

    rpc = furi_record_open(RECORD_RPC);
    for(int i = 0; !(rpc_session[0].session) && (i < 10000); ++i) {
        rpc_session[0].session = rpc_session_open(rpc);
        furi_delay_tick(1);
    }
    furi_check(rpc_session[0].session);

    rpc_session[0].output_stream = furi_stream_buffer_alloc(4096, 1);
    rpc_session_set_send_bytes_callback(rpc_session[0].session, output_bytes_callback);
    rpc_session[0].close_session_semaphore = xSemaphoreCreateBinary();
    rpc_session[0].terminate_semaphore = xSemaphoreCreateBinary();
    rpc_session_set_close_callback(rpc_session[0].session, test_rpc_session_close_callback);
    rpc_session_set_terminated_callback(
        rpc_session[0].session, test_rpc_session_terminated_callback);
    rpc_session_set_context(rpc_session[0].session, &rpc_session[0]);
}

static void test_rpc_setup_second_session(void) {
    furi_check(rpc);
    furi_check(!(rpc_session[1].session));

    for(int i = 0; !(rpc_session[1].session) && (i < 10000); ++i) {
        rpc_session[1].session = rpc_session_open(rpc);
        furi_delay_tick(1);
    }
    furi_check(rpc_session[1].session);

    rpc_session[1].output_stream = furi_stream_buffer_alloc(1000, 1);
    rpc_session_set_send_bytes_callback(rpc_session[1].session, output_bytes_callback);
    rpc_session[1].close_session_semaphore = xSemaphoreCreateBinary();
    rpc_session[1].terminate_semaphore = xSemaphoreCreateBinary();
    rpc_session_set_close_callback(rpc_session[1].session, test_rpc_session_close_callback);
    rpc_session_set_terminated_callback(
        rpc_session[1].session, test_rpc_session_terminated_callback);
    rpc_session_set_context(rpc_session[1].session, &rpc_session[1]);
}

static void test_rpc_teardown(void) {
    furi_check(rpc_session[0].close_session_semaphore);
    xSemaphoreTake(rpc_session[0].terminate_semaphore, 0);
    rpc_session_close(rpc_session[0].session);
    furi_check(xSemaphoreTake(rpc_session[0].terminate_semaphore, portMAX_DELAY));
    furi_record_close(RECORD_RPC);
    furi_stream_buffer_free(rpc_session[0].output_stream);
    vSemaphoreDelete(rpc_session[0].close_session_semaphore);
    vSemaphoreDelete(rpc_session[0].terminate_semaphore);
    ++command_id;
    rpc_session[0].output_stream = NULL;
    rpc_session[0].close_session_semaphore = NULL;
    rpc = NULL;
    rpc_session[0].session = NULL;
}

static void test_rpc_teardown_second_session(void) {
    furi_check(rpc_session[1].close_session_semaphore);
    xSemaphoreTake(rpc_session[1].terminate_semaphore, 0);
    rpc_session_close(rpc_session[1].session);
    furi_check(xSemaphoreTake(rpc_session[1].terminate_semaphore, portMAX_DELAY));
    furi_stream_buffer_free(rpc_session[1].output_stream);
    vSemaphoreDelete(rpc_session[1].close_session_semaphore);
    vSemaphoreDelete(rpc_session[1].terminate_semaphore);
    ++command_id;
    rpc_session[1].output_stream = NULL;
    rpc_session[1].close_session_semaphore = NULL;
    rpc_session[1].session = NULL;
}

static void test_rpc_storage_setup(void) {
    test_rpc_setup();

    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    clean_directory(fs_api, TEST_DIR_NAME);
    furi_record_close(RECORD_STORAGE);
}

static void test_rpc_storage_teardown(void) {
    test_rpc_teardown();

    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    clean_directory(fs_api, TEST_DIR_NAME);
    furi_record_close(RECORD_STORAGE);
}

static void test_rpc_session_close_callback(void* context) {
    furi_check(context);
    RpcSessionContext* callbacks_context = context;

    xSemaphoreGive(callbacks_context->close_session_semaphore);
}

static void test_rpc_session_terminated_callback(void* context) {
    furi_check(context);
    RpcSessionContext* callbacks_context = context;

    xSemaphoreGive(callbacks_context->terminate_semaphore);
}

static void clean_directory(Storage* fs_api, const char* clean_dir) {
    furi_check(fs_api);
    furi_check(clean_dir);

    File* dir = storage_file_alloc(fs_api);
    if(storage_dir_open(dir, clean_dir)) {
        FileInfo fileinfo;
        char* name = malloc(MAX_NAME_LENGTH + 1);
        while(storage_dir_read(dir, &fileinfo, name, MAX_NAME_LENGTH)) {
            size_t size = strlen(clean_dir) + strlen(name) + 1 + 1;
            char* fullname = malloc(size);
            snprintf(fullname, size, "%s/%s", clean_dir, name);
            if(fileinfo.flags & FSF_DIRECTORY) {
                clean_directory(fs_api, fullname);
            }
            FS_Error error = storage_common_remove(fs_api, fullname);
            furi_check(error == FSE_OK);
            free(fullname);
        }
        free(name);
    } else {
        FS_Error error = storage_common_mkdir(fs_api, clean_dir);
        (void)error;
        furi_check(error == FSE_OK);
    }

    storage_dir_close(dir);
    storage_file_free(dir);
}

static void test_rpc_print_message_list(MsgList_t msg_list) {
#if DEBUG_PRINT
    MsgList_reverse(msg_list);
    for
        M_EACH(msg, msg_list, MsgList_t) {
            rpc_debug_print_message(msg);
        }
    MsgList_reverse(msg_list);
#else
    UNUSED(msg_list);
#endif
}

static PB_CommandStatus test_rpc_storage_get_file_error(File* file) {
    FS_Error fs_error = storage_file_get_error(file);
    PB_CommandStatus pb_error;
    switch(fs_error) {
    case FSE_OK:
        pb_error = PB_CommandStatus_OK;
        break;
    case FSE_INVALID_NAME:
        pb_error = PB_CommandStatus_ERROR_STORAGE_INVALID_NAME;
        break;
    case FSE_INVALID_PARAMETER:
        pb_error = PB_CommandStatus_ERROR_STORAGE_INVALID_PARAMETER;
        break;
    case FSE_INTERNAL:
        pb_error = PB_CommandStatus_ERROR_STORAGE_INTERNAL;
        break;
    case FSE_ALREADY_OPEN:
        pb_error = PB_CommandStatus_ERROR_STORAGE_ALREADY_OPEN;
        break;
    case FSE_DENIED:
        pb_error = PB_CommandStatus_ERROR_STORAGE_DENIED;
        break;
    case FSE_EXIST:
        pb_error = PB_CommandStatus_ERROR_STORAGE_EXIST;
        break;
    case FSE_NOT_EXIST:
        pb_error = PB_CommandStatus_ERROR_STORAGE_NOT_EXIST;
        break;
    case FSE_NOT_READY:
        pb_error = PB_CommandStatus_ERROR_STORAGE_NOT_READY;
        break;
    case FSE_NOT_IMPLEMENTED:
        pb_error = PB_CommandStatus_ERROR_STORAGE_NOT_IMPLEMENTED;
        break;
    default:
        pb_error = PB_CommandStatus_ERROR;
        break;
    }

    return pb_error;
}

static void output_bytes_callback(void* ctx, uint8_t* got_bytes, size_t got_size) {
    RpcSessionContext* callbacks_context = ctx;

    size_t bytes_sent = furi_stream_buffer_send(
        callbacks_context->output_stream, got_bytes, got_size, FuriWaitForever);
    (void)bytes_sent;
    furi_check(bytes_sent == got_size);
}

static void test_rpc_add_ping_to_list(MsgList_t msg_list, bool request, uint32_t command_id) {
    PB_Main* response = MsgList_push_new(msg_list);
    response->command_id = command_id;
    response->command_status = PB_CommandStatus_OK;
    response->cb_content.funcs.encode = NULL;
    response->has_next = false;
    response->which_content = (request == PING_REQUEST) ? PB_Main_system_ping_request_tag :
                                                          PB_Main_system_ping_response_tag;
}

static void test_rpc_create_simple_message(
    PB_Main* message,
    uint16_t tag,
    const char* str,
    uint32_t command_id) {
    furi_check(message);

    char* str_copy = NULL;
    if(str) {
        str_copy = strdup(str);
    }
    message->command_id = command_id;
    message->command_status = PB_CommandStatus_OK;
    message->cb_content.funcs.encode = NULL;
    message->which_content = tag;
    message->has_next = false;
    switch(tag) {
    case PB_Main_storage_info_request_tag:
        message->content.storage_info_request.path = str_copy;
        break;
    case PB_Main_storage_stat_request_tag:
        message->content.storage_stat_request.path = str_copy;
        break;
    case PB_Main_storage_list_request_tag:
        message->content.storage_list_request.path = str_copy;
        break;
    case PB_Main_storage_mkdir_request_tag:
        message->content.storage_mkdir_request.path = str_copy;
        break;
    case PB_Main_storage_read_request_tag:
        message->content.storage_read_request.path = str_copy;
        break;
    case PB_Main_storage_delete_request_tag:
        message->content.storage_delete_request.path = str_copy;
        break;
    case PB_Main_storage_md5sum_request_tag:
        message->content.storage_md5sum_request.path = str_copy;
        break;
    case PB_Main_storage_md5sum_response_tag: {
        char* md5sum = message->content.storage_md5sum_response.md5sum;
        size_t md5sum_size = sizeof(message->content.storage_md5sum_response.md5sum);
        furi_check((strlen(str) + 1) <= md5sum_size);
        memcpy(md5sum, str_copy, md5sum_size);
        free(str_copy);
        break;
    }
    default:
        furi_check(0);
        break;
    }
}

static void test_rpc_add_read_or_write_to_list(
    MsgList_t msg_list,
    bool write,
    const char* path,
    const uint8_t* pattern,
    size_t pattern_size,
    size_t pattern_repeats,
    uint32_t command_id) {
    furi_check(pattern_repeats > 0);

    do {
        PB_Main* request = MsgList_push_new(msg_list);
        PB_Storage_File* msg_file = NULL;

        request->command_id = command_id;
        request->command_status = PB_CommandStatus_OK;

        if(write == WRITE_REQUEST) {
            request->content.storage_write_request.path = strdup(path);
            request->which_content = PB_Main_storage_write_request_tag;
            request->content.storage_write_request.has_file = true;
            msg_file = &request->content.storage_write_request.file;
        } else {
            request->which_content = PB_Main_storage_read_response_tag;
            request->content.storage_read_response.has_file = true;
            msg_file = &request->content.storage_read_response.file;
        }

        msg_file->data = malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(pattern_size));
        msg_file->data->size = pattern_size;

        memcpy(msg_file->data->bytes, pattern, pattern_size);

        --pattern_repeats;
        request->has_next = (pattern_repeats > 0);
    } while(pattern_repeats);
}

static void test_rpc_encode_and_feed_one(PB_Main* request, uint8_t session) {
    furi_check(request);
    furi_check(session < TEST_RPC_SESSIONS);

    pb_ostream_t ostream = PB_OSTREAM_SIZING;

    bool result = pb_encode_ex(&ostream, &PB_Main_msg, request, PB_ENCODE_DELIMITED);
    furi_check(result && ostream.bytes_written);

    uint8_t* buffer = malloc(ostream.bytes_written);
    ostream = pb_ostream_from_buffer(buffer, ostream.bytes_written);

    pb_encode_ex(&ostream, &PB_Main_msg, request, PB_ENCODE_DELIMITED);

    size_t bytes_left = ostream.bytes_written;
    uint8_t* buffer_ptr = buffer;
    do {
        size_t bytes_sent =
            rpc_session_feed(rpc_session[session].session, buffer_ptr, bytes_left, 1000);
        mu_check(bytes_sent > 0);

        bytes_left -= bytes_sent;
        buffer_ptr += bytes_sent;
    } while(bytes_left);

    free(buffer);
    pb_release(&PB_Main_msg, request);
}

static void test_rpc_encode_and_feed(MsgList_t msg_list, uint8_t session) {
    MsgList_reverse(msg_list);
    for
        M_EACH(request, msg_list, MsgList_t) {
            test_rpc_encode_and_feed_one(request, session);
        }
    MsgList_reverse(msg_list);
}

static void
    test_rpc_compare_file(PB_Storage_File* result_msg_file, PB_Storage_File* expected_msg_file) {
    mu_check(!result_msg_file->name == !expected_msg_file->name);
    if(result_msg_file->name) {
        mu_check(!strcmp(result_msg_file->name, expected_msg_file->name));
    }
    mu_check(result_msg_file->size == expected_msg_file->size);
    mu_check(result_msg_file->type == expected_msg_file->type);

    if(result_msg_file->data && result_msg_file->type != PB_Storage_File_FileType_DIR) {
        mu_check(!result_msg_file->data == !expected_msg_file->data); // Zlo: WTF???
        mu_check(result_msg_file->data->size == expected_msg_file->data->size);
        for(int i = 0; i < result_msg_file->data->size; ++i) {
            mu_check(result_msg_file->data->bytes[i] == expected_msg_file->data->bytes[i]);
        }
    }
}

static void test_rpc_compare_messages(PB_Main* result, PB_Main* expected) {
    mu_check(result->command_id == expected->command_id);
    mu_check(result->command_status == expected->command_status);
    mu_check(result->has_next == expected->has_next);
    mu_check(result->which_content == expected->which_content);
    if(result->command_status != PB_CommandStatus_OK) {
        mu_check(result->which_content == PB_Main_empty_tag);
    }

    switch(result->which_content) {
    case PB_Main_empty_tag:
    case PB_Main_system_ping_response_tag:
        /* nothing to check */
        break;
    case PB_Main_system_ping_request_tag:
    case PB_Main_storage_list_request_tag:
    case PB_Main_storage_read_request_tag:
    case PB_Main_storage_write_request_tag:
    case PB_Main_storage_delete_request_tag:
    case PB_Main_storage_mkdir_request_tag:
    case PB_Main_storage_md5sum_request_tag:
        /* rpc doesn't send it */
        mu_check(0);
        break;
    case PB_Main_app_lock_status_response_tag: {
        bool result_locked = result->content.app_lock_status_response.locked;
        bool expected_locked = expected->content.app_lock_status_response.locked;
        mu_check(result_locked == expected_locked);
        break;
    }
    case PB_Main_storage_info_response_tag: {
        uint64_t result_total_space = result->content.storage_info_response.total_space;
        uint64_t expected_total_space = expected->content.storage_info_response.total_space;
        mu_check(result_total_space == expected_total_space);

        uint64_t result_free_space = result->content.storage_info_response.free_space;
        uint64_t expected_free_space = expected->content.storage_info_response.free_space;
        mu_check(result_free_space == expected_free_space);
    } break;
    case PB_Main_storage_stat_response_tag: {
        bool result_has_msg_file = result->content.storage_stat_response.has_file;
        bool expected_has_msg_file = expected->content.storage_stat_response.has_file;
        mu_check(result_has_msg_file == expected_has_msg_file);

        if(result_has_msg_file) {
            PB_Storage_File* result_msg_file = &result->content.storage_stat_response.file;
            PB_Storage_File* expected_msg_file = &expected->content.storage_stat_response.file;
            test_rpc_compare_file(result_msg_file, expected_msg_file);
        } else {
            mu_check(0);
        }
    } break;
    case PB_Main_storage_read_response_tag: {
        bool result_has_msg_file = result->content.storage_read_response.has_file;
        bool expected_has_msg_file = expected->content.storage_read_response.has_file;
        mu_check(result_has_msg_file == expected_has_msg_file);

        if(result_has_msg_file) {
            PB_Storage_File* result_msg_file = &result->content.storage_read_response.file;
            PB_Storage_File* expected_msg_file = &expected->content.storage_read_response.file;
            test_rpc_compare_file(result_msg_file, expected_msg_file);
        } else {
            mu_check(0);
        }
    } break;
    case PB_Main_storage_list_response_tag: {
        size_t expected_msg_files = expected->content.storage_list_response.file_count;
        size_t result_msg_files = result->content.storage_list_response.file_count;
        mu_assert_int_eq(expected_msg_files, result_msg_files);
        for(size_t i = 0; i < expected_msg_files; ++i) {
            PB_Storage_File* result_msg_file = &result->content.storage_list_response.file[i];
            PB_Storage_File* expected_msg_file = &expected->content.storage_list_response.file[i];
            test_rpc_compare_file(result_msg_file, expected_msg_file);
        }
        break;
    }
    case PB_Main_storage_md5sum_response_tag: {
        char* result_md5sum = result->content.storage_md5sum_response.md5sum;
        char* expected_md5sum = expected->content.storage_md5sum_response.md5sum;
        mu_check(!strcmp(result_md5sum, expected_md5sum));
        break;
    }
    case PB_Main_system_protobuf_version_response_tag: {
        uint32_t major_version_expected = expected->content.system_protobuf_version_response.major;
        uint32_t minor_version_expected = expected->content.system_protobuf_version_response.minor;
        uint32_t major_version_result = result->content.system_protobuf_version_response.major;
        uint32_t minor_version_result = result->content.system_protobuf_version_response.minor;
        mu_check(major_version_expected == major_version_result);
        mu_check(minor_version_expected == minor_version_result);
        break;
    }
    default:
        furi_check(0);
        break;
    }
}

static bool test_rpc_pb_stream_read(pb_istream_t* istream, pb_byte_t* buf, size_t count) {
    RpcSessionContext* session_context = istream->state;
    size_t bytes_received = 0;

    TickType_t now = xTaskGetTickCount();
    int32_t time_left = session_context->timeout - now;
    time_left = MAX(time_left, 0);
    bytes_received =
        furi_stream_buffer_receive(session_context->output_stream, buf, count, time_left);
    return (count == bytes_received);
}

static void
    test_rpc_storage_list_create_expected_list_root(MsgList_t msg_list, uint32_t command_id) {
    PB_Main* message = MsgList_push_new(msg_list);
    message->has_next = false;
    message->cb_content.funcs.encode = NULL;
    message->command_id = command_id;
    message->which_content = PB_Main_storage_list_response_tag;

    message->content.storage_list_response.file_count = 3;
    message->content.storage_list_response.file[0].data = NULL;
    message->content.storage_list_response.file[1].data = NULL;
    message->content.storage_list_response.file[2].data = NULL;

    message->content.storage_list_response.file[0].size = 0;
    message->content.storage_list_response.file[1].size = 0;
    message->content.storage_list_response.file[2].size = 0;

    message->content.storage_list_response.file[0].type = PB_Storage_File_FileType_DIR;
    message->content.storage_list_response.file[1].type = PB_Storage_File_FileType_DIR;
    message->content.storage_list_response.file[2].type = PB_Storage_File_FileType_DIR;

    char* str = malloc(4);
    strcpy(str, "any");
    message->content.storage_list_response.file[0].name = str;
    str = malloc(4);
    strcpy(str, "int");
    message->content.storage_list_response.file[1].name = str;
    str = malloc(4);
    strcpy(str, "ext");
    message->content.storage_list_response.file[2].name = str;
}

static void test_rpc_storage_list_create_expected_list(
    MsgList_t msg_list,
    const char* path,
    uint32_t command_id) {
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    File* dir = storage_file_alloc(fs_api);

    PB_Main response = {
        .command_id = command_id,
        .has_next = false,
        .which_content = PB_Main_storage_list_response_tag,
        /* other fields (e.g. msg_files ptrs) explicitly initialized by 0 */
    };
    PB_Storage_ListResponse* list = &response.content.storage_list_response;

    bool finish = false;
    int i = 0;

    if(storage_dir_open(dir, path)) {
        response.command_status = PB_CommandStatus_OK;
    } else {
        response.command_status = test_rpc_storage_get_file_error(dir);
        response.which_content = PB_Main_empty_tag;
        finish = true;
    }

    while(!finish) {
        FileInfo fileinfo;
        char* name = malloc(MAX_NAME_LENGTH + 1);
        if(storage_dir_read(dir, &fileinfo, name, MAX_NAME_LENGTH)) {
            if(i == COUNT_OF(list->file)) {
                list->file_count = i;
                response.has_next = true;
                MsgList_push_back(msg_list, response);
                i = 0;
            }

            if(path_contains_only_ascii(name)) {
                list->file[i].type = (fileinfo.flags & FSF_DIRECTORY) ?
                                         PB_Storage_File_FileType_DIR :
                                         PB_Storage_File_FileType_FILE;
                list->file[i].size = fileinfo.size;
                list->file[i].data = NULL;
                /* memory free inside rpc_encode_and_send() -> pb_release() */
                list->file[i].name = name;
                ++i;
            }
        } else {
            finish = true;
            free(name);
        }
    }

    list->file_count = i;
    response.has_next = false;
    MsgList_push_back(msg_list, response);

    storage_dir_close(dir);
    storage_file_free(dir);

    furi_record_close(RECORD_STORAGE);
}

static void test_rpc_decode_and_compare(MsgList_t expected_msg_list, uint8_t session) {
    furi_check(!MsgList_empty_p(expected_msg_list));
    furi_check(session < TEST_RPC_SESSIONS);

    rpc_session[session].timeout = xTaskGetTickCount() + MAX_RECEIVE_OUTPUT_TIMEOUT;
    pb_istream_t istream = {
        .callback = test_rpc_pb_stream_read,
        .state = &rpc_session[session],
        .errmsg = NULL,
        .bytes_left = 0x7FFFFFFF,
    };
    /* other fields explicitly initialized by 0 */
    PB_Main result = {.cb_content.funcs.decode = NULL};

    /* mlib adds msg_files into start of list, so reverse it */
    MsgList_reverse(expected_msg_list);
    for
        M_EACH(expected_msg, expected_msg_list, MsgList_t) {
            if(!pb_decode_ex(&istream, &PB_Main_msg, &result, PB_DECODE_DELIMITED)) {
                mu_fail(
                    "not all expected messages decoded (maybe increase MAX_RECEIVE_OUTPUT_TIMEOUT)");
                break;
            }

            test_rpc_compare_messages(&result, expected_msg);
            pb_release(&PB_Main_msg, &result);
        }

    rpc_session[session].timeout = xTaskGetTickCount() + 50;
    if(pb_decode_ex(&istream, &PB_Main_msg, &result, PB_DECODE_DELIMITED)) {
        mu_fail("decoded more than expected");
    }
    MsgList_reverse(expected_msg_list);
}

static void test_rpc_free_msg_list(MsgList_t msg_list) {
    for
        M_EACH(it, msg_list, MsgList_t) {
            pb_release(&PB_Main_msg, it);
        }
    MsgList_clear(msg_list);
}

static void test_rpc_storage_list_run(const char* path, uint32_t command_id) {
    PB_Main request;
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    test_rpc_create_simple_message(&request, PB_Main_storage_list_request_tag, path, command_id);
    if(!strcmp(path, "/")) {
        test_rpc_storage_list_create_expected_list_root(expected_msg_list, command_id);
    } else {
        test_rpc_storage_list_create_expected_list(expected_msg_list, path, command_id);
    }
    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    pb_release(&PB_Main_msg, &request);
    test_rpc_free_msg_list(expected_msg_list);
}

MU_TEST(test_storage_list) {
    test_rpc_storage_list_run("/", ++command_id);
    test_rpc_storage_list_run(EXT_PATH("nfc"), ++command_id);

    test_rpc_storage_list_run(STORAGE_INT_PATH_PREFIX, ++command_id);
    test_rpc_storage_list_run(STORAGE_EXT_PATH_PREFIX, ++command_id);
    test_rpc_storage_list_run(EXT_PATH("infrared"), ++command_id);
    test_rpc_storage_list_run(EXT_PATH("ibutton"), ++command_id);
    test_rpc_storage_list_run(EXT_PATH("lfrfid"), ++command_id);
    test_rpc_storage_list_run("error_path", ++command_id);
}

static void
    test_rpc_add_empty_to_list(MsgList_t msg_list, PB_CommandStatus status, uint32_t command_id) {
    PB_Main* response = MsgList_push_new(msg_list);
    response->command_id = command_id;
    response->command_status = status;
    response->cb_content.funcs.encode = NULL;
    response->has_next = false;
    response->which_content = PB_Main_empty_tag;
}

static void test_rpc_add_read_to_list_by_reading_real_file(
    MsgList_t msg_list,
    const char* path,
    uint32_t command_id) {
    furi_check(MsgList_empty_p(msg_list));
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(fs_api);

    bool result = false;

    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        size_t size_left = storage_file_size(file);

        do {
            PB_Main* response = MsgList_push_new(msg_list);
            response->command_id = command_id;
            response->command_status = PB_CommandStatus_OK;
            response->has_next = false;
            response->which_content = PB_Main_storage_read_response_tag;
            response->content.storage_read_response.has_file = true;

            response->content.storage_read_response.file.data =
                malloc(PB_BYTES_ARRAY_T_ALLOCSIZE(MIN(size_left, MAX_DATA_SIZE)));
            uint8_t* buffer = response->content.storage_read_response.file.data->bytes;
            uint16_t* read_size_msg = &response->content.storage_read_response.file.data->size;
            size_t read_size = MIN(size_left, MAX_DATA_SIZE);
            *read_size_msg = storage_file_read(file, buffer, read_size);
            size_left -= read_size;
            result = (*read_size_msg == read_size);

            if(result) {
                response->has_next = (size_left > 0);
            }
        } while((size_left != 0) && result);

        if(!result) {
            test_rpc_add_empty_to_list(
                msg_list, test_rpc_storage_get_file_error(file), command_id);
        }
    } else {
        test_rpc_add_empty_to_list(msg_list, test_rpc_storage_get_file_error(file), command_id);
    }

    storage_file_close(file);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);
}

static void test_storage_read_run(const char* path, uint32_t command_id) {
    PB_Main request;
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    test_rpc_add_read_to_list_by_reading_real_file(expected_msg_list, path, command_id);
    test_rpc_create_simple_message(&request, PB_Main_storage_read_request_tag, path, command_id);
    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    pb_release(&PB_Main_msg, &request);
    test_rpc_free_msg_list(expected_msg_list);
}

static bool test_is_exists(const char* path) {
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    FileInfo fileinfo;
    FS_Error result = storage_common_stat(fs_api, path, &fileinfo);
    furi_check((result == FSE_OK) || (result == FSE_NOT_EXIST));
    furi_record_close(RECORD_STORAGE);
    return result == FSE_OK;
}

static void test_create_dir(const char* path) {
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    FS_Error error = storage_common_mkdir(fs_api, path);
    (void)error;
    furi_check((error == FSE_OK) || (error == FSE_EXIST));
    furi_record_close(RECORD_STORAGE);
    furi_check(test_is_exists(path));
}

static void test_create_file(const char* path, size_t size) {
    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(fs_api);

    if(storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        uint8_t buf[128] = {0};
        for(size_t i = 0; i < sizeof(buf); ++i) {
            buf[i] = '0' + (i % 10);
        }
        while(size) {
            size_t written = storage_file_write(file, buf, MIN(size, sizeof(buf)));
            furi_check(written);
            size -= written;
        }
    }

    storage_file_close(file);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);
    furi_check(test_is_exists(path));
}

static void test_rpc_storage_info_run(const char* path, uint32_t command_id) {
    PB_Main request;
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    test_rpc_create_simple_message(&request, PB_Main_storage_info_request_tag, path, command_id);

    PB_Main* response = MsgList_push_new(expected_msg_list);
    response->command_id = command_id;

    Storage* fs_api = furi_record_open(RECORD_STORAGE);

    FS_Error error = storage_common_fs_info(
        fs_api,
        path,
        &response->content.storage_info_response.total_space,
        &response->content.storage_info_response.free_space);

    response->command_status = rpc_system_storage_get_error(error);
    if(error == FSE_OK) {
        response->which_content = PB_Main_storage_info_response_tag;
    } else {
        response->which_content = PB_Main_empty_tag;
    }

    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    pb_release(&PB_Main_msg, &request);
    test_rpc_free_msg_list(expected_msg_list);
}

static void test_rpc_storage_stat_run(const char* path, uint32_t command_id) {
    PB_Main request;
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    test_rpc_create_simple_message(&request, PB_Main_storage_stat_request_tag, path, command_id);

    Storage* fs_api = furi_record_open(RECORD_STORAGE);
    FileInfo fileinfo;
    FS_Error error = storage_common_stat(fs_api, path, &fileinfo);
    furi_record_close(RECORD_STORAGE);

    PB_Main* response = MsgList_push_new(expected_msg_list);
    response->command_id = command_id;
    response->command_status = rpc_system_storage_get_error(error);
    response->has_next = false;
    response->which_content = PB_Main_empty_tag;

    if(error == FSE_OK) {
        response->which_content = PB_Main_storage_stat_response_tag;
        response->content.storage_stat_response.has_file = true;
        response->content.storage_stat_response.file.type = (fileinfo.flags & FSF_DIRECTORY) ?
                                                                PB_Storage_File_FileType_DIR :
                                                                PB_Storage_File_FileType_FILE;
        response->content.storage_stat_response.file.size = fileinfo.size;
    }

    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    pb_release(&PB_Main_msg, &request);
    test_rpc_free_msg_list(expected_msg_list);
}

MU_TEST(test_storage_info) {
    test_rpc_storage_info_run(STORAGE_ANY_PATH_PREFIX, ++command_id);
    test_rpc_storage_info_run(STORAGE_INT_PATH_PREFIX, ++command_id);
    test_rpc_storage_info_run(STORAGE_EXT_PATH_PREFIX, ++command_id);
}

#define TEST_DIR_STAT_NAME TEST_DIR "stat_dir"
#define TEST_DIR_STAT TEST_DIR_STAT_NAME "/"
MU_TEST(test_storage_stat) {
    test_create_dir(TEST_DIR_STAT_NAME);
    test_create_file(TEST_DIR_STAT "empty.txt", 0);
    test_create_file(TEST_DIR_STAT "l33t.txt", 1337);

    test_rpc_storage_stat_run("/", ++command_id);
    test_rpc_storage_stat_run(STORAGE_INT_PATH_PREFIX, ++command_id);
    test_rpc_storage_stat_run(STORAGE_EXT_PATH_PREFIX, ++command_id);

    test_rpc_storage_stat_run(TEST_DIR_STAT "empty.txt", ++command_id);
    test_rpc_storage_stat_run(TEST_DIR_STAT "l33t.txt", ++command_id);
    test_rpc_storage_stat_run(TEST_DIR_STAT "missing", ++command_id);
    test_rpc_storage_stat_run(TEST_DIR_STAT_NAME, ++command_id);

    test_rpc_storage_stat_run(TEST_DIR_STAT, ++command_id);
}

MU_TEST(test_storage_read) {
    test_create_file(TEST_DIR "empty.txt", 0);
    test_create_file(TEST_DIR "file1.txt", 1);
    test_create_file(TEST_DIR "file2.txt", MAX_DATA_SIZE);
    test_create_file(TEST_DIR "file3.txt", MAX_DATA_SIZE + 1);
    test_create_file(TEST_DIR "file4.txt", (MAX_DATA_SIZE * 2) + 1);

    test_storage_read_run(TEST_DIR "empty.txt", ++command_id);
    test_storage_read_run(TEST_DIR "file1.txt", ++command_id);
    test_storage_read_run(TEST_DIR "file2.txt", ++command_id);
    test_storage_read_run(TEST_DIR "file3.txt", ++command_id);
    test_storage_read_run(TEST_DIR "file4.txt", ++command_id);
}

static void test_storage_write_run(
    const char* path,
    size_t write_size,
    size_t write_count,
    uint32_t command_id,
    PB_CommandStatus status) {
    MsgList_t input_msg_list;
    MsgList_init(input_msg_list);
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    uint8_t* buf = malloc(write_size);
    for(size_t i = 0; i < write_size; ++i) {
        buf[i] = '0' + (i % 10);
    }

    test_rpc_add_read_or_write_to_list(
        input_msg_list, WRITE_REQUEST, path, buf, write_size, write_count, command_id);
    test_rpc_add_empty_to_list(expected_msg_list, status, command_id);
    test_rpc_encode_and_feed(input_msg_list, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    test_rpc_free_msg_list(input_msg_list);
    test_rpc_free_msg_list(expected_msg_list);

    free(buf);
}

static void test_storage_write_read_run(
    const char* path,
    const uint8_t* pattern,
    size_t pattern_size,
    size_t pattern_repeats,
    uint32_t* command_id) {
    MsgList_t input_msg_list;
    MsgList_init(input_msg_list);
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    test_rpc_add_read_or_write_to_list(
        input_msg_list, WRITE_REQUEST, path, pattern, pattern_size, pattern_repeats, ++*command_id);
    test_rpc_add_empty_to_list(expected_msg_list, PB_CommandStatus_OK, *command_id);

    test_rpc_create_simple_message(
        MsgList_push_raw(input_msg_list), PB_Main_storage_read_request_tag, path, ++*command_id);
    test_rpc_add_read_or_write_to_list(
        expected_msg_list,
        READ_RESPONSE,
        path,
        pattern,
        pattern_size,
        pattern_repeats,
        *command_id);

    test_rpc_print_message_list(input_msg_list);
    test_rpc_print_message_list(expected_msg_list);

    test_rpc_encode_and_feed(input_msg_list, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    test_rpc_free_msg_list(input_msg_list);
    test_rpc_free_msg_list(expected_msg_list);
}

MU_TEST(test_storage_write_read) {
    uint8_t pattern1[] = "abcdefgh";
    test_storage_write_read_run(TEST_DIR "test1.txt", pattern1, sizeof(pattern1), 1, &command_id);
    test_storage_write_read_run(TEST_DIR "test2.txt", pattern1, 1, 1, &command_id);
    test_storage_write_read_run(TEST_DIR "test3.txt", pattern1, 0, 1, &command_id);
}

MU_TEST(test_storage_write) {
    test_storage_write_run(
        TEST_DIR "afaefo/aefaef/aef/aef/test1.txt",
        1,
        1,
        ++command_id,
        PB_CommandStatus_ERROR_STORAGE_NOT_EXIST);
    test_storage_write_run(TEST_DIR "test1.txt", 100, 1, ++command_id, PB_CommandStatus_OK);
    test_storage_write_run(TEST_DIR "test2.txt", 100, 3, ++command_id, PB_CommandStatus_OK);
    test_storage_write_run(TEST_DIR "test1.txt", 100, 3, ++command_id, PB_CommandStatus_OK);
    test_storage_write_run(TEST_DIR "test2.txt", 100, 3, ++command_id, PB_CommandStatus_OK);
    test_storage_write_run(
        TEST_DIR "afaefo/aefaef/aef/aef/test1.txt",
        1,
        1,
        ++command_id,
        PB_CommandStatus_ERROR_STORAGE_NOT_EXIST);
    test_storage_write_run(TEST_DIR "test2.txt", 1, 50, ++command_id, PB_CommandStatus_OK);
    test_storage_write_run(TEST_DIR "test2.txt", 512, 3, ++command_id, PB_CommandStatus_OK);
}

MU_TEST(test_storage_interrupt_continuous_same_system) {
    MsgList_t input_msg_list;
    MsgList_init(input_msg_list);
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    uint8_t pattern[16] = {0};

    test_rpc_add_read_or_write_to_list(
        input_msg_list,
        WRITE_REQUEST,
        TEST_DIR "test1.txt",
        pattern,
        sizeof(pattern),
        3,
        command_id);

    /* replace last packet (has_next == false) with another command */
    PB_Main message_to_remove;
    MsgList_pop_back(&message_to_remove, input_msg_list);
    pb_release(&PB_Main_msg, &message_to_remove);
    test_rpc_create_simple_message(
        MsgList_push_new(input_msg_list),
        PB_Main_storage_mkdir_request_tag,
        TEST_DIR "dir1",
        command_id + 1);
    test_rpc_add_read_or_write_to_list(
        input_msg_list,
        WRITE_REQUEST,
        TEST_DIR "test2.txt",
        pattern,
        sizeof(pattern),
        3,
        command_id);

    test_rpc_add_empty_to_list(
        expected_msg_list, PB_CommandStatus_ERROR_CONTINUOUS_COMMAND_INTERRUPTED, command_id);
    test_rpc_add_empty_to_list(expected_msg_list, PB_CommandStatus_OK, command_id + 1);

    test_rpc_encode_and_feed(input_msg_list, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    test_rpc_free_msg_list(input_msg_list);
    test_rpc_free_msg_list(expected_msg_list);
}

MU_TEST(test_storage_interrupt_continuous_another_system) {
    MsgList_t input_msg_list;
    MsgList_init(input_msg_list);
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    uint8_t pattern[16] = {0};

    test_rpc_add_read_or_write_to_list(
        input_msg_list,
        WRITE_REQUEST,
        TEST_DIR "test1.txt",
        pattern,
        sizeof(pattern),
        3,
        command_id);

    PB_Main message = {
        .command_id = command_id + 1,
        .command_status = PB_CommandStatus_OK,
        .cb_content.funcs.encode = NULL,
        .has_next = false,
        .which_content = PB_Main_system_ping_request_tag,
    };

    MsgList_it_t it;
    MsgList_it(it, input_msg_list);
    MsgList_next(it);
    MsgList_insert(input_msg_list, it, message);

    test_rpc_add_read_or_write_to_list(
        input_msg_list,
        WRITE_REQUEST,
        TEST_DIR "test2.txt",
        pattern,
        sizeof(pattern),
        3,
        command_id + 2);

    test_rpc_add_ping_to_list(expected_msg_list, PING_RESPONSE, command_id + 1);
    test_rpc_add_empty_to_list(expected_msg_list, PB_CommandStatus_OK, command_id);
    test_rpc_add_empty_to_list(expected_msg_list, PB_CommandStatus_OK, command_id + 2);

    test_rpc_encode_and_feed(input_msg_list, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    test_rpc_free_msg_list(input_msg_list);
    test_rpc_free_msg_list(expected_msg_list);
}

static void test_storage_delete_run(
    const char* path,
    size_t command_id,
    PB_CommandStatus status,
    bool recursive) {
    PB_Main request;
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    test_rpc_create_simple_message(&request, PB_Main_storage_delete_request_tag, path, command_id);
    request.content.storage_delete_request.recursive = recursive;
    test_rpc_add_empty_to_list(expected_msg_list, status, command_id);

    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    pb_release(&PB_Main_msg, &request);
    test_rpc_free_msg_list(expected_msg_list);
}

#define TEST_DIR_RMRF_NAME TEST_DIR "rmrf_test"
#define TEST_DIR_RMRF TEST_DIR_RMRF_NAME "/"
MU_TEST(test_storage_delete_recursive) {
    test_create_dir(TEST_DIR_RMRF_NAME);

    test_create_dir(TEST_DIR_RMRF "dir1");
    test_create_file(TEST_DIR_RMRF "dir1/file1", 1);

    test_create_dir(TEST_DIR_RMRF "dir1/dir1");
    test_create_dir(TEST_DIR_RMRF "dir1/dir2");
    test_create_file(TEST_DIR_RMRF "dir1/dir2/file1", 1);
    test_create_file(TEST_DIR_RMRF "dir1/dir2/file2", 1);
    test_create_dir(TEST_DIR_RMRF "dir1/dir3");
    test_create_dir(TEST_DIR_RMRF "dir1/dir3/dir1");
    test_create_dir(TEST_DIR_RMRF "dir1/dir3/dir1/dir1");
    test_create_dir(TEST_DIR_RMRF "dir1/dir3/dir1/dir1/dir1");
    test_create_dir(TEST_DIR_RMRF "dir1/dir3/dir1/dir1/dir1/dir1");

    test_create_dir(TEST_DIR_RMRF "dir2");
    test_create_dir(TEST_DIR_RMRF "dir2/dir1");
    test_create_dir(TEST_DIR_RMRF "dir2/dir2");
    test_create_file(TEST_DIR_RMRF "dir2/dir2/file1", 1);

    test_create_dir(TEST_DIR_RMRF "dir2/dir2/dir1");
    test_create_dir(TEST_DIR_RMRF "dir2/dir2/dir1/dir1");
    test_create_dir(TEST_DIR_RMRF "dir2/dir2/dir1/dir1/dir1");
    test_create_file(TEST_DIR_RMRF "dir2/dir2/dir1/dir1/dir1/file1", 1);

    test_storage_delete_run(
        TEST_DIR_RMRF_NAME, ++command_id, PB_CommandStatus_ERROR_STORAGE_DIR_NOT_EMPTY, false);
    mu_check(test_is_exists(TEST_DIR_RMRF_NAME));
    test_storage_delete_run(TEST_DIR_RMRF_NAME, ++command_id, PB_CommandStatus_OK, true);
    mu_check(!test_is_exists(TEST_DIR_RMRF_NAME));
    test_storage_delete_run(TEST_DIR_RMRF_NAME, ++command_id, PB_CommandStatus_OK, false);
    mu_check(!test_is_exists(TEST_DIR_RMRF_NAME));

    test_create_dir(TEST_DIR_RMRF_NAME);
    test_storage_delete_run(TEST_DIR_RMRF_NAME, ++command_id, PB_CommandStatus_OK, true);
    mu_check(!test_is_exists(TEST_DIR_RMRF_NAME));

    test_create_dir(TEST_DIR "file1");
    test_storage_delete_run(TEST_DIR "file1", ++command_id, PB_CommandStatus_OK, true);
    mu_check(!test_is_exists(TEST_DIR "file1"));
}

MU_TEST(test_storage_delete) {
    test_storage_delete_run(NULL, ++command_id, PB_CommandStatus_ERROR_INVALID_PARAMETERS, false);

    furi_check(!test_is_exists(TEST_DIR "empty.txt"));
    test_storage_delete_run(TEST_DIR "empty.txt", ++command_id, PB_CommandStatus_OK, false);
    mu_check(!test_is_exists(TEST_DIR "empty.txt"));

    test_create_file(TEST_DIR "empty.txt", 0);
    test_storage_delete_run(TEST_DIR "empty.txt", ++command_id, PB_CommandStatus_OK, false);
    mu_check(!test_is_exists(TEST_DIR "empty.txt"));

    furi_check(!test_is_exists(TEST_DIR "dir1"));
    test_create_dir(TEST_DIR "dir1");
    test_storage_delete_run(TEST_DIR "dir1", ++command_id, PB_CommandStatus_OK, false);
    mu_check(!test_is_exists(TEST_DIR "dir1"));

    test_storage_delete_run(TEST_DIR "dir1", ++command_id, PB_CommandStatus_OK, false);
    mu_check(!test_is_exists(TEST_DIR "dir1"));
}

static void test_storage_mkdir_run(const char* path, size_t command_id, PB_CommandStatus status) {
    PB_Main request;
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    test_rpc_create_simple_message(&request, PB_Main_storage_mkdir_request_tag, path, command_id);
    test_rpc_add_empty_to_list(expected_msg_list, status, command_id);

    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    pb_release(&PB_Main_msg, &request);
    test_rpc_free_msg_list(expected_msg_list);
}

MU_TEST(test_storage_mkdir) {
    furi_check(!test_is_exists(TEST_DIR "dir1"));
    test_storage_mkdir_run(TEST_DIR "dir1", ++command_id, PB_CommandStatus_OK);
    mu_check(test_is_exists(TEST_DIR "dir1"));

    test_storage_mkdir_run(TEST_DIR "dir1", ++command_id, PB_CommandStatus_ERROR_STORAGE_EXIST);
    mu_check(test_is_exists(TEST_DIR "dir1"));

    furi_check(!test_is_exists(TEST_DIR "dir2"));
    test_create_dir(TEST_DIR "dir2");
    test_storage_mkdir_run(TEST_DIR "dir2", ++command_id, PB_CommandStatus_ERROR_STORAGE_EXIST);
    mu_check(test_is_exists(TEST_DIR "dir2"));
}

static void test_storage_calculate_md5sum(const char* path, char* md5sum, size_t md5sum_size) {
    Storage* api = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(api);

    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        const uint16_t once_read_size = 512;
        const uint8_t hash_size = MD5SUM_SIZE;
        uint8_t* data = malloc(once_read_size);
        uint8_t* hash = malloc(sizeof(uint8_t) * hash_size);
        md5_context* md5_ctx = malloc(sizeof(md5_context));

        md5_starts(md5_ctx);
        while(true) {
            uint16_t read_size = storage_file_read(file, data, once_read_size);
            if(read_size == 0) break;
            md5_update(md5_ctx, data, read_size);
        }
        md5_finish(md5_ctx, hash);
        free(md5_ctx);

        for(uint8_t i = 0; i < hash_size; i++) {
            md5sum += snprintf(md5sum, md5sum_size, "%02x", hash[i]);
        }

        free(hash);
        free(data);
    } else {
        furi_check(0);
    }

    storage_file_close(file);
    storage_file_free(file);

    furi_record_close(RECORD_STORAGE);
}

static void test_storage_md5sum_run(
    const char* path,
    uint32_t command_id,
    const char* md5sum,
    PB_CommandStatus status) {
    PB_Main request;
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    test_rpc_create_simple_message(&request, PB_Main_storage_md5sum_request_tag, path, command_id);
    if(status == PB_CommandStatus_OK) {
        PB_Main* response = MsgList_push_new(expected_msg_list);
        test_rpc_create_simple_message(
            response, PB_Main_storage_md5sum_response_tag, md5sum, command_id);
        response->command_status = status;
    } else {
        test_rpc_add_empty_to_list(expected_msg_list, status, command_id);
    }

    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    pb_release(&PB_Main_msg, &request);
    test_rpc_free_msg_list(expected_msg_list);
}

MU_TEST(test_storage_md5sum) {
    char md5sum1[MD5SUM_SIZE * 2 + 1] = {0};
    char md5sum2[MD5SUM_SIZE * 2 + 1] = {0};
    char md5sum3[MD5SUM_SIZE * 2 + 1] = {0};

    test_storage_md5sum_run(
        TEST_DIR "test1.txt", ++command_id, "", PB_CommandStatus_ERROR_STORAGE_NOT_EXIST);

    test_create_file(TEST_DIR "file1.txt", 0);
    test_create_file(TEST_DIR "file2.txt", 1);
    test_create_file(TEST_DIR "file3.txt", 512);
    test_storage_calculate_md5sum(TEST_DIR "file1.txt", md5sum1, MD5SUM_SIZE * 2 + 1);
    test_storage_calculate_md5sum(TEST_DIR "file2.txt", md5sum2, MD5SUM_SIZE * 2 + 1);
    test_storage_calculate_md5sum(TEST_DIR "file3.txt", md5sum3, MD5SUM_SIZE * 2 + 1);

    test_storage_md5sum_run(TEST_DIR "file1.txt", ++command_id, md5sum1, PB_CommandStatus_OK);
    test_storage_md5sum_run(TEST_DIR "file1.txt", ++command_id, md5sum1, PB_CommandStatus_OK);

    test_storage_md5sum_run(TEST_DIR "file2.txt", ++command_id, md5sum2, PB_CommandStatus_OK);
    test_storage_md5sum_run(TEST_DIR "file2.txt", ++command_id, md5sum2, PB_CommandStatus_OK);

    test_storage_md5sum_run(TEST_DIR "file3.txt", ++command_id, md5sum3, PB_CommandStatus_OK);
    test_storage_md5sum_run(TEST_DIR "file3.txt", ++command_id, md5sum3, PB_CommandStatus_OK);

    test_storage_md5sum_run(TEST_DIR "file2.txt", ++command_id, md5sum2, PB_CommandStatus_OK);
    test_storage_md5sum_run(TEST_DIR "file3.txt", ++command_id, md5sum3, PB_CommandStatus_OK);
    test_storage_md5sum_run(TEST_DIR "file1.txt", ++command_id, md5sum1, PB_CommandStatus_OK);
    test_storage_md5sum_run(TEST_DIR "file2.txt", ++command_id, md5sum2, PB_CommandStatus_OK);
}

static void test_rpc_storage_rename_run(
    const char* old_path,
    const char* new_path,
    uint32_t command_id,
    PB_CommandStatus status) {
    PB_Main request;
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    char* str_old_path = strdup(old_path);
    char* str_new_path = strdup(new_path);

    request.command_id = command_id;
    request.command_status = PB_CommandStatus_OK;
    request.cb_content.funcs.encode = NULL;
    request.which_content = PB_Main_storage_rename_request_tag;
    request.has_next = false;
    request.content.storage_rename_request.old_path = str_old_path;
    request.content.storage_rename_request.new_path = str_new_path;

    test_rpc_add_empty_to_list(expected_msg_list, status, command_id);

    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    pb_release(&PB_Main_msg, &request);
    test_rpc_free_msg_list(expected_msg_list);
}

MU_TEST(test_storage_rename) {
    test_rpc_storage_rename_run("", "", ++command_id, PB_CommandStatus_ERROR_STORAGE_INVALID_NAME);

    furi_check(!test_is_exists(TEST_DIR "empty.txt"));
    test_create_file(TEST_DIR "empty.txt", 0);
    test_rpc_storage_rename_run(
        TEST_DIR "empty.txt", TEST_DIR "empty2.txt", ++command_id, PB_CommandStatus_OK);
    mu_check(!test_is_exists(TEST_DIR "empty.txt"));
    mu_check(test_is_exists(TEST_DIR "empty2.txt"));

    furi_check(!test_is_exists(TEST_DIR "dir1"));
    test_create_dir(TEST_DIR "dir1");
    test_rpc_storage_rename_run(
        TEST_DIR "dir1", TEST_DIR "dir2", ++command_id, PB_CommandStatus_OK);
    mu_check(!test_is_exists(TEST_DIR "dir1"));
    mu_check(test_is_exists(TEST_DIR "dir2"));
}

MU_TEST(test_ping) {
    MsgList_t input_msg_list;
    MsgList_init(input_msg_list);
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    test_rpc_add_ping_to_list(input_msg_list, PING_REQUEST, 0);
    test_rpc_add_ping_to_list(input_msg_list, PING_REQUEST, 1);
    test_rpc_add_ping_to_list(input_msg_list, PING_REQUEST, 0);
    test_rpc_add_ping_to_list(input_msg_list, PING_REQUEST, 500);
    test_rpc_add_ping_to_list(input_msg_list, PING_REQUEST, (uint32_t)-1);
    test_rpc_add_ping_to_list(input_msg_list, PING_REQUEST, 700);
    test_rpc_add_ping_to_list(input_msg_list, PING_REQUEST, 1);

    test_rpc_add_ping_to_list(expected_msg_list, PING_RESPONSE, 0);
    test_rpc_add_ping_to_list(expected_msg_list, PING_RESPONSE, 1);
    test_rpc_add_ping_to_list(expected_msg_list, PING_RESPONSE, 0);
    test_rpc_add_ping_to_list(expected_msg_list, PING_RESPONSE, 500);
    test_rpc_add_ping_to_list(expected_msg_list, PING_RESPONSE, (uint32_t)-1);
    test_rpc_add_ping_to_list(expected_msg_list, PING_RESPONSE, 700);
    test_rpc_add_ping_to_list(expected_msg_list, PING_RESPONSE, 1);

    test_rpc_encode_and_feed(input_msg_list, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    test_rpc_free_msg_list(input_msg_list);
    test_rpc_free_msg_list(expected_msg_list);
}

MU_TEST(test_system_protobuf_version) {
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    PB_Main request;
    request.command_id = ++command_id;
    request.command_status = PB_CommandStatus_OK;
    request.cb_content.funcs.decode = NULL;
    request.has_next = false;
    request.which_content = PB_Main_system_protobuf_version_request_tag;

    PB_Main* response = MsgList_push_new(expected_msg_list);
    response->command_id = command_id;
    response->command_status = PB_CommandStatus_OK;
    response->cb_content.funcs.encode = NULL;
    response->has_next = false;
    response->which_content = PB_Main_system_protobuf_version_response_tag;
    response->content.system_protobuf_version_response.major = PROTOBUF_MAJOR_VERSION;
    response->content.system_protobuf_version_response.minor = PROTOBUF_MINOR_VERSION;

    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    test_rpc_free_msg_list(expected_msg_list);
}

MU_TEST_SUITE(test_rpc_system) {
    MU_SUITE_CONFIGURE(&test_rpc_setup, &test_rpc_teardown);

    MU_RUN_TEST(test_ping);
    MU_RUN_TEST(test_system_protobuf_version);
}

MU_TEST_SUITE(test_rpc_storage) {
    MU_SUITE_CONFIGURE(&test_rpc_storage_setup, &test_rpc_storage_teardown);

    MU_RUN_TEST(test_storage_info);
    MU_RUN_TEST(test_storage_stat);
    MU_RUN_TEST(test_storage_list);
    MU_RUN_TEST(test_storage_read);
    MU_RUN_TEST(test_storage_write_read);
    MU_RUN_TEST(test_storage_write);
    MU_RUN_TEST(test_storage_delete);
    MU_RUN_TEST(test_storage_delete_recursive);
    MU_RUN_TEST(test_storage_mkdir);
    MU_RUN_TEST(test_storage_md5sum);
    MU_RUN_TEST(test_storage_rename);

    DISABLE_TEST(MU_RUN_TEST(test_storage_interrupt_continuous_same_system););
    MU_RUN_TEST(test_storage_interrupt_continuous_another_system);
}

static void test_app_create_request(
    PB_Main* request,
    const char* app_name,
    const char* app_args,
    uint32_t command_id) {
    request->command_id = command_id;
    request->command_status = PB_CommandStatus_OK;
    request->cb_content.funcs.encode = NULL;
    request->which_content = PB_Main_app_start_request_tag;
    request->has_next = false;

    if(app_name) {
        char* msg_app_name = strdup(app_name);
        request->content.app_start_request.name = msg_app_name;
    } else {
        request->content.app_start_request.name = NULL;
    }

    if(app_args) {
        char* msg_app_args = strdup(app_args);
        request->content.app_start_request.args = msg_app_args;
    } else {
        request->content.app_start_request.args = NULL;
    }
}

static void test_app_start_run(
    const char* app_name,
    const char* app_args,
    PB_CommandStatus status,
    uint32_t command_id) {
    PB_Main request;
    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);

    test_app_create_request(&request, app_name, app_args, command_id);
    test_rpc_add_empty_to_list(expected_msg_list, status, command_id);

    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    pb_release(&PB_Main_msg, &request);
    test_rpc_free_msg_list(expected_msg_list);
}

static void test_app_get_status_lock_run(bool locked_expected, uint32_t command_id) {
    PB_Main request = {
        .command_id = command_id,
        .command_status = PB_CommandStatus_OK,
        .which_content = PB_Main_app_lock_status_request_tag,
        .has_next = false,
    };

    MsgList_t expected_msg_list;
    MsgList_init(expected_msg_list);
    PB_Main* response = MsgList_push_new(expected_msg_list);
    response->command_id = command_id;
    response->command_status = PB_CommandStatus_OK;
    response->which_content = PB_Main_app_lock_status_response_tag;
    response->has_next = false;
    response->content.app_lock_status_response.locked = locked_expected;

    test_rpc_encode_and_feed_one(&request, 0);
    test_rpc_decode_and_compare(expected_msg_list, 0);

    pb_release(&PB_Main_msg, &request);
    test_rpc_free_msg_list(expected_msg_list);
}

MU_TEST(test_app_start_and_lock_status) {
    test_app_get_status_lock_run(false, ++command_id);
    test_app_start_run(
        NULL, EXT_PATH("file"), PB_CommandStatus_ERROR_INVALID_PARAMETERS, ++command_id);
    test_app_start_run(NULL, NULL, PB_CommandStatus_ERROR_INVALID_PARAMETERS, ++command_id);
    test_app_get_status_lock_run(false, ++command_id);
    test_app_start_run(
        "skynet_destroy_world_app", NULL, PB_CommandStatus_ERROR_INVALID_PARAMETERS, ++command_id);
    test_app_get_status_lock_run(false, ++command_id);

    test_app_start_run("Delay Test", "0", PB_CommandStatus_OK, ++command_id);
    furi_delay_ms(100);
    test_app_get_status_lock_run(false, ++command_id);

    test_app_start_run("Delay Test", "200", PB_CommandStatus_OK, ++command_id);
    test_app_get_status_lock_run(true, ++command_id);
    furi_delay_ms(100);
    test_app_get_status_lock_run(true, ++command_id);
    test_app_start_run("Delay Test", "0", PB_CommandStatus_ERROR_APP_SYSTEM_LOCKED, ++command_id);
    furi_delay_ms(200);
    test_app_get_status_lock_run(false, ++command_id);

    test_app_start_run("Delay Test", "500", PB_CommandStatus_OK, ++command_id);
    furi_delay_ms(100);
    test_app_get_status_lock_run(true, ++command_id);
    test_app_start_run("Infrared", "0", PB_CommandStatus_ERROR_APP_SYSTEM_LOCKED, ++command_id);
    furi_delay_ms(100);
    test_app_get_status_lock_run(true, ++command_id);
    test_app_start_run(
        "2_girls_1_app", "0", PB_CommandStatus_ERROR_INVALID_PARAMETERS, ++command_id);
    furi_delay_ms(100);
    test_app_get_status_lock_run(true, ++command_id);
    furi_delay_ms(500);
    test_app_get_status_lock_run(false, ++command_id);
}

MU_TEST_SUITE(test_rpc_app) {
    MU_SUITE_CONFIGURE(&test_rpc_setup, &test_rpc_teardown);

    DISABLE_TEST(MU_RUN_TEST(test_app_start_and_lock_status););
}

static void
    test_send_rubbish(RpcSession* session, const char* pattern, size_t pattern_size, size_t size) {
    UNUSED(session);
    uint8_t* buf = malloc(size);
    for(size_t i = 0; i < size; ++i) {
        buf[i] = pattern[i % pattern_size];
    }

    size_t bytes_sent = rpc_session_feed(rpc_session[0].session, buf, size, 1000);
    furi_check(bytes_sent == size);
    free(buf);
}

static void test_rpc_feed_rubbish_run(
    MsgList_t input_before,
    MsgList_t input_after,
    MsgList_t expected,
    const char* pattern,
    size_t pattern_size,
    size_t size) {
    test_rpc_setup();

    test_rpc_add_empty_to_list(expected, PB_CommandStatus_ERROR_DECODE, 0);

    furi_check(!xSemaphoreTake(rpc_session[0].close_session_semaphore, 0));
    test_rpc_encode_and_feed(input_before, 0);
    test_send_rubbish(rpc_session[0].session, pattern, pattern_size, size);
    test_rpc_encode_and_feed(input_after, 0);

    test_rpc_decode_and_compare(expected, 0);

    test_rpc_teardown();
}

#define RUN_TEST_RPC_FEED_RUBBISH(ib, ia, e, b, c) \
    test_rpc_feed_rubbish_run(ib, ia, e, b, sizeof(b), c)

#define INIT_LISTS()            \
    MsgList_init(input_before); \
    MsgList_init(input_after);  \
    MsgList_init(expected);

#define FREE_LISTS()                      \
    test_rpc_free_msg_list(input_before); \
    test_rpc_free_msg_list(input_after);  \
    test_rpc_free_msg_list(expected);

MU_TEST(test_rpc_feed_rubbish) {
    MsgList_t input_before;
    MsgList_t input_after;
    MsgList_t expected;

    INIT_LISTS();
    // input is empty
    RUN_TEST_RPC_FEED_RUBBISH(input_before, input_after, expected, "\x12\x30rubbi\x42sh", 50);
    FREE_LISTS();

    INIT_LISTS();
    test_rpc_add_ping_to_list(input_before, PING_REQUEST, ++command_id);
    test_rpc_add_ping_to_list(expected, PING_RESPONSE, command_id);
    RUN_TEST_RPC_FEED_RUBBISH(input_before, input_after, expected, "\x2\x2\x2\x5\x99\x1", 30);
    FREE_LISTS();

    INIT_LISTS();
    test_rpc_add_ping_to_list(input_after, PING_REQUEST, ++command_id);
    RUN_TEST_RPC_FEED_RUBBISH(input_before, input_after, expected, "\x12\x30rubbi\x42sh", 50);
    FREE_LISTS();

    INIT_LISTS();
    test_rpc_add_ping_to_list(input_before, PING_REQUEST, ++command_id);
    test_rpc_add_ping_to_list(expected, PING_RESPONSE, command_id);
    test_rpc_add_ping_to_list(input_before, PING_REQUEST, ++command_id);
    test_rpc_add_ping_to_list(expected, PING_RESPONSE, command_id);
    test_rpc_add_ping_to_list(input_before, PING_REQUEST, ++command_id);
    test_rpc_add_ping_to_list(expected, PING_RESPONSE, command_id);
    test_rpc_add_ping_to_list(input_before, PING_REQUEST, ++command_id);
    test_rpc_add_ping_to_list(expected, PING_RESPONSE, command_id);
    test_rpc_add_ping_to_list(input_before, PING_REQUEST, ++command_id);
    test_rpc_add_ping_to_list(expected, PING_RESPONSE, command_id);
    test_rpc_add_ping_to_list(input_before, PING_REQUEST, ++command_id);
    test_rpc_add_ping_to_list(expected, PING_RESPONSE, command_id);
    test_rpc_add_ping_to_list(input_before, PING_REQUEST, ++command_id);
    test_rpc_add_ping_to_list(expected, PING_RESPONSE, command_id);
    test_rpc_add_ping_to_list(input_before, PING_REQUEST, ++command_id);
    test_rpc_add_ping_to_list(expected, PING_RESPONSE, command_id);
    test_rpc_add_ping_to_list(input_after, PING_REQUEST, command_id);
    test_rpc_add_ping_to_list(input_after, PING_REQUEST, command_id);
    test_rpc_add_ping_to_list(input_after, PING_REQUEST, command_id);
    RUN_TEST_RPC_FEED_RUBBISH(input_before, input_after, expected, "\x99\x2\x2\x5\x99\x1", 300);
    FREE_LISTS();

    INIT_LISTS();
    test_rpc_add_ping_to_list(input_after, PING_REQUEST, ++command_id);
    RUN_TEST_RPC_FEED_RUBBISH(input_before, input_after, expected, "\x1\x99\x2\x5\x99\x1", 300);
    FREE_LISTS();

    INIT_LISTS();
    test_rpc_add_ping_to_list(input_before, PING_REQUEST, ++command_id);
    test_rpc_add_ping_to_list(expected, PING_RESPONSE, command_id);
    RUN_TEST_RPC_FEED_RUBBISH(input_before, input_after, expected, "\x2\x2\x2\x5\x99\x1", 30);
    FREE_LISTS();

    INIT_LISTS();
    test_rpc_add_ping_to_list(input_before, PING_RESPONSE, ++command_id);
    test_rpc_add_empty_to_list(expected, PB_CommandStatus_ERROR_NOT_IMPLEMENTED, command_id);
    test_rpc_add_ping_to_list(input_before, PING_RESPONSE, ++command_id);
    test_rpc_add_empty_to_list(expected, PB_CommandStatus_ERROR_NOT_IMPLEMENTED, command_id);
    RUN_TEST_RPC_FEED_RUBBISH(input_before, input_after, expected, "\x12\x30rubbi\x42sh", 50);
    FREE_LISTS();
}

MU_TEST(test_rpc_multisession_ping) {
    MsgList_t input_0;
    MsgList_init(input_0);
    MsgList_t input_1;
    MsgList_init(input_1);
    MsgList_t expected_0;
    MsgList_init(expected_0);
    MsgList_t expected_1;
    MsgList_init(expected_1);

    test_rpc_setup();

    test_rpc_setup_second_session();
    test_rpc_teardown_second_session();

    test_rpc_setup_second_session();

    test_rpc_add_ping_to_list(input_0, PING_REQUEST, 0);
    test_rpc_add_ping_to_list(input_1, PING_REQUEST, 1);
    test_rpc_add_ping_to_list(expected_0, PING_RESPONSE, 0);
    test_rpc_add_ping_to_list(expected_1, PING_RESPONSE, 1);

    test_rpc_encode_and_feed(input_0, 0);
    test_rpc_encode_and_feed(input_1, 1);
    test_rpc_decode_and_compare(expected_0, 0);
    test_rpc_decode_and_compare(expected_1, 1);

    test_rpc_free_msg_list(input_0);
    test_rpc_free_msg_list(input_1);
    test_rpc_free_msg_list(expected_0);
    test_rpc_free_msg_list(expected_1);

    test_rpc_teardown_second_session();
    test_rpc_teardown();
}

MU_TEST(test_rpc_multisession_storage) {
    MsgList_t input_0;
    MsgList_init(input_0);
    MsgList_t input_1;
    MsgList_init(input_1);
    MsgList_t expected_0;
    MsgList_init(expected_0);
    MsgList_t expected_1;
    MsgList_init(expected_1);

    test_rpc_storage_setup();
    test_rpc_setup_second_session();

    uint8_t pattern[16] = "0123456789abcdef";

    test_rpc_add_read_or_write_to_list(
        input_0, WRITE_REQUEST, TEST_DIR "file0.txt", pattern, sizeof(pattern), 1, ++command_id);
    test_rpc_add_empty_to_list(expected_0, PB_CommandStatus_OK, command_id);

    test_rpc_add_read_or_write_to_list(
        input_1, WRITE_REQUEST, TEST_DIR "file1.txt", pattern, sizeof(pattern), 1, ++command_id);
    test_rpc_add_empty_to_list(expected_1, PB_CommandStatus_OK, command_id);

    test_rpc_create_simple_message(
        MsgList_push_raw(input_0),
        PB_Main_storage_read_request_tag,
        TEST_DIR "file0.txt",
        ++command_id);
    test_rpc_add_read_or_write_to_list(
        expected_0, READ_RESPONSE, TEST_DIR "file0.txt", pattern, sizeof(pattern), 1, command_id);

    test_rpc_create_simple_message(
        MsgList_push_raw(input_1),
        PB_Main_storage_read_request_tag,
        TEST_DIR "file1.txt",
        ++command_id);
    test_rpc_add_read_or_write_to_list(
        expected_1, READ_RESPONSE, TEST_DIR "file1.txt", pattern, sizeof(pattern), 1, command_id);

    test_rpc_print_message_list(input_0);
    test_rpc_print_message_list(input_1);
    test_rpc_print_message_list(expected_0);
    test_rpc_print_message_list(expected_1);

    test_rpc_encode_and_feed(input_0, 0);
    test_rpc_encode_and_feed(input_1, 1);

    test_rpc_decode_and_compare(expected_0, 0);
    test_rpc_decode_and_compare(expected_1, 1);

    test_rpc_free_msg_list(input_0);
    test_rpc_free_msg_list(input_1);
    test_rpc_free_msg_list(expected_0);
    test_rpc_free_msg_list(expected_1);

    test_rpc_teardown_second_session();
    test_rpc_storage_teardown();
}

MU_TEST_SUITE(test_rpc_session) {
    MU_RUN_TEST(test_rpc_feed_rubbish);
    MU_RUN_TEST(test_rpc_multisession_ping);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(storage_sd_status(storage) != FSE_OK) {
        FURI_LOG_E(TAG, "SD card not mounted - skip storage tests");
    } else {
        MU_RUN_TEST(test_rpc_multisession_storage);
    }
    furi_record_close(RECORD_STORAGE);
}

int run_minunit_test_rpc() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(storage_sd_status(storage) != FSE_OK) {
        FURI_LOG_E(TAG, "SD card not mounted - skip storage tests");
    } else {
        MU_RUN_SUITE(test_rpc_storage);
    }
    furi_record_close(RECORD_STORAGE);
    MU_RUN_SUITE(test_rpc_system);
    MU_RUN_SUITE(test_rpc_app);
    MU_RUN_SUITE(test_rpc_session);

    return MU_EXIT_CODE;
}

int32_t delay_test_app(void* p) {
    int timeout = atoi((const char*)p);

    if(timeout > 0) {
        furi_delay_ms(timeout);
    }

    return 0;
}
