#pragma once

#include "../archive_i.h"
#include <storage/storage.h>

#define TAB_RIGHT InputKeyRight // Default tab swith direction
#define TAB_DEFAULT ArchiveTabFavorites // Start tab
#define FILE_LIST_BUF_LEN 100

static const char* tab_default_paths[] = {
    [ArchiveTabFavorites] = "/app:favorites",
    [ArchiveTabIButton] = ANY_PATH("ibutton"),
    [ArchiveTabNFC] = ANY_PATH("nfc"),
    [ArchiveTabSubGhz] = ANY_PATH("subghz"),
    [ArchiveTabLFRFID] = ANY_PATH("lfrfid"),
    [ArchiveTabInfrared] = ANY_PATH("infrared"),
    [ArchiveTabBadUsb] = ANY_PATH("badusb"),
    [ArchiveTabU2f] = "/app:u2f",
    [ArchiveTabBrowser] = STORAGE_ANY_PATH_PREFIX,
};

static const char* known_ext[] = {
    [ArchiveFileTypeIButton] = ".ibtn",
    [ArchiveFileTypeNFC] = ".nfc",
    [ArchiveFileTypeSubGhz] = ".sub",
    [ArchiveFileTypeLFRFID] = ".rfid",
    [ArchiveFileTypeInfrared] = ".ir",
    [ArchiveFileTypeBadUsb] = ".txt",
    [ArchiveFileTypeU2f] = "?",
    [ArchiveFileTypeUpdateManifest] = ".fuf",
    [ArchiveFileTypeFolder] = "?",
    [ArchiveFileTypeUnknown] = "*",
};

static const ArchiveFileTypeEnum known_type[] = {
    [ArchiveTabFavorites] = ArchiveFileTypeUnknown,
    [ArchiveTabIButton] = ArchiveFileTypeIButton,
    [ArchiveTabNFC] = ArchiveFileTypeNFC,
    [ArchiveTabSubGhz] = ArchiveFileTypeSubGhz,
    [ArchiveTabLFRFID] = ArchiveFileTypeLFRFID,
    [ArchiveTabInfrared] = ArchiveFileTypeInfrared,
    [ArchiveTabBadUsb] = ArchiveFileTypeBadUsb,
    [ArchiveTabU2f] = ArchiveFileTypeU2f,
    [ArchiveTabBrowser] = ArchiveFileTypeUnknown,
};

static inline ArchiveFileTypeEnum archive_get_tab_filetype(ArchiveTabEnum tab) {
    return known_type[tab];
}

static inline const char* archive_get_tab_ext(ArchiveTabEnum tab) {
    return known_ext[archive_get_tab_filetype(tab)];
}

static inline const char* archive_get_default_path(ArchiveTabEnum tab) {
    return tab_default_paths[tab];
}

inline bool archive_is_known_app(ArchiveFileTypeEnum type) {
    return (type != ArchiveFileTypeFolder && type != ArchiveFileTypeUnknown);
}

bool archive_is_item_in_array(ArchiveBrowserViewModel* model, uint32_t idx);
void archive_update_offset(ArchiveBrowserView* browser);
void archive_update_focus(ArchiveBrowserView* browser, const char* target);

void archive_file_array_load(ArchiveBrowserView* browser, int8_t dir);
size_t archive_file_get_array_size(ArchiveBrowserView* browser);
void archive_file_array_rm_selected(ArchiveBrowserView* browser);
void archive_file_array_swap(ArchiveBrowserView* browser, int8_t dir);
void archive_file_array_rm_all(ArchiveBrowserView* browser);

void archive_set_item_count(ArchiveBrowserView* browser, uint32_t count);

ArchiveFile_t* archive_get_current_file(ArchiveBrowserView* browser);
ArchiveFile_t* archive_get_file_at(ArchiveBrowserView* browser, size_t idx);
ArchiveTabEnum archive_get_tab(ArchiveBrowserView* browser);
bool archive_is_home(ArchiveBrowserView* browser);
const char* archive_get_name(ArchiveBrowserView* browser);

void archive_add_app_item(ArchiveBrowserView* browser, const char* name);
void archive_add_file_item(ArchiveBrowserView* browser, bool is_folder, const char* name);
void archive_show_file_menu(ArchiveBrowserView* browser, bool show);
void archive_favorites_move_mode(ArchiveBrowserView* browser, bool active);

void archive_switch_tab(ArchiveBrowserView* browser, InputKey key);
void archive_enter_dir(ArchiveBrowserView* browser, string_t name);
void archive_leave_dir(ArchiveBrowserView* browser);
void archive_refresh_dir(ArchiveBrowserView* browser);
