#include <furi/record.h>
#include <m-string.h>
#include "storage.h"
#include "storage-i.h"
#include "storage-message.h"

#define MAX_NAME_LENGTH 256

#define S_API_PROLOGUE                                      \
    osSemaphoreId_t semaphore = osSemaphoreNew(1, 0, NULL); \
    furi_check(semaphore != NULL);

#define S_FILE_API_PROLOGUE           \
    Storage* storage = file->storage; \
    furi_assert(storage);

#define S_API_EPILOGUE                                                                         \
    furi_check(osMessageQueuePut(storage->message_queue, &message, 0, osWaitForever) == osOK); \
    osSemaphoreAcquire(semaphore, osWaitForever);                                              \
    osSemaphoreDelete(semaphore);

#define S_API_MESSAGE(_command)      \
    SAReturn return_data;            \
    StorageMessage message = {       \
        .semaphore = semaphore,      \
        .command = _command,         \
        .data = &data,               \
        .return_data = &return_data, \
    };

#define S_API_DATA_FILE   \
    SAData data = {       \
        .file = {         \
            .file = file, \
        }};

#define S_API_DATA_PATH   \
    SAData data = {       \
        .path = {         \
            .path = path, \
        }};

#define S_RETURN_BOOL (return_data.bool_value);
#define S_RETURN_UINT16 (return_data.uint16_value);
#define S_RETURN_UINT64 (return_data.uint64_value);
#define S_RETURN_ERROR (return_data.error_value);
#define S_RETURN_CSTRING (return_data.cstring_value);

#define FILE_OPENED 1
#define FILE_CLOSED 0

/****************** FILE ******************/

bool storage_file_open(
    File* file,
    const char* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;

    SAData data = {
        .fopen = {
            .file = file,
            .path = path,
            .access_mode = access_mode,
            .open_mode = open_mode,
        }};

    file->file_id = FILE_OPENED;

    S_API_MESSAGE(StorageCommandFileOpen);
    S_API_EPILOGUE;

    return S_RETURN_BOOL;
}

bool storage_file_close(File* file) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;

    S_API_DATA_FILE;
    S_API_MESSAGE(StorageCommandFileClose);
    S_API_EPILOGUE;

    file->file_id = FILE_CLOSED;

    return S_RETURN_BOOL;
}

uint16_t storage_file_read(File* file, void* buff, uint16_t bytes_to_read) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;

    SAData data = {
        .fread = {
            .file = file,
            .buff = buff,
            .bytes_to_read = bytes_to_read,
        }};

    S_API_MESSAGE(StorageCommandFileRead);
    S_API_EPILOGUE;
    return S_RETURN_UINT16;
}

uint16_t storage_file_write(File* file, const void* buff, uint16_t bytes_to_write) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;

    SAData data = {
        .fwrite = {
            .file = file,
            .buff = buff,
            .bytes_to_write = bytes_to_write,
        }};

    S_API_MESSAGE(StorageCommandFileWrite);
    S_API_EPILOGUE;
    return S_RETURN_UINT16;
}

bool storage_file_seek(File* file, uint32_t offset, bool from_start) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;

    SAData data = {
        .fseek = {
            .file = file,
            .offset = offset,
            .from_start = from_start,
        }};

    S_API_MESSAGE(StorageCommandFileSeek);
    S_API_EPILOGUE;
    return S_RETURN_BOOL;
}

uint64_t storage_file_tell(File* file) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;
    S_API_DATA_FILE;
    S_API_MESSAGE(StorageCommandFileTell);
    S_API_EPILOGUE;
    return S_RETURN_UINT64;
}

bool storage_file_truncate(File* file) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;
    S_API_DATA_FILE;
    S_API_MESSAGE(StorageCommandFileTruncate);
    S_API_EPILOGUE;
    return S_RETURN_BOOL;
}

uint64_t storage_file_size(File* file) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;
    S_API_DATA_FILE;
    S_API_MESSAGE(StorageCommandFileSize);
    S_API_EPILOGUE;
    return S_RETURN_UINT64;
}

bool storage_file_sync(File* file) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;
    S_API_DATA_FILE;
    S_API_MESSAGE(StorageCommandFileSync);
    S_API_EPILOGUE;
    return S_RETURN_BOOL;
}

bool storage_file_eof(File* file) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;
    S_API_DATA_FILE;
    S_API_MESSAGE(StorageCommandFileEof);
    S_API_EPILOGUE;
    return S_RETURN_BOOL;
}

/****************** DIR ******************/

bool storage_dir_open(File* file, const char* path) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;

    SAData data = {
        .dopen = {
            .file = file,
            .path = path,
        }};

    file->file_id = FILE_OPENED;

    S_API_MESSAGE(StorageCommandDirOpen);
    S_API_EPILOGUE;
    return S_RETURN_BOOL;
}

bool storage_dir_close(File* file) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;
    S_API_DATA_FILE;
    S_API_MESSAGE(StorageCommandDirClose);
    S_API_EPILOGUE;

    file->file_id = FILE_CLOSED;

    return S_RETURN_BOOL;
}

bool storage_dir_read(File* file, FileInfo* fileinfo, char* name, uint16_t name_length) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;

    SAData data = {
        .dread = {
            .file = file,
            .fileinfo = fileinfo,
            .name = name,
            .name_length = name_length,
        }};

    S_API_MESSAGE(StorageCommandDirRead);
    S_API_EPILOGUE;
    return S_RETURN_BOOL;
}

bool storage_dir_rewind(File* file) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;
    S_API_DATA_FILE;
    S_API_MESSAGE(StorageCommandDirRewind);
    S_API_EPILOGUE;
    return S_RETURN_BOOL;
}

/****************** COMMON ******************/

FS_Error storage_common_stat(Storage* storage, const char* path, FileInfo* fileinfo) {
    S_API_PROLOGUE;

    SAData data = {.cstat = {.path = path, .fileinfo = fileinfo}};

    S_API_MESSAGE(StorageCommandCommonStat);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_common_remove(Storage* storage, const char* path) {
    S_API_PROLOGUE;
    S_API_DATA_PATH;
    S_API_MESSAGE(StorageCommandCommonRemove);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_common_rename(Storage* storage, const char* old_path, const char* new_path) {
    S_API_PROLOGUE;

    SAData data = {
        .cpaths = {
            .old = old_path,
            .new = new_path,
        }};

    S_API_MESSAGE(StorageCommandCommonRename);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_common_copy(Storage* storage, const char* old_path, const char* new_path) {
    S_API_PROLOGUE;

    SAData data = {
        .cpaths = {
            .old = old_path,
            .new = new_path,
        }};

    S_API_MESSAGE(StorageCommandCommonCopy);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_common_mkdir(Storage* storage, const char* path) {
    S_API_PROLOGUE;
    S_API_DATA_PATH;
    S_API_MESSAGE(StorageCommandCommonMkDir);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_common_fs_info(
    Storage* storage,
    const char* fs_path,
    uint64_t* total_space,
    uint64_t* free_space) {
    S_API_PROLOGUE;

    SAData data = {
        .cfsinfo = {
            .fs_path = fs_path,
            .total_space = total_space,
            .free_space = free_space,
        }};

    S_API_MESSAGE(StorageCommandCommonFSInfo);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

/****************** ERROR ******************/

const char* storage_error_get_desc(FS_Error error_id) {
    return filesystem_api_error_get_desc(error_id);
}

FS_Error storage_file_get_error(File* file) {
    furi_check(file != NULL);
    return file->error_id;
}

int32_t storage_file_get_internal_error(File* file) {
    furi_check(file != NULL);
    return file->internal_error_id;
}

const char* storage_file_get_error_desc(File* file) {
    furi_check(file != NULL);
    return filesystem_api_error_get_desc(file->error_id);
}

/****************** Raw SD API ******************/

FS_Error storage_sd_format(Storage* storage) {
    S_API_PROLOGUE;
    SAData data = {};
    S_API_MESSAGE(StorageCommandSDFormat);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_sd_unmount(Storage* storage) {
    S_API_PROLOGUE;
    SAData data = {};
    S_API_MESSAGE(StorageCommandSDUnmount);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_sd_info(Storage* storage, SDInfo* info) {
    S_API_PROLOGUE;
    SAData data = {
        .sdinfo = {
            .info = info,
        }};
    S_API_MESSAGE(StorageCommandSDInfo);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_sd_status(Storage* storage) {
    S_API_PROLOGUE;
    SAData data = {};
    S_API_MESSAGE(StorageCommandSDStatus);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

File* storage_file_alloc(Storage* storage) {
    File* file = furi_alloc(sizeof(File));
    file->file_id = FILE_CLOSED;
    file->storage = storage;

    return file;
}

bool storage_file_is_open(File* file) {
    return (file->file_id != FILE_CLOSED);
}

void storage_file_free(File* file) {
    if(storage_file_is_open(file)) {
        storage_file_close(file);
    }

    free(file);
}

FuriPubSub* storage_get_pubsub(Storage* storage) {
    return storage->pubsub;
}

bool storage_simply_remove_recursive(Storage* storage, const char* path) {
    furi_assert(storage);
    furi_assert(path);
    FileInfo fileinfo;
    bool result = false;
    string_t fullname;
    string_t cur_dir;

    if(storage_simply_remove(storage, path)) {
        return true;
    }

    char* name = furi_alloc(MAX_NAME_LENGTH + 1);
    File* dir = storage_file_alloc(storage);
    string_init_set_str(cur_dir, path);
    bool go_deeper = false;

    while(1) {
        if(!storage_dir_open(dir, string_get_cstr(cur_dir))) {
            storage_dir_close(dir);
            break;
        }

        while(storage_dir_read(dir, &fileinfo, name, MAX_NAME_LENGTH)) {
            if(fileinfo.flags & FSF_DIRECTORY) {
                string_cat_printf(cur_dir, "/%s", name);
                go_deeper = true;
                break;
            }

            string_init_printf(fullname, "%s/%s", string_get_cstr(cur_dir), name);
            FS_Error error = storage_common_remove(storage, string_get_cstr(fullname));
            furi_check(error == FSE_OK);
            string_clear(fullname);
        }
        storage_dir_close(dir);

        if(go_deeper) {
            go_deeper = false;
            continue;
        }

        FS_Error error = storage_common_remove(storage, string_get_cstr(cur_dir));
        furi_check(error == FSE_OK);

        if(string_cmp(cur_dir, path)) {
            size_t last_char = string_search_rchar(cur_dir, '/');
            furi_assert(last_char != STRING_FAILURE);
            string_left(cur_dir, last_char);
        } else {
            result = true;
            break;
        }
    }

    storage_file_free(dir);
    string_clear(cur_dir);
    free(name);
    return result;
}

bool storage_simply_remove(Storage* storage, const char* path) {
    FS_Error result;
    result = storage_common_remove(storage, path);
    return result == FSE_OK || result == FSE_NOT_EXIST;
}

bool storage_simply_mkdir(Storage* storage, const char* path) {
    FS_Error result;
    result = storage_common_mkdir(storage, path);
    return result == FSE_OK || result == FSE_EXIST;
}

void storage_get_next_filename(
    Storage* storage,
    const char* dirname,
    const char* filename,
    const char* fileextension,
    string_t nextfilename) {
    string_t temp_str;
    uint16_t num = 0;

    string_init_printf(temp_str, "%s/%s%s", dirname, filename, fileextension);

    while(storage_common_stat(storage, string_get_cstr(temp_str), NULL) == FSE_OK) {
        num++;
        string_printf(temp_str, "%s/%s%d%s", dirname, filename, num, fileextension);
    }

    if(num) {
        string_printf(nextfilename, "%s%d", filename, num);
    } else {
        string_printf(nextfilename, "%s", filename);
    }

    string_clear(temp_str);
}
