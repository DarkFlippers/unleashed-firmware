#pragma once

#include <gui/gui_i.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>
#include <filesystem-api.h>

#define MAX_LEN_PX 98
#define FRAME_HEIGHT 12
#define MENU_ITEMS 4

typedef enum {
    ArchiveFileTypeIButton,
    ArchiveFileTypeNFC,
    ArchiveFileTypeSubOne,
    ArchiveFileTypeLFRFID,
    ArchiveFileTypeIrda,
    ArchiveFileTypeFolder,
    ArchiveFileTypeUnknown,
    AppIdTotal,
} ArchiveFileTypeEnum;

typedef struct {
    string_t name;
    ArchiveFileTypeEnum type;
} ArchiveFile_t;

static void ArchiveFile_t_init(ArchiveFile_t* obj) {
    obj->type = ArchiveFileTypeUnknown;
    string_init(obj->name);
}

static void ArchiveFile_t_init_set(ArchiveFile_t* obj, const ArchiveFile_t* src) {
    obj->type = src->type;
    string_init_set(obj->name, src->name);
}

static void ArchiveFile_t_set(ArchiveFile_t* obj, const ArchiveFile_t* src) {
    obj->type = src->type;
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

typedef struct {
    uint8_t tab_idx;
    uint8_t menu_idx;
    uint16_t idx;
    uint16_t list_offset;
    files_array_t files;
    bool menu;
} ArchiveViewModel;

void archive_view_render(Canvas* canvas, void* model);
