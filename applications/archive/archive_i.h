#pragma once

#include "archive.h"
#include <stdint.h>
#include <furi.h>
#include <gui/gui_i.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_input.h>
#include <loader/loader.h>

#include <m-string.h>
#include <m-array.h>
#include <storage/storage.h>
#include "applications.h"
#include "file-worker.h"

#include "views/archive_main_view.h"
#include "scenes/archive_scene.h"

#define MAX_FILE_SIZE 128

typedef enum {
    ArchiveViewBrowser,
    ArchiveViewTextInput,
    ArchiveViewTotal,
} ArchiveViewEnum;

static const char* tab_default_paths[] = {
    [ArchiveTabFavorites] = "/any/favorites",
    [ArchiveTabIButton] = "/any/ibutton",
    [ArchiveTabNFC] = "/any/nfc",
    [ArchiveTabSubGhz] = "/any/subghz/saved",
    [ArchiveTabLFRFID] = "/any/lfrfid",
    [ArchiveTabIrda] = "/any/irda",
    [ArchiveTabBrowser] = "/any",
};

static inline const char* get_default_path(ArchiveFileTypeEnum type) {
    switch(type) {
    case ArchiveFileTypeIButton:
        return tab_default_paths[ArchiveTabIButton];
    case ArchiveFileTypeNFC:
        return tab_default_paths[ArchiveTabNFC];
    case ArchiveFileTypeSubGhz:
        return tab_default_paths[ArchiveTabSubGhz];
    case ArchiveFileTypeLFRFID:
        return tab_default_paths[ArchiveTabLFRFID];
    case ArchiveFileTypeIrda:
        return tab_default_paths[ArchiveTabIrda];
    default:
        return false;
    }
}

static inline const char* get_favorites_path() {
    return tab_default_paths[ArchiveTabFavorites];
}

typedef enum {
    EventTypeTick,
    EventTypeKey,
    EventTypeExit,
} EventType;

typedef struct {
    union {
        InputEvent input;
    } value;
    EventType type;
} AppEvent;

struct ArchiveApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    ArchiveMainView* main_view;
    TextInput* text_input;
    char text_store[MAX_NAME_LEN];
};
