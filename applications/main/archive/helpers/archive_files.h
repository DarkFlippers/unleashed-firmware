#pragma once

#include <m-array.h>
#include <furi.h>
#include <storage/storage.h>

#define FAP_MANIFEST_MAX_ICON_SIZE 32

typedef enum {
    ArchiveFileTypeIButton,
    ArchiveFileTypeNFC,
    ArchiveFileTypeSubGhz,
    ArchiveFileTypeLFRFID,
    ArchiveFileTypeInfrared,
    ArchiveFileTypeBadUsb,
    ArchiveFileTypeU2f,
    ArchiveFileTypeUpdateManifest,
    ArchiveFileTypeApplication,
    ArchiveFileTypeJS,
    ArchiveFileTypeFolder,
    ArchiveFileTypeUnknown,
    ArchiveFileTypeAppOrJs,
    ArchiveFileTypeLoading,
} ArchiveFileTypeEnum;

typedef struct {
    FuriString* path;
    ArchiveFileTypeEnum type;
    uint8_t* custom_icon_data;
    FuriString* custom_name;
    bool fav;
    bool is_app;
} ArchiveFile_t;

static void ArchiveFile_t_init(ArchiveFile_t* obj) {
    obj->path = furi_string_alloc();
    obj->type = ArchiveFileTypeUnknown;
    obj->custom_icon_data = NULL;
    obj->custom_name = furi_string_alloc();
    obj->fav = false;
    obj->is_app = false;
}

static void ArchiveFile_t_init_set(ArchiveFile_t* obj, const ArchiveFile_t* src) {
    obj->path = furi_string_alloc_set(src->path);
    obj->type = src->type;
    if(src->custom_icon_data) {
        obj->custom_icon_data = malloc(FAP_MANIFEST_MAX_ICON_SIZE);
        memcpy(obj->custom_icon_data, src->custom_icon_data, FAP_MANIFEST_MAX_ICON_SIZE);
    } else {
        obj->custom_icon_data = NULL;
    }
    obj->custom_name = furi_string_alloc_set(src->custom_name);
    obj->fav = src->fav;
    obj->is_app = src->is_app;
}

static void ArchiveFile_t_set(ArchiveFile_t* obj, const ArchiveFile_t* src) {
    furi_string_set(obj->path, src->path);
    obj->type = src->type;
    if(src->custom_icon_data) {
        obj->custom_icon_data = malloc(FAP_MANIFEST_MAX_ICON_SIZE);
        memcpy(obj->custom_icon_data, src->custom_icon_data, FAP_MANIFEST_MAX_ICON_SIZE);
    } else {
        obj->custom_icon_data = NULL;
    }
    furi_string_set(obj->custom_name, src->custom_name);
    obj->fav = src->fav;
    obj->is_app = src->is_app;
}

static void ArchiveFile_t_clear(ArchiveFile_t* obj) {
    furi_string_free(obj->path);
    if(obj->custom_icon_data) {
        free(obj->custom_icon_data);
        obj->custom_icon_data = NULL;
    }
    furi_string_free(obj->custom_name);
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
void archive_file_append(const char* path, const char* format, ...)
    _ATTRIBUTE((__format__(__printf__, 2, 3)));
void archive_delete_file(void* context, const char* format, ...)
    _ATTRIBUTE((__format__(__printf__, 2, 3)));
