#pragma once

#include "archive.h"
#include <stdint.h>
#include <furi.h>
#include <gui/gui_i.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>

#include <m-string.h>
#include <m-array.h>
#include <filesystem-api.h>
#include "archive_views.h"
#include "applications.h"

#define MAX_DEPTH 32
#define MAX_NAME_LEN 255

typedef enum {
    ArchiveViewMain,
    ArchiveViewTextInput,
    ArchiveViewTotal,
} ArchiveViewEnum;

typedef enum {
    ArchiveTabFavourites,
    ArchiveTabIButton,
    ArchiveTabNFC,
    ArchiveTabSubOne,
    ArchiveTabLFRFID,
    ArchiveTabIrda,
    ArchiveTabBrowser,
    ArchiveTabTotal,
} ArchiveTabEnum;

static const char* known_ext[] = {
    [ArchiveFileTypeIButton] = ".ibtn",
    [ArchiveFileTypeNFC] = ".nfc",
    [ArchiveFileTypeSubOne] = ".sub1",
    [ArchiveFileTypeLFRFID] = ".rfid",
    [ArchiveFileTypeIrda] = ".irda",
};

static const char* tab_default_paths[] = {
    [ArchiveTabFavourites] = "favourites",
    [ArchiveTabIButton] = "ibutton",
    [ArchiveTabNFC] = "nfc",
    [ArchiveTabSubOne] = "subone",
    [ArchiveTabLFRFID] = "lfrfid",
    [ArchiveTabIrda] = "irda",
    [ArchiveTabBrowser] = "/",
};

static inline const char* get_tab_ext(ArchiveTabEnum tab) {
    switch(tab) {
    case ArchiveTabIButton:
        return known_ext[ArchiveFileTypeIButton];
    case ArchiveTabNFC:
        return known_ext[ArchiveFileTypeNFC];
    case ArchiveTabSubOne:
        return known_ext[ArchiveFileTypeSubOne];
    case ArchiveTabLFRFID:
        return known_ext[ArchiveFileTypeLFRFID];
    case ArchiveTabIrda:
        return known_ext[ArchiveFileTypeIrda];
    default:
        return "*";
    }
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

typedef struct {
    ArchiveTabEnum tab_id;
    string_t name;
    string_t path;
    string_t text_input_buffer;

    uint8_t depth;
    uint16_t last_idx[MAX_DEPTH];

    bool menu;
} ArchiveBrowser;

struct ArchiveApp {
    osMessageQueueId_t event_queue;
    FuriThread* app_thread;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    View* view_archive_main;
    TextInput* text_input;

    FS_Api* fs_api;
    ArchiveBrowser browser;
};
