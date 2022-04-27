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
#define MAX_EXT_LEN 6
#define FRAME_HEIGHT 12
#define MENU_ITEMS 4
#define MOVE_OFFSET 5

typedef enum {
    ArchiveTabFavorites,
    ArchiveTabSubGhz,
    ArchiveTabLFRFID,
    ArchiveTabNFC,
    ArchiveTabInfrared,
    ArchiveTabIButton,
    ArchiveTabBadUsb,
    ArchiveTabU2f,
    ArchiveTabBrowser,
    ArchiveTabTotal,
} ArchiveTabEnum;

typedef enum {
    ArchiveBrowserEventFileMenuOpen,
    ArchiveBrowserEventFileMenuClose,
    ArchiveBrowserEventFileMenuRun,
    ArchiveBrowserEventFileMenuPin,
    ArchiveBrowserEventFileMenuAction,
    ArchiveBrowserEventFileMenuDelete,

    ArchiveBrowserEventEnterDir,

    ArchiveBrowserEventFavMoveUp,
    ArchiveBrowserEventFavMoveDown,
    ArchiveBrowserEventEnterFavMove,
    ArchiveBrowserEventExitFavMove,
    ArchiveBrowserEventSaveFavMove,

    ArchiveBrowserEventLoadPrevItems,
    ArchiveBrowserEventLoadNextItems,

    ArchiveBrowserEventLoaderAppExit,

    ArchiveBrowserEventExit,
} ArchiveBrowserEvent;

static const uint8_t file_menu_actions[MENU_ITEMS] = {
    [0] = ArchiveBrowserEventFileMenuRun,
    [1] = ArchiveBrowserEventFileMenuPin,
    [2] = ArchiveBrowserEventFileMenuAction,
    [3] = ArchiveBrowserEventFileMenuDelete,
};

typedef struct ArchiveBrowserView ArchiveBrowserView;

typedef void (*ArchiveBrowserViewCallback)(ArchiveBrowserEvent event, void* context);

typedef enum {
    BrowserActionBrowse,
    BrowserActionItemMenu,
    BrowserActionTotal,
} BrowserActionEnum;

struct ArchiveBrowserView {
    View* view;
    ArchiveBrowserViewCallback callback;
    void* context;
    string_t path;
};

ARRAY_DEF(idx_last_array, int32_t)

typedef struct {
    ArchiveTabEnum tab_idx;
    ArchiveTabEnum last_tab;
    files_array_t files;
    idx_last_array_t idx_last;

    uint8_t menu_idx;
    bool move_fav;
    bool menu;
    bool list_loading;

    uint32_t item_cnt;
    int32_t item_idx;
    int32_t array_offset;
    int32_t list_offset;
} ArchiveBrowserViewModel;

void archive_browser_set_callback(
    ArchiveBrowserView* browser,
    ArchiveBrowserViewCallback callback,
    void* context);

View* archive_browser_get_view(ArchiveBrowserView* browser);

ArchiveBrowserView* browser_alloc();
void browser_free(ArchiveBrowserView* browser);
