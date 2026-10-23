#pragma once

#include "../archive_i.h"
#include <storage/storage.h>
#include <lfrfid/tools/t5577_file.h> // the dump extension and the app that opens one

#define TAB_LEFT          InputKeyLeft // Default tab switch direction
#define TAB_DEFAULT       ArchiveTabFavorites // Start tab
#define FILE_LIST_BUF_LEN 50

static const char* const tab_default_paths[] = {
    [ArchiveTabFavorites] = "/app:favorites",
    [ArchiveTabIButton] = EXT_PATH("ibutton"),
    [ArchiveTabNFC] = EXT_PATH("nfc"),
    [ArchiveTabSubGhz] = EXT_PATH("subghz"),
    [ArchiveTabSubGhzRemote] = EXT_PATH("subghz_remote"),
    [ArchiveTabLFRFID] = EXT_PATH("lfrfid"),
    [ArchiveTabInfrared] = EXT_PATH("infrared"),
    [ArchiveTabBadUsb] = EXT_PATH("badusb"),
    [ArchiveTabU2f] = "/app:u2f",
    [ArchiveTabApplications] = EXT_PATH("apps"),
    [ArchiveTabInternal] = STORAGE_INT_PATH_PREFIX,
    [ArchiveTabBrowser] = STORAGE_EXT_PATH_PREFIX,
};

static const char* const known_ext[] = {
    [ArchiveFileTypeIButton] = ".ibtn",
    [ArchiveFileTypeNFC] = ".nfc",
    [ArchiveFileTypeSubGhz] = ".sub",
    [ArchiveFileTypeSubGhzRemote] = ".txt",
    [ArchiveFileTypeLFRFID] = ".rfid",
    [ArchiveFileTypeInfrared] = ".ir",
    [ArchiveFileTypeBadUsb] = ".txt",
    [ArchiveFileTypeU2f] = "?",
    [ArchiveFileTypeT5577] = T5577_FILE_EXTENSION,
    [ArchiveFileTypeApplication] = ".fap",
    [ArchiveFileTypeJS] = ".js",
    [ArchiveFileTypeUpdateManifest] = ".fuf",
    [ArchiveFileTypeFolder] = "?",
    [ArchiveFileTypeUnknown] = "*",
    [ArchiveFileTypeSetting] = "?",
};

// What each tab lists, which is not the same question as what type a file is: a tab can hold
// more than one kind, and "*" is every kind. A filter with a "|" in it is nothing any file is,
// so these cannot live in known_ext, and C cannot index that table from here either - so an
// extension a tab shares with a file type is written twice. Keep the two in step.
static const char* const tab_ext[] = {
    [ArchiveTabFavorites] = "*",
    [ArchiveTabIButton] = ".ibtn",
    [ArchiveTabNFC] = ".nfc",
    [ArchiveTabSubGhz] = ".sub",
    [ArchiveTabSubGhzRemote] = ".txt",
    [ArchiveTabLFRFID] = ".rfid|" T5577_FILE_EXTENSION, // the keys, and the dumps beside them
    [ArchiveTabInfrared] = ".ir",
    [ArchiveTabBadUsb] = ".txt",
    [ArchiveTabU2f] = "?",
    [ArchiveTabApplications] = ".fap|.js",
    [ArchiveTabInternal] = "*",
    [ArchiveTabBrowser] = "*",
};

static inline const char* archive_get_tab_ext(ArchiveTabEnum tab) {
    return tab_ext[tab];
}

static inline const char* archive_get_default_path(ArchiveTabEnum tab) {
    return tab_default_paths[tab];
}

inline bool archive_is_known_app(ArchiveFileTypeEnum type) {
    return (type != ArchiveFileTypeUnknown);
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
void archive_enter_dir(ArchiveBrowserView* browser, FuriString* name);
void archive_leave_dir(ArchiveBrowserView* browser);
void archive_refresh_dir(ArchiveBrowserView* browser);
