#pragma once
#include <furi.h>
#include <toolbox/api_lock.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    File* file;
    const char* path;
    FS_AccessMode access_mode;
    FS_OpenMode open_mode;
    FuriThreadId thread_id;
} SADataFOpen;

typedef struct {
    File* file;
    void* buff;
    uint16_t bytes_to_read;
} SADataFRead;

typedef struct {
    File* file;
    const void* buff;
    uint16_t bytes_to_write;
} SADataFWrite;

typedef struct {
    File* file;
    uint32_t offset;
    bool from_start;
} SADataFSeek;

typedef struct {
    File* file;
    const char* path;
    FuriThreadId thread_id;
} SADataDOpen;

typedef struct {
    File* file;
    FileInfo* fileinfo;
    char* name;
    uint16_t name_length;
} SADataDRead;

typedef struct {
    const char* path;
    uint32_t* timestamp;
    FuriThreadId thread_id;
} SADataCTimestamp;

typedef struct {
    const char* path;
    FileInfo* fileinfo;
    FuriThreadId thread_id;
} SADataCStat;

typedef struct {
    const char* fs_path;
    uint64_t* total_space;
    uint64_t* free_space;
    FuriThreadId thread_id;
} SADataCFSInfo;

typedef struct {
    FuriString* path;
    FuriThreadId thread_id;
} SADataCResolvePath;

typedef struct {
    uint32_t id;
} SADataError;

typedef struct {
    const char* path;
    FuriThreadId thread_id;
} SADataPath;

typedef struct {
    File* file;
} SADataFile;

typedef struct {
    SDInfo* info;
} SAInfo;

typedef union {
    SADataFOpen fopen;
    SADataFRead fread;
    SADataFWrite fwrite;
    SADataFSeek fseek;

    SADataDOpen dopen;
    SADataDRead dread;

    SADataCTimestamp ctimestamp;
    SADataCStat cstat;
    SADataCFSInfo cfsinfo;
    SADataCResolvePath cresolvepath;

    SADataError error;

    SADataFile file;
    SADataPath path;

    SAInfo sdinfo;
} SAData;

typedef union {
    bool bool_value;
    uint16_t uint16_value;
    uint64_t uint64_value;
    FS_Error error_value;
    const char* cstring_value;
} SAReturn;

typedef enum {
    StorageCommandFileOpen,
    StorageCommandFileClose,
    StorageCommandFileRead,
    StorageCommandFileWrite,
    StorageCommandFileSeek,
    StorageCommandFileTell,
    StorageCommandFileTruncate,
    StorageCommandFileSize,
    StorageCommandFileSync,
    StorageCommandFileEof,
    StorageCommandDirOpen,
    StorageCommandDirClose,
    StorageCommandDirRead,
    StorageCommandDirRewind,
    StorageCommandCommonTimestamp,
    StorageCommandCommonStat,
    StorageCommandCommonRemove,
    StorageCommandCommonMkDir,
    StorageCommandCommonFSInfo,
    StorageCommandSDFormat,
    StorageCommandSDUnmount,
    StorageCommandSDInfo,
    StorageCommandSDStatus,
    StorageCommandCommonResolvePath,
} StorageCommand;

typedef struct {
    FuriApiLock lock;
    StorageCommand command;
    SAData* data;
    SAReturn* return_data;
} StorageMessage;

#ifdef __cplusplus
}
#endif
