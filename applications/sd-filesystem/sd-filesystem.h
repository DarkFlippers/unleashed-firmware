#pragma once

#include <furi.h>
#include <api-hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <m-string.h>
#include "sd-card-api.h"
#include "view_holder.h"
#include <notification/notification-messages.h>

#define SD_FS_MAX_FILES _FS_LOCK
#define SD_STATE_LINES_COUNT 6

/* api data */
typedef FIL SDFile;
typedef DIR SDDir;
typedef FILINFO SDFileInfo;

/* storage for file/directory objects*/
typedef union {
    SDFile file;
    SDDir dir;
} SDFileDirStorage;

typedef enum {
    SD_OK = FR_OK,
    SD_DISK_ERR = FR_DISK_ERR,
    SD_INT_ERR = FR_INT_ERR,
    SD_NO_FILE = FR_NO_FILE,
    SD_NO_PATH = FR_NO_PATH,
    SD_INVALID_NAME = FR_INVALID_NAME,
    SD_DENIED = FR_DENIED,
    SD_EXIST = FR_EXIST,
    SD_INVALID_OBJECT = FR_INVALID_OBJECT,
    SD_WRITE_PROTECTED = FR_WRITE_PROTECTED,
    SD_INVALID_DRIVE = FR_INVALID_DRIVE,
    SD_NOT_ENABLED = FR_NOT_ENABLED,
    SD_NO_FILESYSTEM = FR_NO_FILESYSTEM,
    SD_MKFS_ABORTED = FR_MKFS_ABORTED,
    SD_TIMEOUT = FR_TIMEOUT,
    SD_LOCKED = FR_LOCKED,
    SD_NOT_ENOUGH_CORE = FR_NOT_ENOUGH_CORE,
    SD_TOO_MANY_OPEN_FILES = FR_TOO_MANY_OPEN_FILES,
    SD_INVALID_PARAMETER = FR_INVALID_PARAMETER,
    SD_NO_CARD,
    SD_NOT_A_FILE,
    SD_NOT_A_DIR,
    SD_OTHER_APP,
    SD_LOW_LEVEL_ERR,
} SDError;

typedef enum {
    FDF_DIR,
    FDF_FILE,
    FDF_ANY,
} FiledataFilter;

typedef struct {
    osThreadId_t thread_id;
    bool is_dir;
    SDFileDirStorage data;
} FileData;

/* application data */
typedef struct {
    ViewPort* view_port;
    Icon* mounted;
    Icon* fail;
} SdFsIcon;

typedef struct {
    osMutexId_t mutex;
    FileData files[SD_FS_MAX_FILES];
    SDError status;
    char* path;
    FATFS fat_fs;
} SdFsInfo;

typedef enum {
    SdAppStateBackground,
    SdAppStateFormat,
    SdAppStateFormatInProgress,
    SdAppStateFormatCompleted,
    SdAppStateInfo,
    SdAppStateEject,
    SdAppStateEjected,
    SdAppStateFileSelect,
    SdAppStateCheckError,
} SdAppState;

struct SdApp {
    SdFsInfo info;
    SdFsIcon icon;

    SdCard_Api sd_card_api;
    SdAppState sd_app_state;

    ViewHolder* view_holder;
    osMessageQueueId_t result_receiver;

    osMessageQueueId_t event_queue;
    string_t text_holder;

    NotificationApp* notifications;
};

/* core api fns */
bool _fs_init(SdFsInfo* _fs_info);
bool _fs_lock(SdFsInfo* fs_info);
bool _fs_unlock(SdFsInfo* fs_info);
SDError _fs_status(SdFsInfo* fs_info);

/* sd api fns */
bool fs_file_open(File* file, const char* path, FS_AccessMode access_mode, FS_OpenMode open_mode);
bool fs_file_close(File* file);
uint16_t fs_file_read(File* file, void* buff, uint16_t bytes_to_read);
uint16_t fs_file_write(File* file, void* buff, uint16_t bytes_to_write);
bool fs_file_seek(File* file, uint32_t offset, bool from_start);
uint64_t fs_file_tell(File* file);
bool fs_file_truncate(File* file);
uint64_t fs_file_size(File* file);
bool fs_file_sync(File* file);
bool fs_file_eof(File* file);

/* dir api */
bool fs_dir_open(File* file, const char* path);
bool fs_dir_close(File* file);
bool fs_dir_read(File* file, FileInfo* fileinfo, char* name, uint16_t name_length);
bool fs_dir_rewind(File* file);

/* common api */
FS_Error
fs_common_info(const char* path, FileInfo* fileinfo, char* name, const uint16_t name_length);
FS_Error fs_common_remove(const char* path);
FS_Error fs_common_rename(const char* old_path, const char* new_path);
FS_Error fs_common_set_attr(const char* path, uint8_t attr, uint8_t mask);
FS_Error fs_common_mkdir(const char* path);
FS_Error fs_common_set_time(const char* path, FileDateUnion date, FileTimeUnion time);
FS_Error fs_get_fs_info(uint64_t* total_space, uint64_t* free_space);

/* errors api */
const char* fs_error_get_desc(FS_Error error_id);
const char* fs_error_get_internal_desc(uint32_t internal_error_id);