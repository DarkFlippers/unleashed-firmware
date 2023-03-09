#include "storage_processing.h"
#include <m-list.h>
#include <m-dict.h>

#define FS_CALL(_storage, _fn) ret = _storage->fs_api->_fn;

static bool storage_type_is_valid(StorageType type) {
#ifdef FURI_RAM_EXEC
    return type == ST_EXT;
#else
    return type < ST_ERROR;
#endif
}

static StorageData* get_storage_by_file(File* file, StorageData* storages) {
    StorageData* storage_data = NULL;

    for(uint8_t i = 0; i < STORAGE_COUNT; i++) {
        if(storage_has_file(file, &storages[i])) {
            storage_data = &storages[i];
        }
    }

    return storage_data;
}

static const char* cstr_path_without_vfs_prefix(FuriString* path) {
    const char* path_cstr = furi_string_get_cstr(path);
    return path_cstr + MIN(4u, strlen(path_cstr));
}

static StorageType storage_get_type_by_path(FuriString* path) {
    StorageType type = ST_ERROR;
    const char* path_cstr = furi_string_get_cstr(path);

    if(memcmp(path_cstr, STORAGE_EXT_PATH_PREFIX, strlen(STORAGE_EXT_PATH_PREFIX)) == 0) {
        type = ST_EXT;
    } else if(memcmp(path_cstr, STORAGE_INT_PATH_PREFIX, strlen(STORAGE_INT_PATH_PREFIX)) == 0) {
        type = ST_INT;
    } else if(memcmp(path_cstr, STORAGE_ANY_PATH_PREFIX, strlen(STORAGE_ANY_PATH_PREFIX)) == 0) {
        type = ST_ANY;
    }

    return type;
}

static void storage_path_change_to_real_storage(FuriString* path, StorageType real_storage) {
    if(furi_string_search(path, STORAGE_ANY_PATH_PREFIX) == 0) {
        switch(real_storage) {
        case ST_EXT:
            furi_string_replace_at(
                path, 0, strlen(STORAGE_EXT_PATH_PREFIX), STORAGE_EXT_PATH_PREFIX);
            break;
        case ST_INT:
            furi_string_replace_at(
                path, 0, strlen(STORAGE_INT_PATH_PREFIX), STORAGE_INT_PATH_PREFIX);
            break;
        default:
            break;
        }
    }
}

FS_Error storage_get_data(Storage* app, FuriString* path, StorageData** storage) {
    StorageType type = storage_get_type_by_path(path);

    if(storage_type_is_valid(type)) {
        if(type == ST_ANY) {
            type = ST_INT;
            if(storage_data_status(&app->storage[ST_EXT]) == StorageStatusOK) {
                type = ST_EXT;
            }
            storage_path_change_to_real_storage(path, type);
        }

        furi_assert(type == ST_EXT || type == ST_INT);
        *storage = &app->storage[type];

        return FSE_OK;
    } else {
        return FSE_INVALID_NAME;
    }
}

/******************* File Functions *******************/

bool storage_process_file_open(
    Storage* app,
    File* file,
    FuriString* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    bool ret = false;
    StorageData* storage;
    file->error_id = storage_get_data(app, path, &storage);

    if(file->error_id == FSE_OK) {
        if(storage_path_already_open(path, storage->files)) {
            file->error_id = FSE_ALREADY_OPEN;
        } else {
            if(access_mode & FSAM_WRITE) {
                storage_data_timestamp(storage);
            }
            storage_push_storage_file(file, path, storage);

            const char* path_cstr_no_vfs = cstr_path_without_vfs_prefix(path);
            FS_CALL(storage, file.open(storage, file, path_cstr_no_vfs, access_mode, open_mode));
        }
    }

    return ret;
}

bool storage_process_file_close(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.close(storage, file));
        storage_pop_storage_file(file, storage);

        StorageEvent event = {.type = StorageEventTypeFileClose};
        furi_pubsub_publish(app->pubsub, &event);
    }

    return ret;
}

static uint16_t
    storage_process_file_read(Storage* app, File* file, void* buff, uint16_t const bytes_to_read) {
    uint16_t ret = 0;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.read(storage, file, buff, bytes_to_read));
    }

    return ret;
}

static uint16_t storage_process_file_write(
    Storage* app,
    File* file,
    const void* buff,
    uint16_t const bytes_to_write) {
    uint16_t ret = 0;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        storage_data_timestamp(storage);
        FS_CALL(storage, file.write(storage, file, buff, bytes_to_write));
    }

    return ret;
}

static bool storage_process_file_seek(
    Storage* app,
    File* file,
    const uint32_t offset,
    const bool from_start) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.seek(storage, file, offset, from_start));
    }

    return ret;
}

static uint64_t storage_process_file_tell(Storage* app, File* file) {
    uint64_t ret = 0;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.tell(storage, file));
    }

    return ret;
}

static bool storage_process_file_truncate(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        storage_data_timestamp(storage);
        FS_CALL(storage, file.truncate(storage, file));
    }

    return ret;
}

static bool storage_process_file_sync(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        storage_data_timestamp(storage);
        FS_CALL(storage, file.sync(storage, file));
    }

    return ret;
}

static uint64_t storage_process_file_size(Storage* app, File* file) {
    uint64_t ret = 0;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.size(storage, file));
    }

    return ret;
}

static bool storage_process_file_eof(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, file.eof(storage, file));
    }

    return ret;
}

/******************* Dir Functions *******************/

bool storage_process_dir_open(Storage* app, File* file, FuriString* path) {
    bool ret = false;
    StorageData* storage;
    file->error_id = storage_get_data(app, path, &storage);

    if(file->error_id == FSE_OK) {
        if(storage_path_already_open(path, storage->files)) {
            file->error_id = FSE_ALREADY_OPEN;
        } else {
            storage_push_storage_file(file, path, storage);
            FS_CALL(storage, dir.open(storage, file, cstr_path_without_vfs_prefix(path)));
        }
    }

    return ret;
}

bool storage_process_dir_close(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, dir.close(storage, file));
        storage_pop_storage_file(file, storage);

        StorageEvent event = {.type = StorageEventTypeDirClose};
        furi_pubsub_publish(app->pubsub, &event);
    }

    return ret;
}

bool storage_process_dir_read(
    Storage* app,
    File* file,
    FileInfo* fileinfo,
    char* name,
    const uint16_t name_length) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, dir.read(storage, file, fileinfo, name, name_length));
    }

    return ret;
}

bool storage_process_dir_rewind(Storage* app, File* file) {
    bool ret = false;
    StorageData* storage = get_storage_by_file(file, app->storage);

    if(storage == NULL) {
        file->error_id = FSE_INVALID_PARAMETER;
    } else {
        FS_CALL(storage, dir.rewind(storage, file));
    }

    return ret;
}

/******************* Common FS Functions *******************/

static FS_Error
    storage_process_common_timestamp(Storage* app, FuriString* path, uint32_t* timestamp) {
    StorageData* storage;
    FS_Error ret = storage_get_data(app, path, &storage);

    if(ret == FSE_OK) {
        *timestamp = storage_data_get_timestamp(storage);
    }

    return ret;
}

static FS_Error storage_process_common_stat(Storage* app, FuriString* path, FileInfo* fileinfo) {
    StorageData* storage;
    FS_Error ret = storage_get_data(app, path, &storage);

    if(ret == FSE_OK) {
        FS_CALL(storage, common.stat(storage, cstr_path_without_vfs_prefix(path), fileinfo));
    }

    return ret;
}

static FS_Error storage_process_common_remove(Storage* app, FuriString* path) {
    StorageData* storage;
    FS_Error ret = storage_get_data(app, path, &storage);

    do {
        if(storage_path_already_open(path, storage->files)) {
            ret = FSE_ALREADY_OPEN;
            break;
        }

        storage_data_timestamp(storage);
        FS_CALL(storage, common.remove(storage, cstr_path_without_vfs_prefix(path)));
    } while(false);

    return ret;
}

static FS_Error storage_process_common_mkdir(Storage* app, FuriString* path) {
    StorageData* storage;
    FS_Error ret = storage_get_data(app, path, &storage);

    if(ret == FSE_OK) {
        storage_data_timestamp(storage);
        FS_CALL(storage, common.mkdir(storage, cstr_path_without_vfs_prefix(path)));
    }

    return ret;
}

static FS_Error storage_process_common_fs_info(
    Storage* app,
    FuriString* path,
    uint64_t* total_space,
    uint64_t* free_space) {
    StorageData* storage;
    FS_Error ret = storage_get_data(app, path, &storage);

    if(ret == FSE_OK) {
        FS_CALL(
            storage,
            common.fs_info(storage, cstr_path_without_vfs_prefix(path), total_space, free_space));
    }

    return ret;
}

/****************** Raw SD API ******************/
// TODO think about implementing a custom storage API to split that kind of api linkage
#include "storages/storage_ext.h"

static FS_Error storage_process_sd_format(Storage* app) {
    FS_Error ret = FSE_OK;

    if(storage_data_status(&app->storage[ST_EXT]) == StorageStatusNotReady) {
        ret = FSE_NOT_READY;
    } else {
        ret = sd_format_card(&app->storage[ST_EXT]);
        storage_data_timestamp(&app->storage[ST_EXT]);
    }

    return ret;
}

static FS_Error storage_process_sd_unmount(Storage* app) {
    FS_Error ret = FSE_OK;

    if(storage_data_status(&app->storage[ST_EXT]) == StorageStatusNotReady) {
        ret = FSE_NOT_READY;
    } else {
        sd_unmount_card(&app->storage[ST_EXT]);
        storage_data_timestamp(&app->storage[ST_EXT]);
    }

    return ret;
}

static FS_Error storage_process_sd_info(Storage* app, SDInfo* info) {
    FS_Error ret = FSE_OK;

    if(storage_data_status(&app->storage[ST_EXT]) == StorageStatusNotReady) {
        ret = FSE_NOT_READY;
    } else {
        ret = sd_card_info(&app->storage[ST_EXT], info);
    }

    return ret;
}

static FS_Error storage_process_sd_status(Storage* app) {
    FS_Error ret;
    StorageStatus status = storage_data_status(&app->storage[ST_EXT]);

    switch(status) {
    case StorageStatusOK:
        ret = FSE_OK;
        break;
    case StorageStatusNotReady:
        ret = FSE_NOT_READY;
        break;
    default:
        ret = FSE_INTERNAL;
        break;
    }

    return ret;
}

/******************** Aliases processing *******************/

void storage_process_alias(
    Storage* app,
    FuriString* path,
    FuriThreadId thread_id,
    bool create_folders) {
    if(furi_string_start_with(path, STORAGE_APP_DATA_PATH_PREFIX)) {
        FuriString* apps_data_path_with_appsid = furi_string_alloc_set(APPS_DATA_PATH "/");
        furi_string_cat(apps_data_path_with_appsid, furi_thread_get_appid(thread_id));

        // "/app" -> "/ext/apps_data/appsid"
        furi_string_replace_at(
            path,
            0,
            strlen(STORAGE_APP_DATA_PATH_PREFIX),
            furi_string_get_cstr(apps_data_path_with_appsid));

        // Create app data folder if not exists
        if(create_folders &&
           storage_process_common_stat(app, apps_data_path_with_appsid, NULL) != FSE_OK) {
            furi_string_set(apps_data_path_with_appsid, APPS_DATA_PATH);
            storage_process_common_mkdir(app, apps_data_path_with_appsid);
            furi_string_cat(apps_data_path_with_appsid, "/");
            furi_string_cat(apps_data_path_with_appsid, furi_thread_get_appid(thread_id));
            storage_process_common_mkdir(app, apps_data_path_with_appsid);
        }

        furi_string_free(apps_data_path_with_appsid);
    }
}

/****************** API calls processing ******************/

void storage_process_message_internal(Storage* app, StorageMessage* message) {
    FuriString* path = NULL;

    switch(message->command) {
    // File operations
    case StorageCommandFileOpen:
        path = furi_string_alloc_set(message->data->fopen.path);
        storage_process_alias(app, path, message->data->fopen.thread_id, true);
        message->return_data->bool_value = storage_process_file_open(
            app,
            message->data->fopen.file,
            path,
            message->data->fopen.access_mode,
            message->data->fopen.open_mode);
        break;
    case StorageCommandFileClose:
        message->return_data->bool_value =
            storage_process_file_close(app, message->data->fopen.file);
        break;
    case StorageCommandFileRead:
        message->return_data->uint16_value = storage_process_file_read(
            app,
            message->data->fread.file,
            message->data->fread.buff,
            message->data->fread.bytes_to_read);
        break;
    case StorageCommandFileWrite:
        message->return_data->uint16_value = storage_process_file_write(
            app,
            message->data->fwrite.file,
            message->data->fwrite.buff,
            message->data->fwrite.bytes_to_write);
        break;
    case StorageCommandFileSeek:
        message->return_data->bool_value = storage_process_file_seek(
            app,
            message->data->fseek.file,
            message->data->fseek.offset,
            message->data->fseek.from_start);
        break;
    case StorageCommandFileTell:
        message->return_data->uint64_value =
            storage_process_file_tell(app, message->data->file.file);
        break;
    case StorageCommandFileTruncate:
        message->return_data->bool_value =
            storage_process_file_truncate(app, message->data->file.file);
        break;
    case StorageCommandFileSync:
        message->return_data->bool_value =
            storage_process_file_sync(app, message->data->file.file);
        break;
    case StorageCommandFileSize:
        message->return_data->uint64_value =
            storage_process_file_size(app, message->data->file.file);
        break;
    case StorageCommandFileEof:
        message->return_data->bool_value = storage_process_file_eof(app, message->data->file.file);
        break;

    // Dir operations
    case StorageCommandDirOpen:
        path = furi_string_alloc_set(message->data->dopen.path);
        storage_process_alias(app, path, message->data->dopen.thread_id, true);
        message->return_data->bool_value =
            storage_process_dir_open(app, message->data->dopen.file, path);
        break;
    case StorageCommandDirClose:
        message->return_data->bool_value =
            storage_process_dir_close(app, message->data->file.file);
        break;
    case StorageCommandDirRead:
        message->return_data->bool_value = storage_process_dir_read(
            app,
            message->data->dread.file,
            message->data->dread.fileinfo,
            message->data->dread.name,
            message->data->dread.name_length);
        break;
    case StorageCommandDirRewind:
        message->return_data->bool_value =
            storage_process_dir_rewind(app, message->data->file.file);
        break;

    // Common operations
    case StorageCommandCommonTimestamp:
        path = furi_string_alloc_set(message->data->ctimestamp.path);
        storage_process_alias(app, path, message->data->ctimestamp.thread_id, false);
        message->return_data->error_value =
            storage_process_common_timestamp(app, path, message->data->ctimestamp.timestamp);
        break;
    case StorageCommandCommonStat:
        path = furi_string_alloc_set(message->data->cstat.path);
        storage_process_alias(app, path, message->data->cstat.thread_id, false);
        message->return_data->error_value =
            storage_process_common_stat(app, path, message->data->cstat.fileinfo);
        break;
    case StorageCommandCommonRemove:
        path = furi_string_alloc_set(message->data->path.path);
        storage_process_alias(app, path, message->data->path.thread_id, false);
        message->return_data->error_value = storage_process_common_remove(app, path);
        break;
    case StorageCommandCommonMkDir:
        path = furi_string_alloc_set(message->data->path.path);
        storage_process_alias(app, path, message->data->path.thread_id, true);
        message->return_data->error_value = storage_process_common_mkdir(app, path);
        break;
    case StorageCommandCommonFSInfo:
        path = furi_string_alloc_set(message->data->cfsinfo.fs_path);
        storage_process_alias(app, path, message->data->cfsinfo.thread_id, false);
        message->return_data->error_value = storage_process_common_fs_info(
            app, path, message->data->cfsinfo.total_space, message->data->cfsinfo.free_space);
        break;
    case StorageCommandCommonResolvePath:
        storage_process_alias(
            app, message->data->cresolvepath.path, message->data->cresolvepath.thread_id, true);
        break;

    // SD operations
    case StorageCommandSDFormat:
        message->return_data->error_value = storage_process_sd_format(app);
        break;
    case StorageCommandSDUnmount:
        message->return_data->error_value = storage_process_sd_unmount(app);
        break;
    case StorageCommandSDInfo:
        message->return_data->error_value =
            storage_process_sd_info(app, message->data->sdinfo.info);
        break;
    case StorageCommandSDStatus:
        message->return_data->error_value = storage_process_sd_status(app);
        break;
    }

    if(path != NULL) { //-V547
        furi_string_free(path);
    }

    api_lock_unlock(message->lock);
}

void storage_process_message(Storage* app, StorageMessage* message) {
    storage_process_message_internal(app, message);
}
