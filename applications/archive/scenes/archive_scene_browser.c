#include "../archive_i.h"
#include "../helpers/archive_files.h"
#include "../helpers/archive_apps.h"
#include "../helpers/archive_favorites.h"
#include "../helpers/archive_browser.h"
#include "../views/archive_browser_view.h"

#define TAG "ArchiveSceneBrowser"

static const char* flipper_app_name[] = {
    [ArchiveFileTypeIButton] = "iButton",
    [ArchiveFileTypeNFC] = "NFC",
    [ArchiveFileTypeSubGhz] = "Sub-GHz",
    [ArchiveFileTypeLFRFID] = "125 kHz RFID",
    [ArchiveFileTypeInfrared] = "Infrared",
    [ArchiveFileTypeBadUsb] = "Bad USB",
    [ArchiveFileTypeU2f] = "U2F",
    [ArchiveFileTypeUpdateManifest] = "UpdaterApp",
};

static void archive_run_in_app(ArchiveBrowserView* browser, ArchiveFile_t* selected) {
    Loader* loader = furi_record_open("loader");

    LoaderStatus status;
    if(selected->is_app) {
        char* param = strrchr(string_get_cstr(selected->name), '/');
        if(param != NULL) {
            param++;
        }
        status = loader_start(loader, flipper_app_name[selected->type], param);
    } else {
        status = loader_start(
            loader, flipper_app_name[selected->type], string_get_cstr(selected->name));
    }

    if(status != LoaderStatusOk) {
        FURI_LOG_E(TAG, "loader_start failed: %d", status);
    }

    furi_record_close("loader");
}

void archive_scene_browser_callback(ArchiveBrowserEvent event, void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;
    view_dispatcher_send_custom_event(archive->view_dispatcher, event);
}

void archive_scene_browser_on_enter(void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;
    ArchiveBrowserView* browser = archive->browser;

    archive_browser_set_callback(browser, archive_scene_browser_callback, archive);
    archive_update_focus(browser, archive->text_store);
    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewBrowser);
}

bool archive_scene_browser_on_event(void* context, SceneManagerEvent event) {
    ArchiveApp* archive = (ArchiveApp*)context;
    ArchiveBrowserView* browser = archive->browser;
    ArchiveFile_t* selected = archive_get_current_file(browser);

    const char* name = archive_get_name(browser);
    bool known_app = archive_is_known_app(selected->type);
    bool favorites = archive_get_tab(browser) == ArchiveTabFavorites;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case ArchiveBrowserEventFileMenuOpen:
            archive_show_file_menu(browser, true);
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuClose:
            archive_show_file_menu(browser, false);
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuRun:
            if(known_app) {
                archive_run_in_app(browser, selected);
            }
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuPin:
            if(favorites) {
                archive_favorites_delete(name);
                archive_file_array_rm_selected(browser);
                archive_show_file_menu(browser, false);
            } else if(known_app) {
                if(archive_is_favorite("%s", name)) {
                    archive_favorites_delete("%s", name);
                } else {
                    archive_file_append(ARCHIVE_FAV_PATH, "%s\n", name);
                }
                archive_show_file_menu(browser, false);
            }
            consumed = true;
            break;

        case ArchiveBrowserEventFileMenuAction:
            if(favorites) {
                browser->callback(ArchiveBrowserEventEnterFavMove, browser->context);
            } else if((known_app) && (selected->is_app == false)) {
                archive_show_file_menu(browser, false);
                scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneRename);
            }
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuDelete:
            if(archive_get_tab(browser) != ArchiveTabFavorites) {
                scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneDelete);
            }
            consumed = true;
            break;
        case ArchiveBrowserEventEnterDir:
            archive_enter_dir(browser, selected->name);
            consumed = true;
            break;
        case ArchiveBrowserEventFavMoveUp:
            archive_file_array_swap(browser, 1);
            consumed = true;
            break;
        case ArchiveBrowserEventFavMoveDown:
            archive_file_array_swap(browser, -1);
            consumed = true;
            break;
        case ArchiveBrowserEventEnterFavMove:
            strlcpy(archive->text_store, archive_get_name(browser), MAX_NAME_LEN);
            archive_show_file_menu(browser, false);
            archive_favorites_move_mode(archive->browser, true);
            consumed = true;
            break;
        case ArchiveBrowserEventExitFavMove:
            archive_update_focus(browser, archive->text_store);
            archive_favorites_move_mode(archive->browser, false);
            consumed = true;
            break;
        case ArchiveBrowserEventSaveFavMove:
            archive_favorites_move_mode(archive->browser, false);
            archive_favorites_save(archive->browser);
            consumed = true;
            break;
        case ArchiveBrowserEventLoadPrevItems:
            archive_file_array_load(archive->browser, -1);
            consumed = true;
            break;
        case ArchiveBrowserEventLoadNextItems:
            archive_file_array_load(archive->browser, 1);
            consumed = true;
            break;

        case ArchiveBrowserEventExit:
            if(archive_get_depth(browser)) {
                archive_leave_dir(browser);
            } else {
                view_dispatcher_stop(archive->view_dispatcher);
            }
            consumed = true;
            break;

        default:
            break;
        }
    }
    return consumed;
}

void archive_scene_browser_on_exit(void* context) {
    // ArchiveApp* archive = (ArchiveApp*)context;
}
