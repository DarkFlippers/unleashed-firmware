#include "fatfs.h"
#include "../filesystem-api-internal.h"
#include "storage-ext.h"
#include <furi-hal.h>
#include "sd-notify.h"
#include <furi-hal-sd.h>

typedef FIL SDFile;
typedef DIR SDDir;
typedef FILINFO SDFileInfo;
typedef FRESULT SDError;

#define TAG "StorageExt"
#define STORAGE_PATH "/ext"
/********************* Definitions ********************/

typedef struct {
    FATFS* fs;
    const char* path;
    bool sd_was_present;
} SDData;

static FS_Error storage_ext_parse_error(SDError error);

/******************* Core Functions *******************/

static bool sd_mount_card(StorageData* storage, bool notify) {
    bool result = false;
    const uint8_t max_init_counts = 10;
    uint8_t counter = max_init_counts;
    uint8_t bsp_result;
    SDData* sd_data = storage->data;

    storage_data_lock(storage);

    while(result == false && counter > 0 && hal_sd_detect()) {
        if(notify) {
            NotificationApp* notification = furi_record_open("notification");
            sd_notify_wait(notification);
            furi_record_close("notification");
        }

        if((counter % 2) == 0) {
            // power reset sd card
            bsp_result = BSP_SD_Init(true);
        } else {
            bsp_result = BSP_SD_Init(false);
        }

        if(bsp_result) {
            // bsp error
            storage->status = StorageStatusErrorInternal;
        } else {
            SDError status = f_mount(sd_data->fs, sd_data->path, 1);

            if(status == FR_OK || status == FR_NO_FILESYSTEM) {
                FATFS* fs;
                uint32_t free_clusters;

                status = f_getfree(sd_data->path, &free_clusters, &fs);

                if(status == FR_OK || status == FR_NO_FILESYSTEM) {
                    result = true;
                }

                if(status == FR_OK) {
                    storage->status = StorageStatusOK;
                } else if(status == FR_NO_FILESYSTEM) {
                    storage->status = StorageStatusNoFS;
                } else {
                    storage->status = StorageStatusNotAccessible;
                }
            } else {
                storage->status = StorageStatusNotMounted;
            }
        }

        if(notify) {
            NotificationApp* notification = furi_record_open("notification");
            sd_notify_wait_off(notification);
            furi_record_close("notification");
        }

        if(!result) {
            delay(1000);
            FURI_LOG_E(
                TAG, "init cycle %d, error: %s", counter, storage_data_status_text(storage));
            counter--;
        }
    }

    storage_data_unlock(storage);

    return result;
}

FS_Error sd_unmount_card(StorageData* storage) {
    SDData* sd_data = storage->data;
    SDError error;

    storage_data_lock(storage);
    storage->status = StorageStatusNotReady;
    error = FR_DISK_ERR;

    // TODO do i need to close the files?

    f_mount(0, sd_data->path, 0);
    storage_data_unlock(storage);
    return storage_ext_parse_error(error);
}

FS_Error sd_format_card(StorageData* storage) {
    uint8_t* work_area;
    SDData* sd_data = storage->data;
    SDError error;

    storage_data_lock(storage);

    work_area = malloc(_MAX_SS);
    error = f_mkfs(sd_data->path, FM_ANY, 0, work_area, _MAX_SS);
    free(work_area);

    do {
        storage->status = StorageStatusNotAccessible;
        if(error != FR_OK) break;
        storage->status = StorageStatusNoFS;
        error = f_setlabel("Flipper SD");
        if(error != FR_OK) break;
        storage->status = StorageStatusNotMounted;
        error = f_mount(sd_data->fs, sd_data->path, 1);
        if(error != FR_OK) break;
        storage->status = StorageStatusOK;
    } while(false);

    storage_data_unlock(storage);

    return storage_ext_parse_error(error);
}

FS_Error sd_card_info(StorageData* storage, SDInfo* sd_info) {
    uint32_t free_clusters, free_sectors, total_sectors;
    FATFS* fs;
    SDData* sd_data = storage->data;
    SDError error;

    // clean data
    memset(sd_info, 0, sizeof(SDInfo));

    // get fs info
    storage_data_lock(storage);
    error = f_getlabel(sd_data->path, sd_info->label, NULL);
    if(error == FR_OK) {
        error = f_getfree(sd_data->path, &free_clusters, &fs);
    }
    storage_data_unlock(storage);

    if(error == FR_OK) {
        // calculate size
        total_sectors = (fs->n_fatent - 2) * fs->csize;
        free_sectors = free_clusters * fs->csize;

        uint16_t sector_size = _MAX_SS;
#if _MAX_SS != _MIN_SS
        sector_size = fs->ssize;
#endif

        sd_info->fs_type = fs->fs_type;

        sd_info->kb_total = total_sectors / 1024 * sector_size;
        sd_info->kb_free = free_sectors / 1024 * sector_size;
        sd_info->cluster_size = fs->csize;
        sd_info->sector_size = sector_size;
    }

    return storage_ext_parse_error(error);
}

static void storage_ext_tick_internal(StorageData* storage, bool notify) {
    SDData* sd_data = storage->data;

    if(sd_data->sd_was_present) {
        if(hal_sd_detect()) {
            FURI_LOG_I(TAG, "card detected");
            sd_mount_card(storage, notify);

            if(storage->status != StorageStatusOK) {
                FURI_LOG_E(TAG, "sd init error: %s", storage_data_status_text(storage));
                if(notify) {
                    NotificationApp* notification = furi_record_open("notification");
                    sd_notify_error(notification);
                    furi_record_close("notification");
                }
            } else {
                FURI_LOG_I(TAG, "card mounted");
                if(notify) {
                    NotificationApp* notification = furi_record_open("notification");
                    sd_notify_success(notification);
                    furi_record_close("notification");
                }
            }

            sd_data->sd_was_present = false;

            if(!hal_sd_detect()) {
                FURI_LOG_I(TAG, "card removed while mounting");
                sd_unmount_card(storage);
                sd_data->sd_was_present = true;
            }
        }
    } else {
        if(!hal_sd_detect()) {
            FURI_LOG_I(TAG, "card removed");
            sd_data->sd_was_present = true;

            sd_unmount_card(storage);
            if(notify) {
                NotificationApp* notification = furi_record_open("notification");
                sd_notify_eject(notification);
                furi_record_close("notification");
            }
        }
    }
}

static void storage_ext_tick(StorageData* storage) {
    storage_ext_tick_internal(storage, true);
}

/****************** Common Functions ******************/

static FS_Error storage_ext_parse_error(SDError error) {
    FS_Error result;
    switch(error) {
    case FR_OK:
        result = FSE_OK;
        break;
    case FR_NOT_READY:
        result = FSE_NOT_READY;
        break;
    case FR_NO_FILE:
    case FR_NO_PATH:
    case FR_NO_FILESYSTEM:
        result = FSE_NOT_EXIST;
        break;
    case FR_EXIST:
        result = FSE_EXIST;
        break;
    case FR_INVALID_NAME:
        result = FSE_INVALID_NAME;
        break;
    case FR_INVALID_OBJECT:
    case FR_INVALID_PARAMETER:
        result = FSE_INVALID_PARAMETER;
        break;
    case FR_DENIED:
        result = FSE_DENIED;
        break;
    default:
        result = FSE_INTERNAL;
        break;
    }

    return result;
}

/******************* File Functions *******************/

static bool storage_ext_file_open(
    void* ctx,
    File* file,
    const char* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    StorageData* storage = ctx;
    uint8_t _mode = 0;

    if(access_mode & FSAM_READ) _mode |= FA_READ;
    if(access_mode & FSAM_WRITE) _mode |= FA_WRITE;
    if(open_mode & FSOM_OPEN_EXISTING) _mode |= FA_OPEN_EXISTING;
    if(open_mode & FSOM_OPEN_ALWAYS) _mode |= FA_OPEN_ALWAYS;
    if(open_mode & FSOM_OPEN_APPEND) _mode |= FA_OPEN_APPEND;
    if(open_mode & FSOM_CREATE_NEW) _mode |= FA_CREATE_NEW;
    if(open_mode & FSOM_CREATE_ALWAYS) _mode |= FA_CREATE_ALWAYS;

    SDFile* file_data = malloc(sizeof(SDFile));
    storage_set_storage_file_data(file, file_data, storage);

    file->internal_error_id = f_open(file_data, path, _mode);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
}

static bool storage_ext_file_close(void* ctx, File* file) {
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);
    file->internal_error_id = f_close(file_data);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    free(file_data);
    return (file->error_id == FSE_OK);
}

static uint16_t
    storage_ext_file_read(void* ctx, File* file, void* buff, uint16_t const bytes_to_read) {
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);
    uint16_t bytes_readed = 0;
    file->internal_error_id = f_read(file_data, buff, bytes_to_read, &bytes_readed);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return bytes_readed;
}

static uint16_t
    storage_ext_file_write(void* ctx, File* file, const void* buff, uint16_t const bytes_to_write) {
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);
    uint16_t bytes_written = 0;
    file->internal_error_id = f_write(file_data, buff, bytes_to_write, &bytes_written);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return bytes_written;
}

static bool
    storage_ext_file_seek(void* ctx, File* file, const uint32_t offset, const bool from_start) {
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);

    if(from_start) {
        file->internal_error_id = f_lseek(file_data, offset);
    } else {
        uint64_t position = f_tell(file_data);
        position += offset;
        file->internal_error_id = f_lseek(file_data, position);
    }

    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
}

static uint64_t storage_ext_file_tell(void* ctx, File* file) {
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);

    uint64_t position = 0;
    position = f_tell(file_data);
    file->error_id = FSE_OK;
    return position;
}

static bool storage_ext_file_truncate(void* ctx, File* file) {
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);

    file->internal_error_id = f_truncate(file_data);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
}

static bool storage_ext_file_sync(void* ctx, File* file) {
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);

    file->internal_error_id = f_sync(file_data);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
}

static uint64_t storage_ext_file_size(void* ctx, File* file) {
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);

    uint64_t size = 0;
    size = f_size(file_data);
    file->error_id = FSE_OK;
    return size;
}

static bool storage_ext_file_eof(void* ctx, File* file) {
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);

    bool eof = f_eof(file_data);
    file->internal_error_id = 0;
    file->error_id = FSE_OK;
    return eof;
}

/******************* Dir Functions *******************/

static bool storage_ext_dir_open(void* ctx, File* file, const char* path) {
    StorageData* storage = ctx;

    SDDir* file_data = malloc(sizeof(SDDir));
    storage_set_storage_file_data(file, file_data, storage);
    file->internal_error_id = f_opendir(file_data, path);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
}

static bool storage_ext_dir_close(void* ctx, File* file) {
    StorageData* storage = ctx;
    SDDir* file_data = storage_get_storage_file_data(file, storage);

    file->internal_error_id = f_closedir(file_data);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    free(file_data);
    return (file->error_id == FSE_OK);
}

static bool storage_ext_dir_read(
    void* ctx,
    File* file,
    FileInfo* fileinfo,
    char* name,
    const uint16_t name_length) {
    StorageData* storage = ctx;
    SDDir* file_data = storage_get_storage_file_data(file, storage);

    SDFileInfo _fileinfo;
    file->internal_error_id = f_readdir(file_data, &_fileinfo);
    file->error_id = storage_ext_parse_error(file->internal_error_id);

    if(fileinfo != NULL) {
        fileinfo->size = _fileinfo.fsize;
        fileinfo->flags = 0;

        if(_fileinfo.fattrib & AM_DIR) fileinfo->flags |= FSF_DIRECTORY;
    }

    if(name != NULL) {
        snprintf(name, name_length, "%s", _fileinfo.fname);
    }

    if(_fileinfo.fname[0] == 0) {
        file->error_id = FSE_NOT_EXIST;
    }

    return (file->error_id == FSE_OK);
}

static bool storage_ext_dir_rewind(void* ctx, File* file) {
    StorageData* storage = ctx;
    SDDir* file_data = storage_get_storage_file_data(file, storage);

    file->internal_error_id = f_readdir(file_data, NULL);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
}
/******************* Common FS Functions *******************/

static FS_Error storage_ext_common_stat(void* ctx, const char* path, FileInfo* fileinfo) {
    SDFileInfo _fileinfo;
    SDError result = f_stat(path, &_fileinfo);

    if(fileinfo != NULL) {
        fileinfo->size = _fileinfo.fsize;
        fileinfo->flags = 0;

        if(_fileinfo.fattrib & AM_DIR) fileinfo->flags |= FSF_DIRECTORY;
    }

    return storage_ext_parse_error(result);
}

static FS_Error storage_ext_common_remove(void* ctx, const char* path) {
    SDError result = f_unlink(path);
    return storage_ext_parse_error(result);
}

static FS_Error storage_ext_common_rename(void* ctx, const char* old_path, const char* new_path) {
    SDError result = f_rename(old_path, new_path);
    return storage_ext_parse_error(result);
}

static FS_Error storage_ext_common_mkdir(void* ctx, const char* path) {
    SDError result = f_mkdir(path);
    return storage_ext_parse_error(result);
}

static FS_Error storage_ext_common_fs_info(
    void* ctx,
    const char* fs_path,
    uint64_t* total_space,
    uint64_t* free_space) {
    StorageData* storage = ctx;
    SDData* sd_data = storage->data;

    DWORD free_clusters;
    FATFS* fs;

    SDError fresult = f_getfree(sd_data->path, &free_clusters, &fs);
    if((FRESULT)fresult == FR_OK) {
        uint32_t total_sectors = (fs->n_fatent - 2) * fs->csize;
        uint32_t free_sectors = free_clusters * fs->csize;

        uint16_t sector_size = _MAX_SS;
#if _MAX_SS != _MIN_SS
        sector_size = fs->ssize;
#endif

        if(total_space != NULL) {
            *total_space = (uint64_t)total_sectors * (uint64_t)sector_size;
        }

        if(free_space != NULL) {
            *free_space = (uint64_t)free_sectors * (uint64_t)sector_size;
        }
    }

    return storage_ext_parse_error(fresult);
}

/******************* Init Storage *******************/

void storage_ext_init(StorageData* storage) {
    SDData* sd_data = malloc(sizeof(SDData));
    sd_data->fs = &USERFatFS;
    sd_data->path = "0:/";
    sd_data->sd_was_present = true;

    storage->data = sd_data;
    storage->api.tick = storage_ext_tick;
    storage->fs_api.file.open = storage_ext_file_open;
    storage->fs_api.file.close = storage_ext_file_close;
    storage->fs_api.file.read = storage_ext_file_read;
    storage->fs_api.file.write = storage_ext_file_write;
    storage->fs_api.file.seek = storage_ext_file_seek;
    storage->fs_api.file.tell = storage_ext_file_tell;
    storage->fs_api.file.truncate = storage_ext_file_truncate;
    storage->fs_api.file.size = storage_ext_file_size;
    storage->fs_api.file.sync = storage_ext_file_sync;
    storage->fs_api.file.eof = storage_ext_file_eof;

    storage->fs_api.dir.open = storage_ext_dir_open;
    storage->fs_api.dir.close = storage_ext_dir_close;
    storage->fs_api.dir.read = storage_ext_dir_read;
    storage->fs_api.dir.rewind = storage_ext_dir_rewind;

    storage->fs_api.common.stat = storage_ext_common_stat;
    storage->fs_api.common.mkdir = storage_ext_common_mkdir;
    storage->fs_api.common.rename = storage_ext_common_rename;
    storage->fs_api.common.remove = storage_ext_common_remove;
    storage->fs_api.common.fs_info = storage_ext_common_fs_info;

    hal_sd_detect_init();

    // do not notify on first launch, notifications app is waiting for our thread to read settings
    storage_ext_tick_internal(storage, false);
}
