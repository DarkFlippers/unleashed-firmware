#pragma once

#include <m-array.h>
#include <furi.h>
#include <m-algo.h>
#include <storage/storage.h>
#include "toolbox/path.h"

#include <flipper_application/application_manifest.h> // FAP_MANIFEST_MAX_ICON_SIZE

typedef enum {
    ArchiveFileTypeIButton,
    ArchiveFileTypeNFC,
    ArchiveFileTypeSubGhz,
    ArchiveFileTypeSubGhzRemote,
    ArchiveFileTypeLFRFID,
    ArchiveFileTypeInfrared,
    ArchiveFileTypeBadUsb,
    ArchiveFileTypeU2f,
    ArchiveFileTypeT5577,
    ArchiveFileTypeApplication,
    ArchiveFileTypeUpdateManifest,
    ArchiveFileTypeJS,
    ArchiveFileTypeFolder,
    ArchiveFileTypeUnknown,
    ArchiveFileTypeSetting,
    ArchiveFileTypeLoading,
} ArchiveFileTypeEnum;

/** Types the "Show" item can open, which is a text viewer: the ones above it are binary.
 *
 * A predicate rather than the bare comparison it replaces, so that where a type sits in the
 * enum stops being what decides whether a file can be shown as text.
 */
static inline bool archive_file_type_is_showable(ArchiveFileTypeEnum type) {
    return type <= ArchiveFileTypeBadUsb;
}

typedef struct {
    FuriString* path;
    ArchiveFileTypeEnum type;
    uint8_t* custom_icon_data;
    FuriString* custom_name;
    bool fav;
    bool is_app;
    bool is_text_file;
} ArchiveFile_t;

static void ArchiveFile_t_init(ArchiveFile_t* obj) {
    obj->path = furi_string_alloc();
    obj->type = ArchiveFileTypeUnknown;
    obj->custom_icon_data = NULL;
    obj->custom_name = furi_string_alloc();
    obj->fav = false;
    obj->is_app = false;
    obj->is_text_file = false;
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
    obj->is_text_file = src->is_text_file;
}

static void ArchiveFile_t_set(ArchiveFile_t* obj, const ArchiveFile_t* src) {
    furi_string_set(obj->path, src->path);
    obj->type = src->type;
    // obj already holds an icon of its own: assigning over it without this leaks the buffer
    if(obj->custom_icon_data) {
        free(obj->custom_icon_data);
        obj->custom_icon_data = NULL;
    }
    if(src->custom_icon_data) {
        obj->custom_icon_data = malloc(FAP_MANIFEST_MAX_ICON_SIZE);
        memcpy(obj->custom_icon_data, src->custom_icon_data, FAP_MANIFEST_MAX_ICON_SIZE);
    } else {
        obj->custom_icon_data = NULL;
    }
    furi_string_set(obj->custom_name, src->custom_name);
    obj->fav = src->fav;
    obj->is_app = src->is_app;
    obj->is_text_file = src->is_text_file;
}

static void ArchiveFile_t_clear(ArchiveFile_t* obj) {
    furi_string_free(obj->path);
    if(obj->custom_icon_data) {
        free(obj->custom_icon_data);
        obj->custom_icon_data = NULL;
    }
    furi_string_free(obj->custom_name);
}

// ASCII case-fold compare, same order as furi_string_cmpi without its 4 libc calls per char
static inline int archive_file_cmpi(const char* a, const char* b) {
    unsigned char ca, cb;
    do {
        ca = (unsigned char)*a++;
        cb = (unsigned char)*b++;
        if((ca >= 'A') && (ca <= 'Z')) ca += 'a' - 'A';
        if((cb >= 'A') && (cb <= 'Z')) cb += 'a' - 'A';
    } while((ca == cb) && (ca != '\0'));
    return (int)ca - (int)cb;
}

static int ArchiveFile_t_cmp(const ArchiveFile_t* a, const ArchiveFile_t* b) {
    if(a->type == ArchiveFileTypeFolder && b->type != ArchiveFileTypeFolder) {
        return -1;
    }
    if(a->type != ArchiveFileTypeFolder && b->type == ArchiveFileTypeFolder) {
        return 1;
    }

    return archive_file_cmpi(furi_string_get_cstr(a->path), furi_string_get_cstr(b->path));
}

#define M_OPL_ArchiveFile_t()                 \
    (INIT(API_2(ArchiveFile_t_init)),         \
     SET(API_6(ArchiveFile_t_set)),           \
     INIT_SET(API_6(ArchiveFile_t_init_set)), \
     CLEAR(API_2(ArchiveFile_t_clear)),       \
     CMP(API_6(ArchiveFile_t_cmp)),           \
     SWAP(M_SWAP_DEFAULT),                    \
     EQUAL(API_6(M_EQUAL_DEFAULT)))

ARRAY_DEF(files_array, ArchiveFile_t)

ALGO_DEF(files_array, ARRAY_OPLIST(files_array, M_OPL_ArchiveFile_t()))

void archive_set_file_type(ArchiveFile_t* file, const char* path, bool is_folder, bool is_app);
bool archive_get_items(void* context, const char* path);
void archive_file_append(const char* path, const char* format, ...)
    _ATTRIBUTE((__format__(__printf__, 2, 3)));
void archive_delete_file(void* context, const char* format, ...)
    _ATTRIBUTE((__format__(__printf__, 2, 3)));
FS_Error archive_rename_copy_file_or_dir(
    void* context,
    const char* src_path,
    const char* dst_path,
    bool copy);
