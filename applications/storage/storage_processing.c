#include "storage_processing.h"

#define FS_CALL(_storage, _fn)   \
    storage_data_lock(_storage); \
    ret = _storage->fs_api._fn;  \
    storage_data_unlock(_storage);

#define ST_CALL(_storage, _fn)   \
    storage_data_lock(_storage); \
    ret = _storage->api._fn;     \
    storage_data_unlock(_storage);

static StorageData* storage_get_storage_by_type(Storage* app, StorageType type) {
    StorageData* storage;

    if(type == ST_ANY) {
        type = ST_INT;
        StorageData* ext_storage = &app->storage[ST_EXT];

        if(storage_data_status(ext_storage) == StorageStatusOK) {
            type = ST_EXT;
        }
    }
    storage = &app->storage[type];

    return storage;
}

static bool storage_type_is_not_valid(StorageType type) {
    return type >= ST_ERROR;
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

const char* remove_vfs(const char* path) {
    return path + MIN(4, strlen(path));
}

/******************* File Functions *******************/

bool storage_process_file_open(
    Storage* app,
    File* file,
    const char* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    bool ret = false;
    StorageType type = storage_get_type_by_path(path);
    StorageData* storage;
    file->error_id = FSE_OK;

    if(storage_type_is_not_valid(type)) {
        file->error_id = FSE_INVALID_NAME;
    } else {
        storage = storage_get_storage_by_type(app, type);
        if(storage_path_already_open(path, storage->files)) {
            file->error_id = FSE_ALREADY_OPEN;
        } else {
            storage_push_storage_file(file, path, type, storage);
            FS_CALL(storage, file.open(storage, file, remove_vfs(path), access_mode, open_mode));
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

bool storage_process_dir_open(Storage* app, File* file, const char* path) {
    bool ret = false;
    StorageType type = storage_get_type_by_path(path);
    StorageData* storage;
    file->error_id = FSE_OK;

    if(storage_type_is_not_valid(type)) {
        file->error_id = FSE_INVALID_NAME;
    } else {
        storage = storage_get_storage_by_type(app, type);
        if(storage_path_already_open(path, storage->files)) {
            file->error_id = FSE_ALREADY_OPEN;
        } else {
            storage_push_storage_file(file, path, type, storage);
            FS_CALL(storage, dir.open(storage, file, remove_vfs(path)));
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

static FS_Error storage_process_common_stat(Storage* app, const char* path, FileInfo* fileinfo) {
    FS_Error ret = FSE_OK;
    StorageType type = storage_get_type_by_path(path);

    if(storage_type_is_not_valid(type)) {
        ret = FSE_INVALID_NAME;
    } else {
        StorageData* storage = storage_get_storage_by_type(app, type);
        FS_CALL(storage, common.stat(storage, remove_vfs(path), fileinfo));
    }

    return ret;
}

static FS_Error storage_process_common_remove(Storage* app, const char* path) {
    FS_Error ret = FSE_OK;
    StorageType type = storage_get_type_by_path(path);

    do {
        if(storage_type_is_not_valid(type)) {
            ret = FSE_INVALID_NAME;
            break;
        }

        StorageData* storage = storage_get_storage_by_type(app, type);
        if(storage_path_already_open(path, storage->files)) {
            ret = FSE_ALREADY_OPEN;
            break;
        }

        FS_CALL(storage, common.remove(storage, remove_vfs(path)));
    } while(false);

    return ret;
}

static FS_Error storage_process_common_mkdir(Storage* app, const char* path) {
    FS_Error ret = FSE_OK;
    StorageType type = storage_get_type_by_path(path);

    if(storage_type_is_not_valid(type)) {
        ret = FSE_INVALID_NAME;
    } else {
        StorageData* storage = storage_get_storage_by_type(app, type);
        FS_CALL(storage, common.mkdir(storage, remove_vfs(path)));
    }

    return ret;
}

static FS_Error storage_process_common_copy(Storage* app, const char* old, const char* new) {
    FS_Error ret = FSE_INTERNAL;
    File file_old;
    File file_new;

    FileInfo fileinfo;
    ret = storage_process_common_stat(app, old, &fileinfo);

    if(ret == FSE_OK) {
        if(fileinfo.flags & FSF_DIRECTORY) {
            ret = storage_process_common_mkdir(app, new);
        } else {
            do {
                if(!storage_process_file_open(app, &file_old, old, FSAM_READ, FSOM_OPEN_EXISTING)) {
                    ret = storage_file_get_error(&file_old);
                    storage_process_file_close(app, &file_old);
                    break;
                }

                if(!storage_process_file_open(app, &file_new, new, FSAM_WRITE, FSOM_CREATE_NEW)) {
                    ret = storage_file_get_error(&file_new);
                    storage_process_file_close(app, &file_new);
                    storage_process_file_close(app, &file_old);
                    break;
                }

                const uint16_t buffer_size = 64;
                uint8_t* buffer = malloc(buffer_size);
                uint16_t readed_size = 0;
                uint16_t writed_size = 0;

                while(true) {
                    readed_size = storage_process_file_read(app, &file_old, buffer, buffer_size);
                    ret = storage_file_get_error(&file_old);
                    if(readed_size == 0) break;

                    writed_size = storage_process_file_write(app, &file_new, buffer, readed_size);
                    ret = storage_file_get_error(&file_new);
                    if(writed_size < readed_size) break;
                }

                free(buffer);
                storage_process_file_close(app, &file_old);
                storage_process_file_close(app, &file_new);
            } while(false);
        }
    }

    return ret;
}

static FS_Error storage_process_common_rename(Storage* app, const char* old, const char* new) {
    FS_Error ret = FSE_INTERNAL;
    StorageType type_old = storage_get_type_by_path(old);
    StorageType type_new = storage_get_type_by_path(new);

    if(storage_type_is_not_valid(type_old) || storage_type_is_not_valid(type_new)) {
        ret = FSE_INVALID_NAME;
    } else {
        if(type_old != type_new) {
            ret = storage_process_common_copy(app, old, new);
            if(ret == FSE_OK) {
                ret = storage_process_common_remove(app, old);
            }
        } else {
            StorageData* storage = storage_get_storage_by_type(app, type_old);
            FS_CALL(storage, common.rename(storage, remove_vfs(old), remove_vfs(new)));
        }
    }

    return ret;
}

static FS_Error storage_process_common_fs_info(
    Storage* app,
    const char* fs_path,
    uint64_t* total_space,
    uint64_t* free_space) {
    FS_Error ret = FSE_OK;
    StorageType type = storage_get_type_by_path(fs_path);

    if(storage_type_is_not_valid(type)) {
        ret = FSE_INVALID_NAME;
    } else {
        StorageData* storage = storage_get_storage_by_type(app, type);
        FS_CALL(storage, common.fs_info(storage, remove_vfs(fs_path), total_space, free_space));
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
    }

    return ret;
}

static FS_Error storage_process_sd_unmount(Storage* app) {
    FS_Error ret = FSE_OK;

    if(storage_data_status(&app->storage[ST_EXT]) == StorageStatusNotReady) {
        ret = FSE_NOT_READY;
    } else {
        sd_unmount_card(&app->storage[ST_EXT]);
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

/****************** API calls processing ******************/

void storage_process_message(Storage* app, StorageMessage* message) {
    switch(message->command) {
    case StorageCommandFileOpen:
        message->return_data->bool_value = storage_process_file_open(
            app,
            message->data->fopen.file,
            message->data->fopen.path,
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

    case StorageCommandDirOpen:
        message->return_data->bool_value =
            storage_process_dir_open(app, message->data->dopen.file, message->data->dopen.path);
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
    case StorageCommandCommonStat:
        message->return_data->error_value = storage_process_common_stat(
            app, message->data->cstat.path, message->data->cstat.fileinfo);
        break;
    case StorageCommandCommonRemove:
        message->return_data->error_value =
            storage_process_common_remove(app, message->data->path.path);
        break;
    case StorageCommandCommonRename:
        message->return_data->error_value = storage_process_common_rename(
            app, message->data->cpaths.old, message->data->cpaths.new);
        break;
    case StorageCommandCommonCopy:
        message->return_data->error_value =
            storage_process_common_copy(app, message->data->cpaths.old, message->data->cpaths.new);
        break;
    case StorageCommandCommonMkDir:
        message->return_data->error_value =
            storage_process_common_mkdir(app, message->data->path.path);
        break;
    case StorageCommandCommonFSInfo:
        message->return_data->error_value = storage_process_common_fs_info(
            app,
            message->data->cfsinfo.fs_path,
            message->data->cfsinfo.total_space,
            message->data->cfsinfo.free_space);
        break;
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

    osSemaphoreRelease(message->semaphore);
}
