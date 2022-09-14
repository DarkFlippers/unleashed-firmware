#include "storage_int.h"
#include <lfs.h>
#include <furi_hal.h>
#include <toolbox/path.h>

#define TAG "StorageInt"
#define STORAGE_PATH STORAGE_INT_PATH_PREFIX
#define LFS_CLEAN_FINGERPRINT 0

/* When less than LFS_RESERVED_PAGES_COUNT are left free, creation & 
 * modification of non-dot files is restricted */
#define LFS_RESERVED_PAGES_COUNT 3

typedef struct {
    const size_t start_address;
    const size_t start_page;
    struct lfs_config config;
    lfs_t lfs;
} LFSData;

typedef struct {
    void* data;
    bool open;
} LFSHandle;

static LFSHandle* lfs_handle_alloc_file() {
    LFSHandle* handle = malloc(sizeof(LFSHandle));
    handle->data = malloc(sizeof(lfs_file_t));
    return handle;
}

static LFSHandle* lfs_handle_alloc_dir() {
    LFSHandle* handle = malloc(sizeof(LFSHandle));
    handle->data = malloc(sizeof(lfs_dir_t));
    return handle;
}

/* INTERNALS */

static lfs_dir_t* lfs_handle_get_dir(LFSHandle* handle) {
    return handle->data;
}

static lfs_file_t* lfs_handle_get_file(LFSHandle* handle) {
    return handle->data;
}

static void lfs_handle_free(LFSHandle* handle) {
    free(handle->data);
    free(handle);
}

static void lfs_handle_set_open(LFSHandle* handle) {
    handle->open = true;
}

static bool lfs_handle_is_open(LFSHandle* handle) {
    return handle->open;
}

static lfs_t* lfs_get_from_storage(StorageData* storage) {
    return &((LFSData*)storage->data)->lfs;
}

static LFSData* lfs_data_get_from_storage(StorageData* storage) {
    return (LFSData*)storage->data;
}

static int storage_int_device_read(
    const struct lfs_config* c,
    lfs_block_t block,
    lfs_off_t off,
    void* buffer,
    lfs_size_t size) {
    LFSData* lfs_data = c->context;
    size_t address = lfs_data->start_address + block * c->block_size + off;

    FURI_LOG_T(
        TAG,
        "Device read: block %d, off %d, buffer: %p, size %d, translated address: %p",
        block,
        off,
        buffer,
        size,
        address);

    memcpy(buffer, (void*)address, size);

    return 0;
}

static int storage_int_device_prog(
    const struct lfs_config* c,
    lfs_block_t block,
    lfs_off_t off,
    const void* buffer,
    lfs_size_t size) {
    LFSData* lfs_data = c->context;
    size_t address = lfs_data->start_address + block * c->block_size + off;

    FURI_LOG_T(
        TAG,
        "Device prog: block %d, off %d, buffer: %p, size %d, translated address: %p",
        block,
        off,
        buffer,
        size,
        address);

    int ret = 0;
    while(size > 0) {
        if(!furi_hal_flash_write_dword(address, *(uint64_t*)buffer)) {
            ret = -1;
            break;
        }
        address += c->prog_size;
        buffer += c->prog_size;
        size -= c->prog_size;
    }

    return ret;
}

static int storage_int_device_erase(const struct lfs_config* c, lfs_block_t block) {
    LFSData* lfs_data = c->context;
    size_t page = lfs_data->start_page + block;

    FURI_LOG_D(TAG, "Device erase: page %d, translated page: %x", block, page);

    if(furi_hal_flash_erase(page)) {
        return 0;
    } else {
        return -1;
    }
}

static int storage_int_device_sync(const struct lfs_config* c) {
    UNUSED(c);
    FURI_LOG_D(TAG, "Device sync: skipping, cause ");
    return 0;
}

static LFSData* storage_int_lfs_data_alloc() {
    LFSData* lfs_data = malloc(sizeof(LFSData));

    // Internal storage start address
    *(size_t*)(&lfs_data->start_address) = furi_hal_flash_get_free_page_start_address();
    *(size_t*)(&lfs_data->start_page) =
        (lfs_data->start_address - furi_hal_flash_get_base()) / furi_hal_flash_get_page_size();

    // LFS configuration
    // Glue and context
    lfs_data->config.context = lfs_data;
    lfs_data->config.read = storage_int_device_read;
    lfs_data->config.prog = storage_int_device_prog;
    lfs_data->config.erase = storage_int_device_erase;
    lfs_data->config.sync = storage_int_device_sync;

    // Block device description
    lfs_data->config.read_size = furi_hal_flash_get_read_block_size();
    lfs_data->config.prog_size = furi_hal_flash_get_write_block_size();
    lfs_data->config.block_size = furi_hal_flash_get_page_size();
    lfs_data->config.block_count = furi_hal_flash_get_free_page_count();
    lfs_data->config.block_cycles = furi_hal_flash_get_cycles_count();
    lfs_data->config.cache_size = 16;
    lfs_data->config.lookahead_size = 16;

    return lfs_data;
};

// Returns true if fingerprint was invalid and LFS reformatting is needed
static bool storage_int_check_and_set_fingerprint(LFSData* lfs_data) {
    bool value = false;

    uint32_t os_fingerprint = 0;
    os_fingerprint |= ((lfs_data->start_page & 0xFF) << 0);
    os_fingerprint |= ((lfs_data->config.block_count & 0xFF) << 8);
    os_fingerprint |= ((LFS_DISK_VERSION_MAJOR & 0xFFFF) << 16);

    uint32_t rtc_fingerprint = furi_hal_rtc_get_register(FuriHalRtcRegisterLfsFingerprint);
    if(rtc_fingerprint == LFS_CLEAN_FINGERPRINT) {
        FURI_LOG_I(TAG, "Storing LFS fingerprint in RTC");
        furi_hal_rtc_set_register(FuriHalRtcRegisterLfsFingerprint, os_fingerprint);
    } else if(rtc_fingerprint != os_fingerprint) {
        FURI_LOG_E(TAG, "LFS fingerprint mismatch");
        furi_hal_rtc_set_register(FuriHalRtcRegisterLfsFingerprint, os_fingerprint);
        value = true;
    }

    return value;
}

static void storage_int_lfs_mount(LFSData* lfs_data, StorageData* storage) {
    int err;
    lfs_t* lfs = &lfs_data->lfs;

    bool was_fingerprint_outdated = storage_int_check_and_set_fingerprint(lfs_data);
    bool need_format = furi_hal_rtc_is_flag_set(FuriHalRtcFlagFactoryReset) ||
                       was_fingerprint_outdated;

    if(need_format) {
        // Format storage
        err = lfs_format(lfs, &lfs_data->config);
        if(err == 0) {
            FURI_LOG_I(TAG, "Factory reset: Format successful, trying to mount");
            furi_hal_rtc_reset_flag(FuriHalRtcFlagFactoryReset);
            err = lfs_mount(lfs, &lfs_data->config);
            if(err == 0) {
                FURI_LOG_I(TAG, "Factory reset: Mounted");
                storage->status = StorageStatusOK;
            } else {
                FURI_LOG_E(TAG, "Factory reset: Mount after format failed");
                storage->status = StorageStatusNotMounted;
            }
        } else {
            FURI_LOG_E(TAG, "Factory reset: Format failed");
            storage->status = StorageStatusNoFS;
        }
    } else {
        // Normal
        err = lfs_mount(lfs, &lfs_data->config);
        if(err == 0) {
            FURI_LOG_I(TAG, "Mounted");
            storage->status = StorageStatusOK;
        } else {
            FURI_LOG_E(TAG, "Mount failed, formatting");
            err = lfs_format(lfs, &lfs_data->config);
            if(err == 0) {
                FURI_LOG_I(TAG, "Format successful, trying to mount");
                err = lfs_mount(lfs, &lfs_data->config);
                if(err == 0) {
                    FURI_LOG_I(TAG, "Mounted");
                    storage->status = StorageStatusOK;
                } else {
                    FURI_LOG_E(TAG, "Mount after format failed");
                    storage->status = StorageStatusNotMounted;
                }
            } else {
                FURI_LOG_E(TAG, "Format failed");
                storage->status = StorageStatusNoFS;
            }
        }
    }
}

/****************** Common Functions ******************/

static FS_Error storage_int_parse_error(int error) {
    FS_Error result = FSE_INTERNAL;

    if(error >= LFS_ERR_OK) {
        result = FSE_OK;
    } else {
        switch(error) {
        case LFS_ERR_IO:
            result = FSE_INTERNAL;
            break;
        case LFS_ERR_CORRUPT:
            result = FSE_INTERNAL;
            break;
        case LFS_ERR_NOENT:
            result = FSE_NOT_EXIST;
            break;
        case LFS_ERR_EXIST:
            result = FSE_EXIST;
            break;
        case LFS_ERR_NOTDIR:
            result = FSE_INVALID_NAME;
            break;
        case LFS_ERR_ISDIR:
            result = FSE_INVALID_NAME;
            break;
        case LFS_ERR_NOTEMPTY:
            result = FSE_DENIED;
            break;
        case LFS_ERR_BADF:
            result = FSE_INVALID_NAME;
            break;
        case LFS_ERR_FBIG:
            result = FSE_INTERNAL;
            break;
        case LFS_ERR_INVAL:
            result = FSE_INVALID_PARAMETER;
            break;
        case LFS_ERR_NOSPC:
            result = FSE_INTERNAL;
            break;
        case LFS_ERR_NOMEM:
            result = FSE_INTERNAL;
            break;
        case LFS_ERR_NOATTR:
            result = FSE_INVALID_PARAMETER;
            break;
        case LFS_ERR_NAMETOOLONG:
            result = FSE_INVALID_NAME;
            break;
        default:
            break;
        }
    }

    return result;
}

/* Returns false if less than reserved space is left free */
static bool storage_int_check_for_free_space(StorageData* storage) {
    LFSData* lfs_data = lfs_data_get_from_storage(storage);

    lfs_ssize_t result = lfs_fs_size(lfs_get_from_storage(storage));
    if(result >= 0) {
        lfs_size_t free_space =
            (lfs_data->config.block_count - result) * lfs_data->config.block_size;

        return (free_space > LFS_RESERVED_PAGES_COUNT * furi_hal_flash_get_page_size());
    }

    return false;
}
/******************* File Functions *******************/

static bool storage_int_file_open(
    void* ctx,
    File* file,
    const char* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);

    bool enough_free_space = storage_int_check_for_free_space(storage);

    int flags = 0;

    if(access_mode & FSAM_READ) flags |= LFS_O_RDONLY;
    if(access_mode & FSAM_WRITE) flags |= LFS_O_WRONLY;

    if(open_mode & FSOM_OPEN_EXISTING) flags |= 0;
    if(open_mode & FSOM_OPEN_ALWAYS) flags |= LFS_O_CREAT;
    if(open_mode & FSOM_OPEN_APPEND) flags |= LFS_O_CREAT | LFS_O_APPEND;
    if(open_mode & FSOM_CREATE_NEW) flags |= LFS_O_CREAT | LFS_O_EXCL;
    if(open_mode & FSOM_CREATE_ALWAYS) flags |= LFS_O_CREAT | LFS_O_TRUNC;

    LFSHandle* handle = lfs_handle_alloc_file();
    storage_set_storage_file_data(file, handle, storage);

    if(!enough_free_space) {
        string_t filename;
        string_init(filename);
        path_extract_basename(path, filename);
        bool is_dot_file = (!string_empty_p(filename) && (string_get_char(filename, 0) == '.'));
        string_clear(filename);

        /* Restrict write & creation access to all non-dot files */
        if(!is_dot_file && (flags & (LFS_O_CREAT | LFS_O_WRONLY))) {
            file->internal_error_id = LFS_ERR_NOSPC;
            file->error_id = FSE_DENIED;
            FURI_LOG_W(TAG, "Denied access to '%s': no free space", path);
            return false;
        }
    }

    file->internal_error_id = lfs_file_open(lfs, lfs_handle_get_file(handle), path, flags);

    if(file->internal_error_id >= LFS_ERR_OK) {
        lfs_handle_set_open(handle);
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);

    return (file->error_id == FSE_OK);
}

static bool storage_int_file_close(void* ctx, File* file) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    if(lfs_handle_is_open(handle)) {
        file->internal_error_id = lfs_file_close(lfs, lfs_handle_get_file(handle));
    } else {
        file->internal_error_id = LFS_ERR_BADF;
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);
    lfs_handle_free(handle);
    return (file->error_id == FSE_OK);
}

static uint16_t
    storage_int_file_read(void* ctx, File* file, void* buff, uint16_t const bytes_to_read) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    uint16_t bytes_read = 0;

    if(lfs_handle_is_open(handle)) {
        file->internal_error_id =
            lfs_file_read(lfs, lfs_handle_get_file(handle), buff, bytes_to_read);
    } else {
        file->internal_error_id = LFS_ERR_BADF;
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);

    if(file->error_id == FSE_OK) {
        bytes_read = file->internal_error_id;
        file->internal_error_id = 0;
    }
    return bytes_read;
}

static uint16_t
    storage_int_file_write(void* ctx, File* file, const void* buff, uint16_t const bytes_to_write) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    uint16_t bytes_written = 0;

    if(lfs_handle_is_open(handle)) {
        file->internal_error_id =
            lfs_file_write(lfs, lfs_handle_get_file(handle), buff, bytes_to_write);
    } else {
        file->internal_error_id = LFS_ERR_BADF;
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);

    if(file->error_id == FSE_OK) {
        bytes_written = file->internal_error_id;
        file->internal_error_id = 0;
    }
    return bytes_written;
}

static bool
    storage_int_file_seek(void* ctx, File* file, const uint32_t offset, const bool from_start) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    if(lfs_handle_is_open(handle)) {
        if(from_start) {
            file->internal_error_id =
                lfs_file_seek(lfs, lfs_handle_get_file(handle), offset, LFS_SEEK_SET);
        } else {
            file->internal_error_id =
                lfs_file_seek(lfs, lfs_handle_get_file(handle), offset, LFS_SEEK_CUR);
        }
    } else {
        file->internal_error_id = LFS_ERR_BADF;
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
}

static uint64_t storage_int_file_tell(void* ctx, File* file) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    if(lfs_handle_is_open(handle)) {
        file->internal_error_id = lfs_file_tell(lfs, lfs_handle_get_file(handle));
    } else {
        file->internal_error_id = LFS_ERR_BADF;
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);

    int32_t position = 0;
    if(file->error_id == FSE_OK) {
        position = file->internal_error_id;
        file->internal_error_id = 0;
    }

    return position;
}

static bool storage_int_file_truncate(void* ctx, File* file) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    if(lfs_handle_is_open(handle)) {
        file->internal_error_id = lfs_file_tell(lfs, lfs_handle_get_file(handle));
        file->error_id = storage_int_parse_error(file->internal_error_id);

        if(file->error_id == FSE_OK) {
            uint32_t position = file->internal_error_id;
            file->internal_error_id =
                lfs_file_truncate(lfs, lfs_handle_get_file(handle), position);
            file->error_id = storage_int_parse_error(file->internal_error_id);
        }
    } else {
        file->internal_error_id = LFS_ERR_BADF;
        file->error_id = storage_int_parse_error(file->internal_error_id);
    }

    return (file->error_id == FSE_OK);
}

static bool storage_int_file_sync(void* ctx, File* file) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    if(lfs_handle_is_open(handle)) {
        file->internal_error_id = lfs_file_sync(lfs, lfs_handle_get_file(handle));
    } else {
        file->internal_error_id = LFS_ERR_BADF;
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
}

static uint64_t storage_int_file_size(void* ctx, File* file) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    if(lfs_handle_is_open(handle)) {
        file->internal_error_id = lfs_file_size(lfs, lfs_handle_get_file(handle));
    } else {
        file->internal_error_id = LFS_ERR_BADF;
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);

    uint32_t size = 0;
    if(file->error_id == FSE_OK) {
        size = file->internal_error_id;
        file->internal_error_id = 0;
    }

    return size;
}

static bool storage_int_file_eof(void* ctx, File* file) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    bool eof = true;

    if(lfs_handle_is_open(handle)) {
        int32_t position = lfs_file_tell(lfs, lfs_handle_get_file(handle));
        int32_t size = lfs_file_size(lfs, lfs_handle_get_file(handle));

        if(position < 0) {
            file->internal_error_id = position;
        } else if(size < 0) {
            file->internal_error_id = size;
        } else {
            file->internal_error_id = LFS_ERR_OK;
            eof = (position >= size);
        }
    } else {
        file->internal_error_id = LFS_ERR_BADF;
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);
    return eof;
}

/******************* Dir Functions *******************/

static bool storage_int_dir_open(void* ctx, File* file, const char* path) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);

    LFSHandle* handle = lfs_handle_alloc_dir();
    storage_set_storage_file_data(file, handle, storage);

    file->internal_error_id = lfs_dir_open(lfs, lfs_handle_get_dir(handle), path);
    if(file->internal_error_id >= LFS_ERR_OK) {
        lfs_handle_set_open(handle);
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
}

static bool storage_int_dir_close(void* ctx, File* file) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    if(lfs_handle_is_open(handle)) {
        file->internal_error_id = lfs_dir_close(lfs, lfs_handle_get_dir(handle));
    } else {
        file->internal_error_id = LFS_ERR_BADF;
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);
    lfs_handle_free(handle);
    return (file->error_id == FSE_OK);
}

static bool storage_int_dir_read(
    void* ctx,
    File* file,
    FileInfo* fileinfo,
    char* name,
    const uint16_t name_length) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    if(lfs_handle_is_open(handle)) {
        struct lfs_info _fileinfo;

        // LFS returns virtual directories "." and "..", so we read until we get something meaningful or an empty string
        do {
            file->internal_error_id = lfs_dir_read(lfs, lfs_handle_get_dir(handle), &_fileinfo);
            file->error_id = storage_int_parse_error(file->internal_error_id);
        } while(strcmp(_fileinfo.name, ".") == 0 || strcmp(_fileinfo.name, "..") == 0);

        if(fileinfo != NULL) {
            fileinfo->size = _fileinfo.size;
            fileinfo->flags = 0;
            if(_fileinfo.type & LFS_TYPE_DIR) fileinfo->flags |= FSF_DIRECTORY;
        }

        if(name != NULL) {
            snprintf(name, name_length, "%s", _fileinfo.name);
        }

        // set FSE_NOT_EXIST error on end of directory
        if(file->internal_error_id == 0) {
            file->error_id = FSE_NOT_EXIST;
        }
    } else {
        file->internal_error_id = LFS_ERR_BADF;
        file->error_id = storage_int_parse_error(file->internal_error_id);
    }

    return (file->error_id == FSE_OK);
}

static bool storage_int_dir_rewind(void* ctx, File* file) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSHandle* handle = storage_get_storage_file_data(file, storage);

    if(lfs_handle_is_open(handle)) {
        file->internal_error_id = lfs_dir_rewind(lfs, lfs_handle_get_dir(handle));
    } else {
        file->internal_error_id = LFS_ERR_BADF;
    }

    file->error_id = storage_int_parse_error(file->internal_error_id);
    return (file->error_id == FSE_OK);
}

/******************* Common FS Functions *******************/

static FS_Error storage_int_common_stat(void* ctx, const char* path, FileInfo* fileinfo) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    struct lfs_info _fileinfo;
    int result = lfs_stat(lfs, path, &_fileinfo);

    if(fileinfo != NULL) {
        fileinfo->size = _fileinfo.size;
        fileinfo->flags = 0;
        if(_fileinfo.type & LFS_TYPE_DIR) fileinfo->flags |= FSF_DIRECTORY;
    }

    return storage_int_parse_error(result);
}

static FS_Error storage_int_common_remove(void* ctx, const char* path) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    int result = lfs_remove(lfs, path);
    return storage_int_parse_error(result);
}

static FS_Error storage_int_common_mkdir(void* ctx, const char* path) {
    StorageData* storage = ctx;
    lfs_t* lfs = lfs_get_from_storage(storage);
    int result = lfs_mkdir(lfs, path);
    return storage_int_parse_error(result);
}

static FS_Error storage_int_common_fs_info(
    void* ctx,
    const char* fs_path,
    uint64_t* total_space,
    uint64_t* free_space) {
    UNUSED(fs_path);
    StorageData* storage = ctx;

    lfs_t* lfs = lfs_get_from_storage(storage);
    LFSData* lfs_data = lfs_data_get_from_storage(storage);

    if(total_space) {
        *total_space = lfs_data->config.block_size * lfs_data->config.block_count;
    }

    lfs_ssize_t result = lfs_fs_size(lfs);
    if(free_space && (result >= 0)) {
        *free_space = (lfs_data->config.block_count - result) * lfs_data->config.block_size;
    }

    return storage_int_parse_error(result);
}

/******************* Init Storage *******************/
static const FS_Api fs_api = {
    .file =
        {
            .open = storage_int_file_open,
            .close = storage_int_file_close,
            .read = storage_int_file_read,
            .write = storage_int_file_write,
            .seek = storage_int_file_seek,
            .tell = storage_int_file_tell,
            .truncate = storage_int_file_truncate,
            .size = storage_int_file_size,
            .sync = storage_int_file_sync,
            .eof = storage_int_file_eof,
        },
    .dir =
        {
            .open = storage_int_dir_open,
            .close = storage_int_dir_close,
            .read = storage_int_dir_read,
            .rewind = storage_int_dir_rewind,
        },
    .common =
        {
            .stat = storage_int_common_stat,
            .mkdir = storage_int_common_mkdir,
            .remove = storage_int_common_remove,
            .fs_info = storage_int_common_fs_info,
        },
};

void storage_int_init(StorageData* storage) {
    FURI_LOG_I(TAG, "Starting");
    LFSData* lfs_data = storage_int_lfs_data_alloc();
    FURI_LOG_I(
        TAG,
        "Config: start %p, read %d, write %d, page size: %d, page count: %d, cycles: %d",
        lfs_data->start_address,
        lfs_data->config.read_size,
        lfs_data->config.prog_size,
        lfs_data->config.block_size,
        lfs_data->config.block_count,
        lfs_data->config.block_cycles);

    storage_int_lfs_mount(lfs_data, storage);

    storage->data = lfs_data;
    storage->api.tick = NULL;
    storage->fs_api = &fs_api;
}
