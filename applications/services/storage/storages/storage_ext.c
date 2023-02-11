#include "fatfs.h"
#include "../filesystem_api_internal.h"
#include "storage_ext.h"
#include <furi_hal.h>
#include "sd_notify.h"
#include <furi_hal_sd.h>

typedef FIL SDFile;
typedef DIR SDDir;
typedef FILINFO SDFileInfo;
typedef FRESULT SDError;

#define TAG "StorageExt"

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
    uint8_t counter = sd_max_mount_retry_count();
    uint8_t bsp_result;
    SDData* sd_data = storage->data;

    while(result == false && counter > 0 && hal_sd_detect()) {
        if(notify) {
            NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
            sd_notify_wait(notification);
            furi_record_close(RECORD_NOTIFICATION);
        }

        if((counter % 2) == 0) {
            // power reset sd card
            bsp_result = sd_init(true);
        } else {
            bsp_result = sd_init(false);
        }

        if(bsp_result) {
            // bsp error
            storage->status = StorageStatusErrorInternal;
        } else {
            SDError status = f_mount(sd_data->fs, sd_data->path, 1);

            if(status == FR_OK || status == FR_NO_FILESYSTEM) {
#ifndef FURI_RAM_EXEC
                FATFS* fs;
                uint32_t free_clusters;

                status = f_getfree(sd_data->path, &free_clusters, &fs);
#endif

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
            NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
            sd_notify_wait_off(notification);
            furi_record_close(RECORD_NOTIFICATION);
        }

        if(!result) {
            furi_delay_ms(1000);
            FURI_LOG_E(
                TAG, "init cycle %d, error: %s", counter, storage_data_status_text(storage));
            counter--;
        }
    }

    storage_data_timestamp(storage);

    return result;
}

FS_Error sd_unmount_card(StorageData* storage) {
    SDData* sd_data = storage->data;
    SDError error;

    storage->status = StorageStatusNotReady;
    error = FR_DISK_ERR;

    // TODO do i need to close the files?
    f_mount(0, sd_data->path, 0);

    return storage_ext_parse_error(error);
}

FS_Error sd_format_card(StorageData* storage) {
#ifdef FURI_RAM_EXEC
    UNUSED(storage);
    return FSE_NOT_READY;
#else
    uint8_t* work_area;
    SDData* sd_data = storage->data;
    SDError error;

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

    return storage_ext_parse_error(error);
#endif
}

FS_Error sd_card_info(StorageData* storage, SDInfo* sd_info) {
#ifndef FURI_RAM_EXEC
    uint32_t free_clusters, free_sectors, total_sectors;
    FATFS* fs;
#endif
    SDData* sd_data = storage->data;
    SDError error;

    // clean data
    memset(sd_info, 0, sizeof(SDInfo));

    // get fs info
    error = f_getlabel(sd_data->path, sd_info->label, NULL);
    if(error == FR_OK) {
#ifndef FURI_RAM_EXEC
        error = f_getfree(sd_data->path, &free_clusters, &fs);
#endif
    }

    if(error == FR_OK) {
        // calculate size
#ifndef FURI_RAM_EXEC
        total_sectors = (fs->n_fatent - 2) * fs->csize;
        free_sectors = free_clusters * fs->csize;
#endif

        uint16_t sector_size = _MAX_SS;
#if _MAX_SS != _MIN_SS
        sector_size = fs->ssize;
#endif

#ifdef FURI_RAM_EXEC
        sd_info->fs_type = 0;
        sd_info->kb_total = 0;
        sd_info->kb_free = 0;
        sd_info->cluster_size = 512;
        sd_info->sector_size = sector_size;
#else
        sd_info->fs_type = fs->fs_type;
        switch(fs->fs_type) {
        case FS_FAT12:
            sd_info->fs_type = FST_FAT12;
            break;
        case FS_FAT16:
            sd_info->fs_type = FST_FAT16;
            break;
        case FS_FAT32:
            sd_info->fs_type = FST_FAT32;
            break;
        case FS_EXFAT:
            sd_info->fs_type = FST_EXFAT;
            break;
        default:
            sd_info->fs_type = FST_UNKNOWN;
            break;
        }

        sd_info->kb_total = total_sectors / 1024 * sector_size;
        sd_info->kb_free = free_sectors / 1024 * sector_size;
        sd_info->cluster_size = fs->csize;
        sd_info->sector_size = sector_size;
#endif
    }

    SD_CID cid;
    SdSpiStatus status = sd_get_cid(&cid);

    if(status == SdSpiStatusOK) {
        sd_info->manufacturer_id = cid.ManufacturerID;
        memcpy(sd_info->oem_id, cid.OEM_AppliID, sizeof(cid.OEM_AppliID));
        memcpy(sd_info->product_name, cid.ProdName, sizeof(cid.ProdName));
        sd_info->product_revision_major = cid.ProdRev >> 4;
        sd_info->product_revision_minor = cid.ProdRev & 0x0F;
        sd_info->product_serial_number = cid.ProdSN;
        sd_info->manufacturing_year = 2000 + cid.ManufactYear;
        sd_info->manufacturing_month = cid.ManufactMonth;
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
                    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
                    sd_notify_error(notification);
                    furi_record_close(RECORD_NOTIFICATION);
                }
            } else {
                FURI_LOG_I(TAG, "card mounted");
                if(notify) {
                    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
                    sd_notify_success(notification);
                    furi_record_close(RECORD_NOTIFICATION);
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
                NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
                sd_notify_eject(notification);
                furi_record_close(RECORD_NOTIFICATION);
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
    uint16_t bytes_read = 0;
    file->internal_error_id = f_read(file_data, buff, bytes_to_read, &bytes_read);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return bytes_read;
}

static uint16_t
    storage_ext_file_write(void* ctx, File* file, const void* buff, uint16_t const bytes_to_write) {
#ifdef FURI_RAM_EXEC
    UNUSED(ctx);
    UNUSED(file);
    UNUSED(buff);
    UNUSED(bytes_to_write);
    return FSE_NOT_READY;
#else
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);
    uint16_t bytes_written = 0;
    file->internal_error_id = f_write(file_data, buff, bytes_to_write, &bytes_written);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return bytes_written;
#endif
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
#ifdef FURI_RAM_EXEC
    UNUSED(ctx);
    UNUSED(file);
    return FSE_NOT_READY;
#else
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);

    file->internal_error_id = f_truncate(file_data);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
#endif
}

static bool storage_ext_file_sync(void* ctx, File* file) {
#ifdef FURI_RAM_EXEC
    UNUSED(ctx);
    UNUSED(file);
    return FSE_NOT_READY;
#else
    StorageData* storage = ctx;
    SDFile* file_data = storage_get_storage_file_data(file, storage);

    file->internal_error_id = f_sync(file_data);
    file->error_id = storage_ext_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
#endif
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
    UNUSED(ctx);
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
    UNUSED(ctx);
#ifdef FURI_RAM_EXEC
    UNUSED(path);
    return FSE_NOT_READY;
#else
    SDError result = f_unlink(path);
    return storage_ext_parse_error(result);
#endif
}

static FS_Error storage_ext_common_mkdir(void* ctx, const char* path) {
    UNUSED(ctx);
#ifdef FURI_RAM_EXEC
    UNUSED(path);
    return FSE_NOT_READY;
#else
    SDError result = f_mkdir(path);
    return storage_ext_parse_error(result);
#endif
}

static FS_Error storage_ext_common_fs_info(
    void* ctx,
    const char* fs_path,
    uint64_t* total_space,
    uint64_t* free_space) {
    UNUSED(fs_path);
#ifdef FURI_RAM_EXEC
    UNUSED(ctx);
    UNUSED(total_space);
    UNUSED(free_space);
    return FSE_NOT_READY;
#else
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
#endif
}

/******************* Init Storage *******************/
static const FS_Api fs_api = {
    .file =
        {
            .open = storage_ext_file_open,
            .close = storage_ext_file_close,
            .read = storage_ext_file_read,
            .write = storage_ext_file_write,
            .seek = storage_ext_file_seek,
            .tell = storage_ext_file_tell,
            .truncate = storage_ext_file_truncate,
            .size = storage_ext_file_size,
            .sync = storage_ext_file_sync,
            .eof = storage_ext_file_eof,
        },
    .dir =
        {
            .open = storage_ext_dir_open,
            .close = storage_ext_dir_close,
            .read = storage_ext_dir_read,
            .rewind = storage_ext_dir_rewind,
        },
    .common =
        {
            .stat = storage_ext_common_stat,
            .mkdir = storage_ext_common_mkdir,
            .remove = storage_ext_common_remove,
            .fs_info = storage_ext_common_fs_info,
        },
};

void storage_ext_init(StorageData* storage) {
    SDData* sd_data = malloc(sizeof(SDData));
    sd_data->fs = &USERFatFS;
    sd_data->path = "0:/";
    sd_data->sd_was_present = true;

    storage->data = sd_data;
    storage->api.tick = storage_ext_tick;
    storage->fs_api = &fs_api;

    hal_sd_detect_init();

    // do not notify on first launch, notifications app is waiting for our thread to read settings
    storage_ext_tick_internal(storage, false);
}
