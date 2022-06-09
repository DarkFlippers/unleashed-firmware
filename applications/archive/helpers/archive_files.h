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
    string_t path;
    ArchiveFileTypeEnum type;
    bool fav;
    bool is_app;
} ArchiveFile_t;

static void ArchiveFile_t_init(ArchiveFile_t* obj) {
    obj->type = ArchiveFileTypeUnknown;
    obj->is_app = false;
    obj->fav = false;
    string_init(obj->path);
}

static void ArchiveFile_t_init_set(ArchiveFile_t* obj, const ArchiveFile_t* src) {
    obj->type = src->type;
    obj->is_app = src->is_app;
    obj->fav = src->fav;
    string_init_set(obj->path, src->path);
}

static void ArchiveFile_t_set(ArchiveFile_t* obj, const ArchiveFile_t* src) {
    obj->type = src->type;
    obj->is_app = src->is_app;
    obj->fav = src->fav;
    string_set(obj->path, src->path);
}

static void ArchiveFile_t_clear(ArchiveFile_t* obj) {
    string_clear(obj->path);
}

ARRAY_DEF(
    files_array,
    ArchiveFile_t,
    (INIT(API_2(ArchiveFile_t_init)),
     SET(API_6(ArchiveFile_t_set)),
     INIT_SET(API_6(ArchiveFile_t_init_set)),
     CLEAR(API_2(ArchiveFile_t_clear))))

void archive_set_file_type(ArchiveFile_t* file, const char* path, bool is_folder, bool is_app);
bool archive_get_items(void* context, const char* path);
void archive_file_append(const char* path, const char* format, ...);
void archive_delete_file(void* context, const char* format, ...);
