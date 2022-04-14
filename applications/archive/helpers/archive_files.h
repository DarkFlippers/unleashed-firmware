#pragma once

#include <m-array.h>
#include <m-string.h>
#include <storage/storage.h>

typedef enum {
    ArchiveFileTypeIButton,
    ArchiveFileTypeNFC,
    ArchiveFileTypeSubGhz,
    ArchiveFileTypeLFRFID,
    ArchiveFileTypeInfrared,
    ArchiveFileTypeBadUsb,
    ArchiveFileTypeU2f,
    ArchiveFileTypeUpdateManifest,
    ArchiveFileTypeFolder,
    ArchiveFileTypeUnknown,
    ArchiveFileTypeLoading,
} ArchiveFileTypeEnum;

typedef struct {
    string_t name;
    ArchiveFileTypeEnum type;
    bool fav;
    bool is_app;
} ArchiveFile_t;

static void ArchiveFile_t_init(ArchiveFile_t* obj) {
    obj->type = ArchiveFileTypeUnknown;
    obj->is_app = false;
    obj->fav = false;
    string_init(obj->name);
}

static void ArchiveFile_t_init_set(ArchiveFile_t* obj, const ArchiveFile_t* src) {
    obj->type = src->type;
    obj->is_app = src->is_app;
    obj->fav = src->fav;
    string_init_set(obj->name, src->name);
}

static void ArchiveFile_t_set(ArchiveFile_t* obj, const ArchiveFile_t* src) {
    obj->type = src->type;
    obj->is_app = src->is_app;
    obj->fav = src->fav;
    string_set(obj->name, src->name);
}

static void ArchiveFile_t_clear(ArchiveFile_t* obj) {
    string_clear(obj->name);
}

ARRAY_DEF(
    files_array,
    ArchiveFile_t,
    (INIT(API_2(ArchiveFile_t_init)),
     SET(API_6(ArchiveFile_t_set)),
     INIT_SET(API_6(ArchiveFile_t_init_set)),
     CLEAR(API_2(ArchiveFile_t_clear))))

bool archive_filter_by_extension(FileInfo* file_info, const char* tab_ext, const char* name);
void archive_set_file_type(ArchiveFile_t* file, FileInfo* file_info, const char* path, bool is_app);
void archive_trim_file_path(char* name, bool ext);
void archive_get_file_extension(char* name, char* ext);
bool archive_get_filenames(void* context, const char* path);
uint32_t archive_dir_count_items(void* context, const char* path);
uint32_t archive_dir_read_items(void* context, const char* path, uint32_t offset, uint32_t count);
void archive_file_append(const char* path, const char* format, ...);
void archive_delete_file(void* context, const char* format, ...);
