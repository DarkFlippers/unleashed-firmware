#include <core/log.h>
#include <core/record.h>
#include "storage.h"
#include "storage_i.h"
#include "storage_message.h"
#include <toolbox/stream/file_stream.h>
#include <toolbox/dir_walk.h>
#include "toolbox/path.h"

#define MAX_NAME_LENGTH 256
#define MAX_EXT_LEN 16

#define TAG "StorageAPI"

#define S_API_PROLOGUE FuriApiLock lock = api_lock_alloc_locked();

#define S_FILE_API_PROLOGUE           \
    Storage* storage = file->storage; \
    furi_assert(storage);

#define S_API_EPILOGUE                                                               \
    furi_check(                                                                      \
        furi_message_queue_put(storage->message_queue, &message, FuriWaitForever) == \
        FuriStatusOk);                                                               \
    api_lock_wait_unlock_and_free(lock)

#define S_API_MESSAGE(_command)      \
    SAReturn return_data;            \
    StorageMessage message = {       \
        .lock = lock,                \
        .command = _command,         \
        .data = &data,               \
        .return_data = &return_data, \
    };

#define S_API_DATA_FILE   \
    SAData data = {       \
        .file = {         \
            .file = file, \
        }};

#define S_RETURN_BOOL (return_data.bool_value);
#define S_RETURN_UINT16 (return_data.uint16_value);
#define S_RETURN_UINT64 (return_data.uint64_value);
#define S_RETURN_ERROR (return_data.error_value);
#define S_RETURN_CSTRING (return_data.cstring_value);

typedef enum {
    StorageEventFlagFileClose = (1 << 0),
} StorageEventFlag;
/****************** FILE ******************/

static bool storage_file_open_internal(
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
            .thread_id = furi_thread_get_current_id(),
        }};

    file->type = FileTypeOpenFile;

    S_API_MESSAGE(StorageCommandFileOpen);
    S_API_EPILOGUE;

    return S_RETURN_BOOL;
}

static void storage_file_close_callback(const void* message, void* context) {
    const StorageEvent* storage_event = message;

    if(storage_event->type == StorageEventTypeFileClose ||
       storage_event->type == StorageEventTypeDirClose) {
        furi_assert(context);
        FuriEventFlag* event = context;
        furi_event_flag_set(event, StorageEventFlagFileClose);
    }
}

bool storage_file_open(
    File* file,
    const char* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    bool result;
    FuriEventFlag* event = furi_event_flag_alloc();
    FuriPubSubSubscription* subscription = furi_pubsub_subscribe(
        storage_get_pubsub(file->storage), storage_file_close_callback, event);

    do {
        result = storage_file_open_internal(file, path, access_mode, open_mode);

        if(!result && file->error_id == FSE_ALREADY_OPEN) {
            furi_event_flag_wait(
                event, StorageEventFlagFileClose, FuriFlagWaitAny, FuriWaitForever);
        } else {
            break;
        }
    } while(true);

    furi_pubsub_unsubscribe(storage_get_pubsub(file->storage), subscription);
    furi_event_flag_free(event);

    FURI_LOG_T(
        TAG,
        "File %p - %p open (%s)",
        (void*)((uint32_t)file - SRAM_BASE),
        (void*)(file->file_id - SRAM_BASE),
        path);

    return result;
}

bool storage_file_close(File* file) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;

    S_API_DATA_FILE;
    S_API_MESSAGE(StorageCommandFileClose);
    S_API_EPILOGUE;

    FURI_LOG_T(
        TAG,
        "File %p - %p closed",
        (void*)((uint32_t)file - SRAM_BASE),
        (void*)(file->file_id - SRAM_BASE));
    file->type = FileTypeClosed;

    return S_RETURN_BOOL;
}

uint16_t storage_file_read(File* file, void* buff, uint16_t bytes_to_read) {
    if(bytes_to_read == 0) {
        return 0;
    }

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
    if(bytes_to_write == 0) {
        return 0;
    }

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

bool storage_file_exists(Storage* storage, const char* path) {
    bool exist = false;
    FileInfo fileinfo;
    FS_Error error = storage_common_stat(storage, path, &fileinfo);

    if(error == FSE_OK && !file_info_is_dir(&fileinfo)) {
        exist = true;
    }

    return exist;
}

/****************** DIR ******************/

static bool storage_dir_open_internal(File* file, const char* path) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;

    SAData data = {
        .dopen = {
            .file = file,
            .path = path,
            .thread_id = furi_thread_get_current_id(),
        }};

    file->type = FileTypeOpenDir;

    S_API_MESSAGE(StorageCommandDirOpen);
    S_API_EPILOGUE;
    return S_RETURN_BOOL;
}

bool storage_dir_open(File* file, const char* path) {
    bool result;
    FuriEventFlag* event = furi_event_flag_alloc();
    FuriPubSubSubscription* subscription = furi_pubsub_subscribe(
        storage_get_pubsub(file->storage), storage_file_close_callback, event);

    do {
        result = storage_dir_open_internal(file, path);

        if(!result && file->error_id == FSE_ALREADY_OPEN) {
            furi_event_flag_wait(
                event, StorageEventFlagFileClose, FuriFlagWaitAny, FuriWaitForever);
        } else {
            break;
        }
    } while(true);

    furi_pubsub_unsubscribe(storage_get_pubsub(file->storage), subscription);
    furi_event_flag_free(event);

    FURI_LOG_T(
        TAG,
        "Dir %p - %p open (%s)",
        (void*)((uint32_t)file - SRAM_BASE),
        (void*)(file->file_id - SRAM_BASE),
        path);

    return result;
}

bool storage_dir_close(File* file) {
    S_FILE_API_PROLOGUE;
    S_API_PROLOGUE;
    S_API_DATA_FILE;
    S_API_MESSAGE(StorageCommandDirClose);
    S_API_EPILOGUE;

    FURI_LOG_T(
        TAG,
        "Dir %p - %p closed",
        (void*)((uint32_t)file - SRAM_BASE),
        (void*)(file->file_id - SRAM_BASE));

    file->type = FileTypeClosed;

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

bool storage_dir_exists(Storage* storage, const char* path) {
    bool exist = false;
    FileInfo fileinfo;
    FS_Error error = storage_common_stat(storage, path, &fileinfo);

    if(error == FSE_OK && file_info_is_dir(&fileinfo)) {
        exist = true;
    }

    return exist;
}
/****************** COMMON ******************/

FS_Error storage_common_timestamp(Storage* storage, const char* path, uint32_t* timestamp) {
    S_API_PROLOGUE;

    SAData data = {
        .ctimestamp = {
            .path = path,
            .timestamp = timestamp,
            .thread_id = furi_thread_get_current_id(),
        }};

    S_API_MESSAGE(StorageCommandCommonTimestamp);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_common_stat(Storage* storage, const char* path, FileInfo* fileinfo) {
    S_API_PROLOGUE;
    SAData data = {
        .cstat = {
            .path = path,
            .fileinfo = fileinfo,
            .thread_id = furi_thread_get_current_id(),
        }};

    S_API_MESSAGE(StorageCommandCommonStat);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_common_remove(Storage* storage, const char* path) {
    S_API_PROLOGUE;
    SAData data = {
        .path = {
            .path = path,
            .thread_id = furi_thread_get_current_id(),
        }};

    S_API_MESSAGE(StorageCommandCommonRemove);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

FS_Error storage_common_rename(Storage* storage, const char* old_path, const char* new_path) {
    FS_Error error = storage_common_copy(storage, old_path, new_path);
    if(error == FSE_OK) {
        if(!storage_simply_remove_recursive(storage, old_path)) {
            error = FSE_INTERNAL;
        }
    }

    return error;
}

static FS_Error
    storage_copy_recursive(Storage* storage, const char* old_path, const char* new_path) {
    FS_Error error = storage_common_mkdir(storage, new_path);
    DirWalk* dir_walk = dir_walk_alloc(storage);
    FuriString* path;
    FuriString* tmp_new_path;
    FuriString* tmp_old_path;
    FileInfo fileinfo;
    path = furi_string_alloc();
    tmp_new_path = furi_string_alloc();
    tmp_old_path = furi_string_alloc();

    do {
        if(error != FSE_OK) break;

        if(!dir_walk_open(dir_walk, old_path)) {
            error = dir_walk_get_error(dir_walk);
            break;
        }

        while(1) {
            DirWalkResult res = dir_walk_read(dir_walk, path, &fileinfo);

            if(res == DirWalkError) {
                error = dir_walk_get_error(dir_walk);
                break;
            } else if(res == DirWalkLast) {
                break;
            } else {
                furi_string_set(tmp_old_path, path);
                furi_string_right(path, strlen(old_path));
                furi_string_printf(tmp_new_path, "%s%s", new_path, furi_string_get_cstr(path));

                if(file_info_is_dir(&fileinfo)) {
                    error = storage_common_mkdir(storage, furi_string_get_cstr(tmp_new_path));
                } else {
                    error = storage_common_copy(
                        storage,
                        furi_string_get_cstr(tmp_old_path),
                        furi_string_get_cstr(tmp_new_path));
                }

                if(error != FSE_OK) break;
            }
        }

    } while(false);

    furi_string_free(tmp_new_path);
    furi_string_free(tmp_old_path);
    furi_string_free(path);
    dir_walk_free(dir_walk);
    return error;
}

FS_Error storage_common_copy(Storage* storage, const char* old_path, const char* new_path) {
    FS_Error error;

    FileInfo fileinfo;
    error = storage_common_stat(storage, old_path, &fileinfo);

    if(error == FSE_OK) {
        if(file_info_is_dir(&fileinfo)) {
            error = storage_copy_recursive(storage, old_path, new_path);
        } else {
            Stream* stream_from = file_stream_alloc(storage);
            Stream* stream_to = file_stream_alloc(storage);

            do {
                if(!file_stream_open(stream_from, old_path, FSAM_READ, FSOM_OPEN_EXISTING)) break;
                if(!file_stream_open(stream_to, new_path, FSAM_WRITE, FSOM_CREATE_NEW)) break;
                stream_copy_full(stream_from, stream_to);
            } while(false);

            error = file_stream_get_error(stream_from);
            if(error == FSE_OK) {
                error = file_stream_get_error(stream_to);
            }

            stream_free(stream_from);
            stream_free(stream_to);
        }
    }

    return error;
}

static FS_Error
    storage_merge_recursive(Storage* storage, const char* old_path, const char* new_path) {
    FS_Error error = FSE_OK;
    DirWalk* dir_walk = dir_walk_alloc(storage);
    FuriString *path, *file_basename, *tmp_new_path;
    FileInfo fileinfo;
    path = furi_string_alloc();
    file_basename = furi_string_alloc();
    tmp_new_path = furi_string_alloc();

    do {
        if(!storage_simply_mkdir(storage, new_path)) break;

        dir_walk_set_recursive(dir_walk, false);
        if(!dir_walk_open(dir_walk, old_path)) {
            error = dir_walk_get_error(dir_walk);
            break;
        }

        while(1) {
            DirWalkResult res = dir_walk_read(dir_walk, path, &fileinfo);

            if(res == DirWalkError) {
                error = dir_walk_get_error(dir_walk);
                break;
            } else if(res == DirWalkLast) {
                break;
            } else {
                path_extract_basename(furi_string_get_cstr(path), file_basename);
                path_concat(new_path, furi_string_get_cstr(file_basename), tmp_new_path);

                if(file_info_is_dir(&fileinfo)) {
                    if(storage_common_stat(
                           storage, furi_string_get_cstr(tmp_new_path), &fileinfo) == FSE_OK) {
                        if(file_info_is_dir(&fileinfo)) {
                            error =
                                storage_common_mkdir(storage, furi_string_get_cstr(tmp_new_path));
                            if(error != FSE_OK && error != FSE_EXIST) {
                                break;
                            }
                        }
                    }
                }
                error = storage_common_merge(
                    storage, furi_string_get_cstr(path), furi_string_get_cstr(tmp_new_path));

                if(error != FSE_OK) {
                    break;
                }
            }
        }

    } while(false);

    furi_string_free(tmp_new_path);
    furi_string_free(file_basename);
    furi_string_free(path);
    dir_walk_free(dir_walk);
    return error;
}

FS_Error storage_common_merge(Storage* storage, const char* old_path, const char* new_path) {
    FS_Error error;
    const char* new_path_tmp = NULL;
    FuriString* new_path_next = NULL;
    new_path_next = furi_string_alloc();

    FileInfo fileinfo;
    error = storage_common_stat(storage, old_path, &fileinfo);

    if(error == FSE_OK) {
        if(file_info_is_dir(&fileinfo)) {
            error = storage_merge_recursive(storage, old_path, new_path);
        } else {
            error = storage_common_stat(storage, new_path, &fileinfo);
            if(error == FSE_OK) {
                furi_string_set(new_path_next, new_path);
                FuriString* dir_path;
                FuriString* filename;
                char extension[MAX_EXT_LEN] = {0};

                dir_path = furi_string_alloc();
                filename = furi_string_alloc();

                path_extract_filename(new_path_next, filename, true);
                path_extract_dirname(new_path, dir_path);
                path_extract_extension(new_path_next, extension, MAX_EXT_LEN);

                storage_get_next_filename(
                    storage,
                    furi_string_get_cstr(dir_path),
                    furi_string_get_cstr(filename),
                    extension,
                    new_path_next,
                    255);
                furi_string_cat_printf(
                    dir_path, "/%s%s", furi_string_get_cstr(new_path_next), extension);
                furi_string_set(new_path_next, dir_path);

                furi_string_free(dir_path);
                furi_string_free(filename);
                new_path_tmp = furi_string_get_cstr(new_path_next);
            } else {
                new_path_tmp = new_path;
            }
            Stream* stream_from = file_stream_alloc(storage);
            Stream* stream_to = file_stream_alloc(storage);

            do {
                if(!file_stream_open(stream_from, old_path, FSAM_READ, FSOM_OPEN_EXISTING)) break;
                if(!file_stream_open(stream_to, new_path_tmp, FSAM_WRITE, FSOM_CREATE_NEW)) break;
                stream_copy_full(stream_from, stream_to);
            } while(false);

            error = file_stream_get_error(stream_from);
            if(error == FSE_OK) {
                error = file_stream_get_error(stream_to);
            }

            stream_free(stream_from);
            stream_free(stream_to);
        }
    }

    furi_string_free(new_path_next);

    return error;
}

FS_Error storage_common_mkdir(Storage* storage, const char* path) {
    S_API_PROLOGUE;
    SAData data = {
        .path = {
            .path = path,
            .thread_id = furi_thread_get_current_id(),
        }};

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
            .thread_id = furi_thread_get_current_id(),
        }};

    S_API_MESSAGE(StorageCommandCommonFSInfo);
    S_API_EPILOGUE;
    return S_RETURN_ERROR;
}

void storage_common_resolve_path_and_ensure_app_directory(Storage* storage, FuriString* path) {
    S_API_PROLOGUE;

    SAData data = {
        .cresolvepath = {
            .path = path,
            .thread_id = furi_thread_get_current_id(),
        }};

    S_API_MESSAGE(StorageCommandCommonResolvePath);
    S_API_EPILOGUE;
}

FS_Error storage_common_migrate(Storage* storage, const char* source, const char* dest) {
    if(!storage_common_exists(storage, source)) {
        return FSE_OK;
    }

    FS_Error error = storage_common_merge(storage, source, dest);

    if(error == FSE_OK) {
        storage_simply_remove_recursive(storage, source);
    }

    return error;
}

bool storage_common_exists(Storage* storage, const char* path) {
    FileInfo file_info;
    return storage_common_stat(storage, path, &file_info) == FSE_OK;
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
    File* file = malloc(sizeof(File));
    file->type = FileTypeClosed;
    file->storage = storage;

    FURI_LOG_T(TAG, "File/Dir %p alloc", (void*)((uint32_t)file - SRAM_BASE));

    return file;
}

bool storage_file_is_open(File* file) {
    return (file->type != FileTypeClosed);
}

bool storage_file_is_dir(File* file) {
    return (file->type == FileTypeOpenDir);
}

void storage_file_free(File* file) {
    if(storage_file_is_open(file)) {
        if(storage_file_is_dir(file)) {
            storage_dir_close(file);
        } else {
            storage_file_close(file);
        }
    }

    FURI_LOG_T(TAG, "File/Dir %p free", (void*)((uint32_t)file - SRAM_BASE));
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
    FuriString* fullname;
    FuriString* cur_dir;

    if(storage_simply_remove(storage, path)) {
        return true;
    }

    char* name = malloc(MAX_NAME_LENGTH + 1); //-V799
    File* dir = storage_file_alloc(storage);
    cur_dir = furi_string_alloc_set(path);
    bool go_deeper = false;

    while(1) {
        if(!storage_dir_open(dir, furi_string_get_cstr(cur_dir))) {
            storage_dir_close(dir);
            break;
        }

        while(storage_dir_read(dir, &fileinfo, name, MAX_NAME_LENGTH)) {
            if(file_info_is_dir(&fileinfo)) {
                furi_string_cat_printf(cur_dir, "/%s", name);
                go_deeper = true;
                break;
            }

            fullname = furi_string_alloc_printf("%s/%s", furi_string_get_cstr(cur_dir), name);
            FS_Error error = storage_common_remove(storage, furi_string_get_cstr(fullname));
            furi_check(error == FSE_OK);
            furi_string_free(fullname);
        }
        storage_dir_close(dir);

        if(go_deeper) {
            go_deeper = false;
            continue;
        }

        FS_Error error = storage_common_remove(storage, furi_string_get_cstr(cur_dir));
        furi_check(error == FSE_OK);

        if(furi_string_cmp(cur_dir, path)) {
            size_t last_char = furi_string_search_rchar(cur_dir, '/');
            furi_assert(last_char != FURI_STRING_FAILURE);
            furi_string_left(cur_dir, last_char);
        } else {
            result = true;
            break;
        }
    }

    storage_file_free(dir);
    furi_string_free(cur_dir);
    free(name);
    return result;
} //-V773

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
    FuriString* nextfilename,
    uint8_t max_len) {
    FuriString* temp_str;
    uint16_t num = 0;

    temp_str = furi_string_alloc_printf("%s/%s%s", dirname, filename, fileextension);

    while(storage_common_stat(storage, furi_string_get_cstr(temp_str), NULL) == FSE_OK) {
        num++;
        furi_string_printf(temp_str, "%s/%s%d%s", dirname, filename, num, fileextension);
    }
    if(num && (max_len > strlen(filename))) {
        furi_string_printf(nextfilename, "%s%d", filename, num);
    } else {
        furi_string_printf(nextfilename, "%s", filename);
    }

    furi_string_free(temp_str);
}
