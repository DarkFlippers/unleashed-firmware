#include "flipper.pb.h"
#include "furi/common_defines.h"
#include "furi/memmgr.h"
#include "furi/record.h"
#include "pb_decode.h"
#include "rpc/rpc.h"
#include "rpc_i.h"
#include "storage.pb.h"
#include "storage/filesystem-api-defines.h"
#include "storage/storage.h"
#include <stdint.h>
#include <lib/toolbox/md5.h>

#define RPC_TAG "RPC_STORAGE"
#define MAX_NAME_LENGTH 255
#define MAX_DATA_SIZE 512

typedef enum {
    RpcStorageStateIdle = 0,
    RpcStorageStateWriting,
} RpcStorageState;

typedef struct {
    Rpc* rpc;
    Storage* api;
    File* file;
    RpcStorageState state;
    uint32_t current_command_id;
} RpcStorageSystem;

void rpc_print_message(const PB_Main* message);

static void rpc_system_storage_reset_state(RpcStorageSystem* rpc_storage, bool send_error) {
    furi_assert(rpc_storage);

    if(rpc_storage->state != RpcStorageStateIdle) {
        if(send_error) {
            rpc_encode_and_send_empty(
                rpc_storage->rpc,
                rpc_storage->current_command_id,
                PB_CommandStatus_ERROR_CONTINUOUS_COMMAND_INTERRUPTED);
        }

        if(rpc_storage->state == RpcStorageStateWriting) {
            storage_file_close(rpc_storage->file);
            storage_file_free(rpc_storage->file);
            furi_record_close("storage");
        }

        rpc_storage->state = RpcStorageStateIdle;
    }
}

static PB_CommandStatus rpc_system_storage_get_error(FS_Error fs_error) {
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

static PB_CommandStatus rpc_system_storage_get_file_error(File* file) {
    return rpc_system_storage_get_error(storage_file_get_error(file));
}

static void rpc_system_storage_list_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(context);
    furi_assert(request->which_content == PB_Main_storage_list_request_tag);

    RpcStorageSystem* rpc_storage = context;
    rpc_system_storage_reset_state(rpc_storage, true);

    Storage* fs_api = furi_record_open("storage");
    File* dir = storage_file_alloc(fs_api);

    PB_Main response = {
        .command_id = request->command_id,
        .has_next = false,
        .which_content = PB_Main_storage_list_request_tag,
        .command_status = PB_CommandStatus_OK,
    };
    PB_Storage_ListResponse* list = &response.content.storage_list_response;
    response.which_content = PB_Main_storage_list_response_tag;

    bool finish = false;
    int i = 0;

    if(!storage_dir_open(dir, request->content.storage_list_request.path)) {
        response.command_status = rpc_system_storage_get_file_error(dir);
        response.which_content = PB_Main_empty_tag;
        finish = true;
    }

    while(!finish) {
        FileInfo fileinfo;
        char* name = furi_alloc(MAX_NAME_LENGTH + 1);
        if(storage_dir_read(dir, &fileinfo, name, MAX_NAME_LENGTH)) {
            if(i == COUNT_OF(list->file)) {
                list->file_count = i;
                response.has_next = true;
                rpc_encode_and_send(rpc_storage->rpc, &response);
                pb_release(&PB_Main_msg, &response);
                i = 0;
            }
            list->file[i].type = (fileinfo.flags & FSF_DIRECTORY) ? PB_Storage_File_FileType_DIR :
                                                                    PB_Storage_File_FileType_FILE;
            list->file[i].size = fileinfo.size;
            list->file[i].data = NULL;
            list->file[i].name = name;
            ++i;
        } else {
            list->file_count = i;
            finish = true;
            free(name);
        }
    }

    response.has_next = false;
    rpc_encode_and_send(rpc_storage->rpc, &response);
    pb_release(&PB_Main_msg, &response);

    storage_dir_close(dir);
    storage_file_free(dir);

    furi_record_close("storage");
}

static void rpc_system_storage_read_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_storage_read_request_tag);

    RpcStorageSystem* rpc_storage = context;
    rpc_system_storage_reset_state(rpc_storage, true);

    /* use same message memory to send reponse */
    PB_Main* response = furi_alloc(sizeof(PB_Main));
    response->command_id = request->command_id;
    response->which_content = PB_Main_storage_read_response_tag;
    response->command_status = PB_CommandStatus_OK;
    const char* path = request->content.storage_read_request.path;
    Storage* fs_api = furi_record_open("storage");
    File* file = storage_file_alloc(fs_api);
    bool result = false;

    if(storage_file_open(file, path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        size_t size_left = storage_file_size(file);
        response->content.storage_read_response.has_file = true;
        response->content.storage_read_response.file.data =
            furi_alloc(PB_BYTES_ARRAY_T_ALLOCSIZE(MIN(size_left, MAX_DATA_SIZE)));
        do {
            uint8_t* buffer = response->content.storage_read_response.file.data->bytes;
            uint16_t* read_size_msg = &response->content.storage_read_response.file.data->size;

            size_t read_size = MIN(size_left, MAX_DATA_SIZE);
            *read_size_msg = storage_file_read(file, buffer, read_size);
            size_left -= read_size;
            result = (*read_size_msg == read_size);

            if(result) {
                response->has_next = (size_left > 0);
                rpc_encode_and_send(rpc_storage->rpc, response);
                // no pb_release(...);
            }
        } while((size_left != 0) && result);

        if(!result) {
            rpc_encode_and_send_empty(
                rpc_storage->rpc, request->command_id, rpc_system_storage_get_file_error(file));
        }
    } else {
        rpc_encode_and_send_empty(
            rpc_storage->rpc, request->command_id, rpc_system_storage_get_file_error(file));
    }

    pb_release(&PB_Main_msg, response);
    free(response);
    storage_file_close(file);
    storage_file_free(file);

    furi_record_close("storage");
}

static void rpc_system_storage_write_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_storage_write_request_tag);

    RpcStorageSystem* rpc_storage = context;
    bool result = true;

    if((request->command_id != rpc_storage->current_command_id) &&
       (rpc_storage->state == RpcStorageStateWriting)) {
        rpc_system_storage_reset_state(rpc_storage, true);
    }

    if(rpc_storage->state != RpcStorageStateWriting) {
        rpc_storage->api = furi_record_open("storage");
        rpc_storage->file = storage_file_alloc(rpc_storage->api);
        rpc_storage->current_command_id = request->command_id;
        rpc_storage->state = RpcStorageStateWriting;
        const char* path = request->content.storage_write_request.path;
        result = storage_file_open(rpc_storage->file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    }

    File* file = rpc_storage->file;

    if(result) {
        uint8_t* buffer = request->content.storage_write_request.file.data->bytes;
        size_t buffer_size = request->content.storage_write_request.file.data->size;

        uint16_t written_size = storage_file_write(file, buffer, buffer_size);
        result = (written_size == buffer_size);

        if(result && !request->has_next) {
            rpc_encode_and_send_empty(
                rpc_storage->rpc, rpc_storage->current_command_id, PB_CommandStatus_OK);
            rpc_system_storage_reset_state(rpc_storage, false);
        }
    }

    if(!result) {
        rpc_encode_and_send_empty(
            rpc_storage->rpc,
            rpc_storage->current_command_id,
            rpc_system_storage_get_file_error(file));
        rpc_system_storage_reset_state(rpc_storage, false);
    }
}

static void rpc_system_storage_delete_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_storage_delete_request_tag);
    furi_assert(context);
    RpcStorageSystem* rpc_storage = context;
    PB_CommandStatus status;
    rpc_system_storage_reset_state(rpc_storage, true);

    Storage* fs_api = furi_record_open("storage");
    char* path = request->content.storage_mkdir_request.path;
    if(path) {
        FS_Error error = storage_common_remove(fs_api, path);
        status = rpc_system_storage_get_error(error);
    } else {
        status = PB_CommandStatus_ERROR_INVALID_PARAMETERS;
    }
    rpc_encode_and_send_empty(rpc_storage->rpc, request->command_id, status);
}

static void rpc_system_storage_mkdir_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_storage_mkdir_request_tag);
    furi_assert(context);
    RpcStorageSystem* rpc_storage = context;
    PB_CommandStatus status;
    rpc_system_storage_reset_state(rpc_storage, true);

    Storage* fs_api = furi_record_open("storage");
    char* path = request->content.storage_mkdir_request.path;
    if(path) {
        FS_Error error = storage_common_mkdir(fs_api, path);
        status = rpc_system_storage_get_error(error);
    } else {
        status = PB_CommandStatus_ERROR_INVALID_PARAMETERS;
    }
    rpc_encode_and_send_empty(rpc_storage->rpc, request->command_id, status);
}

static void rpc_system_storage_md5sum_process(const PB_Main* request, void* context) {
    furi_assert(request);
    furi_assert(request->which_content == PB_Main_storage_md5sum_request_tag);
    furi_assert(context);
    RpcStorageSystem* rpc_storage = context;
    rpc_system_storage_reset_state(rpc_storage, true);

    const char* filename = request->content.storage_md5sum_request.path;
    if(!filename) {
        rpc_encode_and_send_empty(
            rpc_storage->rpc, request->command_id, PB_CommandStatus_ERROR_INVALID_PARAMETERS);
        return;
    }

    Storage* fs_api = furi_record_open("storage");
    File* file = storage_file_alloc(fs_api);

    if(storage_file_open(file, filename, FSAM_READ, FSOM_OPEN_EXISTING)) {
        const uint16_t read_size = 512;
        const uint8_t hash_size = 16;
        uint8_t* data = malloc(read_size);
        uint8_t* hash = malloc(sizeof(uint8_t) * hash_size);
        md5_context* md5_ctx = malloc(sizeof(md5_context));

        md5_starts(md5_ctx);
        while(true) {
            uint16_t readed_size = storage_file_read(file, data, read_size);
            if(readed_size == 0) break;
            md5_update(md5_ctx, data, readed_size);
        }
        md5_finish(md5_ctx, hash);
        free(md5_ctx);

        PB_Main response = {
            .command_id = request->command_id,
            .command_status = PB_CommandStatus_OK,
            .which_content = PB_Main_storage_md5sum_response_tag,
            .has_next = false,
        };

        char* md5sum = response.content.storage_md5sum_response.md5sum;
        size_t md5sum_size = sizeof(response.content.storage_md5sum_response.md5sum);
        (void)md5sum_size;
        furi_assert(hash_size <= ((md5sum_size - 1) / 2));
        for(uint8_t i = 0; i < hash_size; i++) {
            md5sum += sprintf(md5sum, "%02x", hash[i]);
        }

        free(hash);
        free(data);
        storage_file_close(file);
        rpc_encode_and_send(rpc_storage->rpc, &response);
    } else {
        rpc_encode_and_send_empty(
            rpc_storage->rpc, request->command_id, rpc_system_storage_get_file_error(file));
    }

    storage_file_free(file);

    furi_record_close("storage");
}

void* rpc_system_storage_alloc(Rpc* rpc) {
    furi_assert(rpc);

    RpcStorageSystem* rpc_storage = furi_alloc(sizeof(RpcStorageSystem));
    rpc_storage->api = furi_record_open("storage");
    rpc_storage->rpc = rpc;
    rpc_storage->state = RpcStorageStateIdle;

    RpcHandler rpc_handler = {
        .message_handler = NULL,
        .decode_submessage = NULL,
        .context = rpc_storage,
    };

    rpc_handler.message_handler = rpc_system_storage_list_process;
    rpc_add_handler(rpc, PB_Main_storage_list_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_storage_read_process;
    rpc_add_handler(rpc, PB_Main_storage_read_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_storage_write_process;
    rpc_add_handler(rpc, PB_Main_storage_write_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_storage_delete_process;
    rpc_add_handler(rpc, PB_Main_storage_delete_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_storage_mkdir_process;
    rpc_add_handler(rpc, PB_Main_storage_mkdir_request_tag, &rpc_handler);

    rpc_handler.message_handler = rpc_system_storage_md5sum_process;
    rpc_add_handler(rpc, PB_Main_storage_md5sum_request_tag, &rpc_handler);

    return rpc_storage;
}

void rpc_system_storage_free(void* ctx) {
    RpcStorageSystem* rpc_storage = ctx;
    rpc_system_storage_reset_state(rpc_storage, false);
    free(rpc_storage);
}
