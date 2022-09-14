#include "storage_processing.h"
#include <m-list.h>
#include <m-dict.h>
#include <m-string.h>

#define FS_CALL(_storage, _fn)   \
    storage_data_lock(_storage); \
    ret = _storage->fs_api->_fn; \
    storage_data_unlock(_storage);

#define ST_CALL(_storage, _fn)   \
    storage_data_lock(_storage); \
    ret = _storage->api._fn;     \
    storage_data_unlock(_storage);

static StorageData* storage_get_storage_by_type(Storage* app, StorageType type) {
    furi_check(type == ST_EXT || type == ST_INT);
    StorageData* storage = &app->storage[type];
    return storage;
}

static bool storage_type_is_not_valid(StorageType type) {
#ifdef FURI_RAM_EXEC
    return type != ST_EXT;
#else
    return type >= ST_ERROR;
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

static const char* remove_vfs(const char* path) {
    return path + MIN(4u, strlen(path));
}

static StorageType storage_get_type_by_path(Storage* app, const char* path) {
    StorageType type = ST_ERROR;
    if(strlen(path) >= strlen(STORAGE_EXT_PATH_PREFIX) &&
       memcmp(path, STORAGE_EXT_PATH_PREFIX, strlen(STORAGE_EXT_PATH_PREFIX)) == 0) {
        type = ST_EXT;
    } else if(
        strlen(path) >= strlen(STORAGE_INT_PATH_PREFIX) &&
        memcmp(path, STORAGE_INT_PATH_PREFIX, strlen(STORAGE_INT_PATH_PREFIX)) == 0) {
        type = ST_INT;
    } else if(
        strlen(path) >= strlen(STORAGE_ANY_PATH_PREFIX) &&
        memcmp(path, STORAGE_ANY_PATH_PREFIX, strlen(STORAGE_ANY_PATH_PREFIX)) == 0) {
        type = ST_ANY;
    }

    if(type == ST_ANY) {
        type = ST_INT;
        if(storage_data_status(&app->storage[ST_EXT]) == StorageStatusOK) {
            type = ST_EXT;
        }
    }

    return type;
}

static void storage_path_change_to_real_storage(string_t path, StorageType real_storage) {
    if(memcmp(string_get_cstr(path), STORAGE_ANY_PATH_PREFIX, strlen(STORAGE_ANY_PATH_PREFIX)) ==
       0) {
        switch(real_storage) {
        case ST_EXT:
            string_set_char(path, 0, STORAGE_EXT_PATH_PREFIX[0]);
            string_set_char(path, 1, STORAGE_EXT_PATH_PREFIX[1]);
            string_set_char(path, 2, STORAGE_EXT_PATH_PREFIX[2]);
            string_set_char(path, 3, STORAGE_EXT_PATH_PREFIX[3]);
            break;
        case ST_INT:
            string_set_char(path, 0, STORAGE_INT_PATH_PREFIX[0]);
            string_set_char(path, 1, STORAGE_INT_PATH_PREFIX[1]);
            string_set_char(path, 2, STORAGE_INT_PATH_PREFIX[2]);
            string_set_char(path, 3, STORAGE_INT_PATH_PREFIX[3]);
            break;
        default:
            break;
        }
    }
}

/******************* File Functions *******************/

bool storage_process_file_open(
    Storage* app,
    File* file,
    const char* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    bool ret = false;
    StorageType type = storage_get_type_by_path(app, path);
    StorageData* storage;
    file->error_id = FSE_OK;

    if(storage_type_is_not_valid(type)) {
        file->error_id = FSE_INVALID_NAME;
    } else {
        storage = storage_get_storage_by_type(app, type);
        string_t real_path;
        string_init_set(real_path, path);
        storage_path_change_to_real_storage(real_path, type);

        if(storage_path_already_open(real_path, storage->files)) {
            file->error_id = FSE_ALREADY_OPEN;
        } else {
            storage_push_storage_file(file, real_path, type, storage);
            FS_CALL(storage, file.open(storage, file, remove_vfs(path), access_mode, open_mode));
        }

        string_clear(real_path);
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
    StorageType type = storage_get_type_by_path(app, path);
    StorageData* storage;
    file->error_id = FSE_OK;

    if(storage_type_is_not_valid(type)) {
        file->error_id = FSE_INVALID_NAME;
    } else {
        storage = storage_get_storage_by_type(app, type);
        string_t real_path;
        string_init_set(real_path, path);
        storage_path_change_to_real_storage(real_path, type);

        if(storage_path_already_open(real_path, storage->files)) {
            file->error_id = FSE_ALREADY_OPEN;
        } else {
            storage_push_storage_file(file, real_path, type, storage);
            FS_CALL(storage, dir.open(storage, file, remove_vfs(path)));
        }
        string_clear(real_path);
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

static FS_Error storage_process_common_stat(Storage* app, const char* path, FileInfo* fileinfo) {
    FS_Error ret = FSE_OK;
    StorageType type = storage_get_type_by_path(app, path);

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
    StorageType type = storage_get_type_by_path(app, path);

    string_t real_path;
    string_init_set(real_path, path);
    storage_path_change_to_real_storage(real_path, type);

    do {
        if(storage_type_is_not_valid(type)) {
            ret = FSE_INVALID_NAME;
            break;
        }

        StorageData* storage = storage_get_storage_by_type(app, type);
        if(storage_path_already_open(real_path, storage->files)) {
            ret = FSE_ALREADY_OPEN;
            break;
        }

        FS_CALL(storage, common.remove(storage, remove_vfs(path)));
    } while(false);

    string_clear(real_path);

    return ret;
}

static FS_Error storage_process_common_mkdir(Storage* app, const char* path) {
    FS_Error ret = FSE_OK;
    StorageType type = storage_get_type_by_path(app, path);

    if(storage_type_is_not_valid(type)) {
        ret = FSE_INVALID_NAME;
    } else {
        StorageData* storage = storage_get_storage_by_type(app, type);
        FS_CALL(storage, common.mkdir(storage, remove_vfs(path)));
    }

    return ret;
}

static FS_Error storage_process_common_fs_info(
    Storage* app,
    const char* fs_path,
    uint64_t* total_space,
    uint64_t* free_space) {
    FS_Error ret = FSE_OK;
    StorageType type = storage_get_type_by_path(app, fs_path);

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
void storage_process_message_internal(Storage* app, StorageMessage* message) {
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

    furi_semaphore_release(message->semaphore);
}

void storage_process_message(Storage* app, StorageMessage* message) {
    storage_process_message_internal(app, message);
}
