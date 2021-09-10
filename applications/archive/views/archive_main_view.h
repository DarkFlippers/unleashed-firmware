#pragma once

#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>
#include <storage/storage.h>
#include "../helpers/archive_files.h"
#include "../helpers/archive_favorites.h"

#define MAX_LEN_PX 110
#define MAX_NAME_LEN 255
#define FRAME_HEIGHT 12
#define MENU_ITEMS 4
#define MAX_DEPTH 32

typedef enum {
    ArchiveTabFavorites,
    ArchiveTabLFRFID,
    ArchiveTabSubGhz,
    ArchiveTabNFC,
    ArchiveTabIButton,
    ArchiveTabIrda,
    ArchiveTabBrowser,
    ArchiveTabTotal,
} ArchiveTabEnum;

static const char* known_ext[] = {
    [ArchiveFileTypeIButton] = ".ibtn",
    [ArchiveFileTypeNFC] = ".nfc",
    [ArchiveFileTypeSubGhz] = ".sub",
    [ArchiveFileTypeLFRFID] = ".rfid",
    [ArchiveFileTypeIrda] = ".ir",
};

static inline const char* get_tab_ext(ArchiveTabEnum tab) {
    switch(tab) {
    case ArchiveTabIButton:
        return known_ext[ArchiveFileTypeIButton];
    case ArchiveTabNFC:
        return known_ext[ArchiveFileTypeNFC];
    case ArchiveTabSubGhz:
        return known_ext[ArchiveFileTypeSubGhz];
    case ArchiveTabLFRFID:
        return known_ext[ArchiveFileTypeLFRFID];
    case ArchiveTabIrda:
        return known_ext[ArchiveFileTypeIrda];
    default:
        return "*";
    }
}

typedef enum {
    ArchiveBrowserEventRename,
    ArchiveBrowserEventExit,
    ArchiveBrowserEventLeaveDir,
} ArchiveBrowserEvent;

typedef struct ArchiveMainView ArchiveMainView;

typedef void (*ArchiveMainViewCallback)(ArchiveBrowserEvent event, void* context);

typedef enum {
    BrowserActionBrowse,
    BrowserActionItemMenu,
    BrowserActionTotal,
} BrowserActionEnum;

struct ArchiveMainView {
    View* view;
    ArchiveMainViewCallback callback;
    void* context;

    string_t name;
    string_t path;
};

typedef struct {
    ArchiveTabEnum tab_idx;
    BrowserActionEnum action;
    files_array_t files;

    uint8_t depth;
    uint8_t menu_idx;

    uint16_t idx;
    uint16_t last_idx[MAX_DEPTH];
    uint16_t list_offset;

} ArchiveMainViewModel;

void archive_browser_set_callback(
    ArchiveMainView* main_view,
    ArchiveMainViewCallback callback,
    void* context);

View* archive_main_get_view(ArchiveMainView* main_view);

ArchiveMainView* main_view_alloc();
void main_view_free(ArchiveMainView* main_view);

void archive_file_array_remove_selected(ArchiveMainView* main_view);
void archive_file_array_clean(ArchiveMainView* main_view);

void archive_view_add_item(ArchiveMainView* main_view, FileInfo* file_info, const char* name);
void archive_browser_update(ArchiveMainView* main_view);

size_t archive_file_array_size(ArchiveMainView* main_view);
ArchiveFile_t* archive_get_current_file(ArchiveMainView* main_view);
const char* archive_get_path(ArchiveMainView* main_view);
const char* archive_get_name(ArchiveMainView* main_view);
void archive_set_name(ArchiveMainView* main_view, const char* name);

static inline bool is_known_app(ArchiveFileTypeEnum type) {
    return (type != ArchiveFileTypeFolder && type != ArchiveFileTypeUnknown);
}
