#include "fatfs.h"
#include "filesystem-api.h"
#include "sd-filesystem.h"

/******************* Global vars for api *******************/

static SdFsInfo* fs_info;

/******************* Core Functions *******************/

bool _fs_init(SdFsInfo* _fs_info) {
    bool result = true;
    _fs_info->mutex = osMutexNew(NULL);
    if(_fs_info->mutex == NULL) result = false;

    for(uint8_t i = 0; i < SD_FS_MAX_FILES; i++) {
        _fs_info->files[i].thread_id = NULL;
    }

    _fs_info->path = "0:/";
    _fs_info->status = SD_NO_CARD;

    // store pointer for api fns
    fs_info = _fs_info;

    return result;
}

bool _fs_lock(SdFsInfo* fs_info) {
    api_hal_power_insomnia_enter();
    return (osMutexAcquire(fs_info->mutex, osWaitForever) == osOK);
}

bool _fs_unlock(SdFsInfo* fs_info) {
    api_hal_power_insomnia_exit();
    return (osMutexRelease(fs_info->mutex) == osOK);
}

SDError _get_filedata(SdFsInfo* fs_info, File* file, FileData** filedata, FiledataFilter filter) {
    SDError error = SD_OK;
    _fs_lock(fs_info);

    if(fs_info->status == SD_OK) {
        if(file != NULL && file->file_id < SD_FS_MAX_FILES) {
            if(fs_info->files[file->file_id].thread_id == osThreadGetId()) {
                if(filter == FDF_ANY) {
                    // any type
                    *filedata = &fs_info->files[file->file_id];
                } else if(filter == FDF_FILE) {
                    // file type
                    if(!fs_info->files[file->file_id].is_dir) {
                        *filedata = &fs_info->files[file->file_id];
                    } else {
                        error = SD_NOT_A_FILE;
                    }
                } else if(filter == FDF_DIR) {
                    // dir type
                    if(fs_info->files[file->file_id].is_dir) {
                        *filedata = &fs_info->files[file->file_id];
                    } else {
                        error = SD_NOT_A_DIR;
                    }
                }
            } else {
                error = SD_OTHER_APP;
            }
        } else {
            error = SD_INVALID_PARAMETER;
        }
    } else {
        error = SD_NO_CARD;
    }

    _fs_unlock(fs_info);
    return error;
}

SDError _get_file(SdFsInfo* fs_info, File* file, FileData** filedata) {
    return _get_filedata(fs_info, file, filedata, FDF_FILE);
}

SDError _get_dir(SdFsInfo* fs_info, File* file, FileData** filedata) {
    return _get_filedata(fs_info, file, filedata, FDF_DIR);
}

SDError _get_any(SdFsInfo* fs_info, File* file, FileData** filedata) {
    return _get_filedata(fs_info, file, filedata, FDF_ANY);
}

SDError _fs_status(SdFsInfo* fs_info) {
    SDError result;

    _fs_lock(fs_info);
    result = fs_info->status;
    _fs_unlock(fs_info);

    return result;
}

void _fs_on_client_app_exit(SdFsInfo* fs_info) {
    _fs_lock(fs_info);
    for(uint8_t i = 0; i < SD_FS_MAX_FILES; i++) {
        if(fs_info->files[i].thread_id == osThreadGetId()) {
            if(fs_info->files[i].is_dir) {
                // TODO close dir
            } else {
                // TODO close file
            }
        }
    }
    _fs_unlock(fs_info);
}

FS_Error _fs_parse_error(SDError error) {
    FS_Error result;
    switch(error) {
    case SD_OK:
        result = FSE_OK;
        break;
    case SD_INT_ERR:
        result = FSE_INTERNAL;
        break;
    case SD_NO_FILE:
        result = FSE_NOT_EXIST;
        break;
    case SD_NO_PATH:
        result = FSE_NOT_EXIST;
        break;
    case SD_INVALID_NAME:
        result = FSE_INVALID_NAME;
        break;
    case SD_DENIED:
        result = FSE_DENIED;
        break;
    case SD_EXIST:
        result = FSE_EXIST;
        break;
    case SD_INVALID_OBJECT:
        result = FSE_INTERNAL;
        break;
    case SD_WRITE_PROTECTED:
        result = FSE_INTERNAL;
        break;
    case SD_INVALID_DRIVE:
        result = FSE_INTERNAL;
        break;
    case SD_NOT_ENABLED:
        result = FSE_INTERNAL;
        break;
    case SD_NO_FILESYSTEM:
        result = FSE_NOT_READY;
        break;
    case SD_MKFS_ABORTED:
        result = FSE_INTERNAL;
        break;
    case SD_TIMEOUT:
        result = FSE_INTERNAL;
        break;
    case SD_LOCKED:
        result = FSE_INTERNAL;
        break;
    case SD_NOT_ENOUGH_CORE:
        result = FSE_INTERNAL;
        break;
    case SD_TOO_MANY_OPEN_FILES:
        result = FSE_INTERNAL;
        break;
    case SD_INVALID_PARAMETER:
        result = FSE_INVALID_PARAMETER;
        break;
    case SD_NO_CARD:
        result = FSE_NOT_READY;
        break;
    case SD_NOT_A_FILE:
        result = FSE_INVALID_PARAMETER;
        break;
    case SD_NOT_A_DIR:
        result = FSE_INVALID_PARAMETER;
        break;
    case SD_OTHER_APP:
        result = FSE_INTERNAL;
        break;

    default:
        result = FSE_INTERNAL;
        break;
    }

    return result;
}

/******************* File Functions *******************/

// Open/Create a file
bool fs_file_open(File* file, const char* path, FS_AccessMode access_mode, FS_OpenMode open_mode) {
    SDFile* sd_file = NULL;

    _fs_lock(fs_info);
    for(uint8_t index = 0; index < SD_FS_MAX_FILES; index++) {
        FileData* filedata = &fs_info->files[index];

        if(filedata->thread_id == NULL) {
            file->file_id = index;

            memset(&(filedata->data), 0, sizeof(SDFileDirStorage));
            filedata->thread_id = osThreadGetId();
            filedata->is_dir = false;
            sd_file = &(filedata->data.file);

            break;
        }
    }
    _fs_unlock(fs_info);

    if(sd_file == NULL) {
        file->internal_error_id = SD_TOO_MANY_OPEN_FILES;
    } else {
        uint8_t _mode = 0;

        if(access_mode & FSAM_READ) _mode |= FA_READ;
        if(access_mode & FSAM_WRITE) _mode |= FA_WRITE;
        if(open_mode & FSOM_OPEN_EXISTING) _mode |= FA_OPEN_EXISTING;
        if(open_mode & FSOM_OPEN_ALWAYS) _mode |= FA_OPEN_ALWAYS;
        if(open_mode & FSOM_OPEN_APPEND) _mode |= FA_OPEN_APPEND;
        if(open_mode & FSOM_CREATE_NEW) _mode |= FA_CREATE_NEW;
        if(open_mode & FSOM_CREATE_ALWAYS) _mode |= FA_CREATE_ALWAYS;

        file->internal_error_id = f_open(sd_file, path, _mode);
    }

    // TODO on exit
    //furiac_onexit(_fs_on_client_app_exit, fs_info);

    file->error_id = _fs_parse_error(file->internal_error_id);
    return (file->internal_error_id == SD_OK);
}

// Close an opened file
bool fs_file_close(File* file) {
    FileData* filedata = NULL;
    file->internal_error_id = _get_file(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        file->internal_error_id = f_close(&filedata->data.file);

        _fs_lock(fs_info);
        filedata->thread_id = NULL;
        _fs_unlock(fs_info);
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return (file->internal_error_id == SD_OK);
}

// Read data from the file
uint16_t fs_file_read(File* file, void* buff, uint16_t const bytes_to_read) {
    FileData* filedata = NULL;
    uint16_t bytes_readed = 0;

    file->internal_error_id = _get_file(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        file->internal_error_id = f_read(&filedata->data.file, buff, bytes_to_read, &bytes_readed);
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return bytes_readed;
}

// Write data to the file
uint16_t fs_file_write(File* file, const void* buff, uint16_t const bytes_to_write) {
    FileData* filedata = NULL;
    uint16_t bytes_written = 0;

    file->internal_error_id = _get_file(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        file->internal_error_id =
            f_write(&filedata->data.file, buff, bytes_to_write, &bytes_written);
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return bytes_written;
}

// Move read/write pointer, expand size
bool fs_file_seek(File* file, const uint32_t offset, const bool from_start) {
    FileData* filedata = NULL;

    file->internal_error_id = _get_file(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        if(from_start) {
            file->internal_error_id = f_lseek(&filedata->data.file, offset);
        } else {
            uint64_t position = f_tell(&filedata->data.file);
            position += offset;
            file->internal_error_id = f_lseek(&filedata->data.file, position);
        }
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return (file->internal_error_id == SD_OK);
}

// Tell pointer position
uint64_t fs_file_tell(File* file) {
    FileData* filedata = NULL;
    uint64_t position = 0;
    file->internal_error_id = _get_file(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        position = f_tell(&filedata->data.file);
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return position;
}

// Truncate file size to current pointer value
bool fs_file_truncate(File* file) {
    FileData* filedata = NULL;

    file->internal_error_id = _get_file(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        file->internal_error_id = f_truncate(&filedata->data.file);
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return (file->internal_error_id == SD_OK);
}

// Flush cached data
bool fs_file_sync(File* file) {
    FileData* filedata = NULL;

    file->internal_error_id = _get_file(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        file->internal_error_id = f_sync(&filedata->data.file);
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return (file->internal_error_id == SD_OK);
}

// Get size
uint64_t fs_file_size(File* file) {
    FileData* filedata = NULL;
    uint64_t size = 0;
    file->internal_error_id = _get_file(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        size = f_size(&filedata->data.file);
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return size;
}

// Test EOF
bool fs_file_eof(File* file) {
    FileData* filedata = NULL;
    bool eof = true;
    file->internal_error_id = _get_file(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        eof = f_eof(&filedata->data.file);
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return eof;
}

/******************* Dir Functions *******************/

// Open directory
bool fs_dir_open(File* file, const char* path) {
    SDDir* sd_dir = NULL;

    _fs_lock(fs_info);
    for(uint8_t index = 0; index < SD_FS_MAX_FILES; index++) {
        FileData* filedata = &fs_info->files[index];

        if(filedata->thread_id == NULL) {
            file->file_id = index;

            memset(&(filedata->data), 0, sizeof(SDFileDirStorage));
            filedata->thread_id = osThreadGetId();
            filedata->is_dir = true;
            sd_dir = &(filedata->data.dir);

            break;
        }
    }
    _fs_unlock(fs_info);

    if(sd_dir == NULL) {
        file->internal_error_id = SD_TOO_MANY_OPEN_FILES;
    } else {
        file->internal_error_id = f_opendir(sd_dir, path);
    }

    // TODO on exit
    //furiac_onexit(_fs_on_client_app_exit, fs_info);

    file->error_id = _fs_parse_error(file->internal_error_id);
    return (file->internal_error_id == SD_OK);
}

// Close directory
bool fs_dir_close(File* file) {
    FileData* filedata = NULL;
    file->internal_error_id = _get_dir(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        file->internal_error_id = f_closedir(&filedata->data.dir);

        _fs_lock(fs_info);
        filedata->thread_id = NULL;
        _fs_unlock(fs_info);
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return (file->internal_error_id == SD_OK);
}

// Read next file info and name from directory
bool fs_dir_read(File* file, FileInfo* fileinfo, char* name, const uint16_t name_length) {
    FileData* filedata = NULL;
    file->internal_error_id = _get_dir(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        SDFileInfo _fileinfo;
        file->internal_error_id = f_readdir(&filedata->data.dir, &_fileinfo);

        if(fileinfo != NULL) {
            fileinfo->date.value = _fileinfo.fdate;
            fileinfo->time.value = _fileinfo.ftime;
            fileinfo->size = _fileinfo.fsize;
            fileinfo->flags = 0;

            if(_fileinfo.fattrib & AM_RDO) fileinfo->flags |= FSF_READ_ONLY;
            if(_fileinfo.fattrib & AM_HID) fileinfo->flags |= FSF_HIDDEN;
            if(_fileinfo.fattrib & AM_SYS) fileinfo->flags |= FSF_SYSTEM;
            if(_fileinfo.fattrib & AM_DIR) fileinfo->flags |= FSF_DIRECTORY;
            if(_fileinfo.fattrib & AM_ARC) fileinfo->flags |= FSF_ARCHIVE;
        }

        if(name != NULL && name_length > 0) {
            strlcpy(name, _fileinfo.fname, name_length);
        }
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return (file->internal_error_id == SD_OK);
}

bool fs_dir_rewind(File* file) {
    FileData* filedata = NULL;
    file->internal_error_id = _get_dir(fs_info, file, &filedata);

    if(file->internal_error_id == SD_OK) {
        file->internal_error_id = f_readdir(&filedata->data.dir, NULL);
    }

    file->error_id = _fs_parse_error(file->internal_error_id);
    return (file->internal_error_id == SD_OK);
}

/******************* Common FS Functions *******************/

// Get info about file/dir
FS_Error
fs_common_info(const char* path, FileInfo* fileinfo, char* name, const uint16_t name_length) {
    SDFileInfo _fileinfo;
    SDError fresult = _fs_status(fs_info);

    if(fresult == SD_OK) {
        fresult = f_stat(path, &_fileinfo);
        if((FRESULT)fresult == FR_OK) {
            if(fileinfo != NULL) {
                fileinfo->date.value = _fileinfo.fdate;
                fileinfo->time.value = _fileinfo.ftime;
                fileinfo->size = _fileinfo.fsize;
                fileinfo->flags = 0;

                if(_fileinfo.fattrib & AM_RDO) fileinfo->flags |= FSF_READ_ONLY;
                if(_fileinfo.fattrib & AM_HID) fileinfo->flags |= FSF_HIDDEN;
                if(_fileinfo.fattrib & AM_SYS) fileinfo->flags |= FSF_SYSTEM;
                if(_fileinfo.fattrib & AM_DIR) fileinfo->flags |= FSF_DIRECTORY;
                if(_fileinfo.fattrib & AM_ARC) fileinfo->flags |= FSF_ARCHIVE;
            }

            if(name != NULL && name_length > 0) {
                strlcpy(name, _fileinfo.fname, name_length);
            }
        }
    }

    return _fs_parse_error(fresult);
}

// Delete file/dir
// File/dir must not have read-only attribute.
// File/dir must be empty.
// File/dir must not be opened, or the FAT volume can be collapsed. FF_FS_LOCK fix that.
FS_Error fs_common_remove(const char* path) {
    SDError fresult = _fs_status(fs_info);

    if(fresult == SD_OK) {
        fresult = f_unlink(path);
    }

    return _fs_parse_error(fresult);
}

// Rename file/dir
// File/dir must not be opened, or the FAT volume can be collapsed. FF_FS_LOCK fix that.
FS_Error fs_common_rename(const char* old_path, const char* new_path) {
    SDError fresult = _fs_status(fs_info);

    if(fresult == SD_OK) {
        fresult = f_rename(old_path, new_path);
    }

    return _fs_parse_error(fresult);
}

// Set attributes of file/dir
// For example:
// set "read only" flag and remove "hidden" flag
// fs_common_set_attr("file.txt", FSF_READ_ONLY, FSF_READ_ONLY | FSF_HIDDEN);
FS_Error fs_common_set_attr(const char* path, uint8_t attr, uint8_t mask) {
    SDError fresult = _fs_status(fs_info);

    if(fresult == SD_OK) {
        uint8_t _mask = 0;
        uint8_t _attr = 0;

        if(mask & FSF_READ_ONLY) _mask |= AM_RDO;
        if(mask & FSF_HIDDEN) _mask |= AM_HID;
        if(mask & FSF_SYSTEM) _mask |= AM_SYS;
        if(mask & FSF_DIRECTORY) _mask |= AM_DIR;
        if(mask & FSF_ARCHIVE) _mask |= AM_ARC;

        if(attr & FSF_READ_ONLY) _attr |= AM_RDO;
        if(attr & FSF_HIDDEN) _attr |= AM_HID;
        if(attr & FSF_SYSTEM) _attr |= AM_SYS;
        if(attr & FSF_DIRECTORY) _attr |= AM_DIR;
        if(attr & FSF_ARCHIVE) _attr |= AM_ARC;

        fresult = f_chmod(path, attr, mask);
    }

    return _fs_parse_error(fresult);
}

// Set time of file/dir
FS_Error fs_common_set_time(const char* path, FileDateUnion date, FileTimeUnion time) {
    SDError fresult = _fs_status(fs_info);

    if(fresult == SD_OK) {
        SDFileInfo _fileinfo;

        _fileinfo.fdate = date.value;
        _fileinfo.ftime = time.value;

        fresult = f_utime(path, &_fileinfo);
    }

    return _fs_parse_error(fresult);
}

// Create new directory
FS_Error fs_common_mkdir(const char* path) {
    SDError fresult = _fs_status(fs_info);

    if(fresult == SD_OK) {
        fresult = f_mkdir(path);
    }

    return _fs_parse_error(fresult);
}

// Get common info about FS
FS_Error fs_get_fs_info(uint64_t* total_space, uint64_t* free_space) {
    SDError fresult = _fs_status(fs_info);

    if(fresult == SD_OK) {
        DWORD free_clusters;
        FATFS* fs;

        fresult = f_getfree("0:/", &free_clusters, &fs);
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
    }

    return _fs_parse_error(fresult);
}

/******************* Error Reporting Functions *******************/

// Get common error description
const char* fs_error_get_desc(FS_Error error_id) {
    const char* result;
    switch(error_id) {
    case(FSE_OK):
        result = "OK";
        break;
    case(FSE_NOT_READY):
        result = "filesystem not ready";
        break;
    case(FSE_EXIST):
        result = "file/dir already exist";
        break;
    case(FSE_NOT_EXIST):
        result = "file/dir not exist";
        break;
    case(FSE_INVALID_PARAMETER):
        result = "invalid parameter";
        break;
    case(FSE_DENIED):
        result = "access denied";
        break;
    case(FSE_INVALID_NAME):
        result = "invalid name/path";
        break;
    case(FSE_INTERNAL):
        result = "internal error";
        break;
    case(FSE_NOT_IMPLEMENTED):
        result = "function not implemented";
        break;
    default:
        result = "unknown error";
        break;
    }
    return result;
}

// Get internal error description
const char* fs_error_get_internal_desc(uint32_t internal_error_id) {
    const char* result;
    switch(internal_error_id) {
    case(SD_OK):
        result = "OK";
        break;
    case(SD_DISK_ERR):
        result = "disk error";
        break;
    case(SD_INT_ERR):
        result = "internal error";
        break;
    case(SD_NO_FILE):
        result = "no file";
        break;
    case(SD_NO_PATH):
        result = "no path";
        break;
    case(SD_INVALID_NAME):
        result = "invalid name";
        break;
    case(SD_DENIED):
        result = "access denied";
        break;
    case(SD_EXIST):
        result = "file/dir exist";
        break;
    case(SD_INVALID_OBJECT):
        result = "invalid object";
        break;
    case(SD_WRITE_PROTECTED):
        result = "write protected";
        break;
    case(SD_INVALID_DRIVE):
        result = "invalid drive";
        break;
    case(SD_NOT_ENABLED):
        result = "not enabled";
        break;
    case(SD_NO_FILESYSTEM):
        result = "no filesystem";
        break;
    case(SD_MKFS_ABORTED):
        result = "aborted";
        break;
    case(SD_TIMEOUT):
        result = "timeout";
        break;
    case(SD_LOCKED):
        result = "file locked";
        break;
    case(SD_NOT_ENOUGH_CORE):
        result = "not enough memory";
        break;
    case(SD_TOO_MANY_OPEN_FILES):
        result = "too many open files";
        break;
    case(SD_INVALID_PARAMETER):
        result = "invalid parameter";
        break;
    case(SD_NO_CARD):
        result = "no SD Card";
        break;
    case(SD_NOT_A_FILE):
        result = "not a file";
        break;
    case(SD_NOT_A_DIR):
        result = "not a directory";
        break;
    case(SD_OTHER_APP):
        result = "opened by other app";
        break;
    case(SD_LOW_LEVEL_ERR):
        result = "low level error";
        break;
    default:
        result = "unknown error";
        break;
    }
    return result;
}