#pragma once

#include "archive.h"
#include <stdint.h>
#include <furi.h>
#include <gui/gui_i.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>
#include <loader/loader.h>

#include <m-string.h>
#include <m-array.h>
#include <filesystem-api.h>
#include "archive_views.h"
#include "applications.h"

#define MAX_DEPTH 32
#define MAX_FILES 100 //temp
#define MAX_FILE_SIZE 128

typedef enum {
    ArchiveViewMain,
    ArchiveViewTextInput,
    ArchiveViewTotal,
} ArchiveViewEnum;

static const char* flipper_app_name[] = {
    [ArchiveFileTypeIButton] = "iButton",
    [ArchiveFileTypeNFC] = "NFC",
    [ArchiveFileTypeSubOne] = "Sub-1 GHz",
    [ArchiveFileTypeLFRFID] = "125 kHz RFID",
    [ArchiveFileTypeIrda] = "Infrared",
};

static const char* known_ext[] = {
    [ArchiveFileTypeIButton] = ".ibtn",
    [ArchiveFileTypeNFC] = ".nfc",
    [ArchiveFileTypeSubOne] = ".sub1",
    [ArchiveFileTypeLFRFID] = ".rfid",
    [ArchiveFileTypeIrda] = ".ir",
};

static const char* tab_default_paths[] = {
    [ArchiveTabFavorites] = "favorites",
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

static inline const char* get_default_path(ArchiveFileTypeEnum type) {
    switch(type) {
    case ArchiveFileTypeIButton:
        return tab_default_paths[ArchiveTabIButton];
    case ArchiveFileTypeNFC:
        return tab_default_paths[ArchiveTabNFC];
    case ArchiveFileTypeSubOne:
        return tab_default_paths[ArchiveTabSubOne];
    case ArchiveFileTypeLFRFID:
        return tab_default_paths[ArchiveTabLFRFID];
    case ArchiveFileTypeIrda:
        return tab_default_paths[ArchiveTabIrda];
    default:
        return false;
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
    char text_input_buffer[MAX_NAME_LEN];

    uint8_t depth;
    uint16_t last_idx[MAX_DEPTH];

    bool menu;
} ArchiveBrowser;

struct ArchiveApp {
    osMessageQueueId_t event_queue;
    FuriThread* app_thread;
    Loader* loader;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    View* view_archive_main;
    TextInput* text_input;

    FS_Api* fs_api;
    ArchiveBrowser browser;
};
