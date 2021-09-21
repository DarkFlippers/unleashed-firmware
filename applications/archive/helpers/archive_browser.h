#pragma once

#include "../archive_i.h"

#define DEFAULT_TAB_DIR InputKeyRight //default tab swith direction

static const char* tab_default_paths[] = {
    [ArchiveTabFavorites] = "/any/favorites",
    [ArchiveTabIButton] = "/any/ibutton",
    [ArchiveTabNFC] = "/any/nfc",
    [ArchiveTabSubGhz] = "/any/subghz/saved",
    [ArchiveTabLFRFID] = "/any/lfrfid",
    [ArchiveTabIrda] = "/any/irda",
    [ArchiveTabBrowser] = "/any",
};

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

static inline const char* archive_get_default_path(ArchiveTabEnum tab) {
    return tab_default_paths[tab];
}

inline bool is_known_app(ArchiveFileTypeEnum type) {
    return (type != ArchiveFileTypeFolder && type != ArchiveFileTypeUnknown);
}

void archive_update_offset(ArchiveBrowserView* browser);
void archive_update_focus(ArchiveBrowserView* browser, const char* target);

size_t archive_file_array_size(ArchiveBrowserView* browser);
void archive_file_array_rm_selected(ArchiveBrowserView* browser);
void archive_file_array_rm_all(ArchiveBrowserView* browser);

ArchiveFile_t* archive_get_current_file(ArchiveBrowserView* browser);
ArchiveTabEnum archive_get_tab(ArchiveBrowserView* browser);
uint8_t archive_get_depth(ArchiveBrowserView* browser);
const char* archive_get_path(ArchiveBrowserView* browser);
const char* archive_get_name(ArchiveBrowserView* browser);

void archive_add_item(ArchiveBrowserView* browser, FileInfo* file_info, const char* name);
void archive_show_file_menu(ArchiveBrowserView* browser, bool show);

void archive_switch_tab(ArchiveBrowserView* browser, InputKey key);
void archive_enter_dir(ArchiveBrowserView* browser, string_t name);
void archive_leave_dir(ArchiveBrowserView* browser);
