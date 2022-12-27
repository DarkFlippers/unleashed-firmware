#pragma once

#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>
#include <storage/storage.h>
#include "../helpers/archive_files.h"
#include "../helpers/archive_menu.h"
#include "../helpers/archive_favorites.h"
#include "gui/modules/file_browser_worker.h"

#define MAX_LEN_PX 110
#define MAX_NAME_LEN 255
#define MAX_EXT_LEN 6
#define FRAME_HEIGHT 12
#define MENU_ITEMS 5u
#define MOVE_OFFSET 5u

typedef enum {
    ArchiveTabFavorites,
    ArchiveTabSubGhz,
    ArchiveTabLFRFID,
    ArchiveTabNFC,
    ArchiveTabInfrared,
    ArchiveTabIButton,
    ArchiveTabBadUsb,
    ArchiveTabU2f,
    ArchiveTabApplications,
    ArchiveTabBrowser,
    ArchiveTabTotal,
} ArchiveTabEnum;

typedef enum {
    ArchiveBrowserEventFileMenuNone,
    ArchiveBrowserEventFileMenuOpen,
    ArchiveBrowserEventFileMenuRun,
    ArchiveBrowserEventFileMenuPin,
    ArchiveBrowserEventFileMenuRename,
    ArchiveBrowserEventFileMenuDelete,
    ArchiveBrowserEventFileMenuInfo,
    ArchiveBrowserEventFileMenuShow,
    ArchiveBrowserEventFileMenuClose,

    ArchiveBrowserEventEnterDir,

    ArchiveBrowserEventFavMoveUp,
    ArchiveBrowserEventFavMoveDown,
    ArchiveBrowserEventEnterFavMove,
    ArchiveBrowserEventExitFavMove,
    ArchiveBrowserEventSaveFavMove,

    ArchiveBrowserEventLoadPrevItems,
    ArchiveBrowserEventLoadNextItems,

    ArchiveBrowserEventListRefresh,

    ArchiveBrowserEventExit,
} ArchiveBrowserEvent;

typedef struct ArchiveBrowserView ArchiveBrowserView;

typedef void (*ArchiveBrowserViewCallback)(ArchiveBrowserEvent event, void* context);

typedef enum {
    BrowserActionBrowse,
    BrowserActionItemMenu,
    BrowserActionTotal,
} BrowserActionEnum;

struct ArchiveBrowserView {
    View* view;
    BrowserWorker* worker;
    bool worker_running;
    ArchiveBrowserViewCallback callback;
    void* context;
    FuriString* path;
    InputKey last_tab_switch_dir;
    bool is_root;
    FuriTimer* scroll_timer;
};

typedef struct {
    ArchiveTabEnum tab_idx;
    files_array_t files;

    uint8_t menu_idx;
    bool menu;
    menu_array_t context_menu;

    bool move_fav;
    bool list_loading;
    bool folder_loading;

    uint32_t item_cnt;
    int32_t item_idx;
    int32_t array_offset;
    int32_t list_offset;
    size_t scroll_counter;
} ArchiveBrowserViewModel;

void archive_browser_set_callback(
    ArchiveBrowserView* browser,
    ArchiveBrowserViewCallback callback,
    void* context);

View* archive_browser_get_view(ArchiveBrowserView* browser);

ArchiveBrowserView* browser_alloc();

void browser_free(ArchiveBrowserView* browser);
