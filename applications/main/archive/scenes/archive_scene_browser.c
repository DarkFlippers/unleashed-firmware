#include "../archive_i.h"
#include "../helpers/archive_files.h"
#include "../helpers/archive_apps.h"
#include "../helpers/archive_favorites.h"
#include "../helpers/archive_browser.h"
#include "../views/archive_browser_view.h"
#include "archive/scenes/archive_scene.h"

#define TAG "ArchiveSceneBrowser"

#define SCENE_STATE_DEFAULT (0)
#define SCENE_STATE_NEED_REFRESH (1)

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

static void archive_loader_callback(const void* message, void* context) {
    furi_assert(message);
    furi_assert(context);
    const LoaderEvent* event = message;
    ArchiveApp* archive = (ArchiveApp*)context;

    if(event->type == LoaderEventTypeApplicationStopped) {
        view_dispatcher_send_custom_event(
            archive->view_dispatcher, ArchiveBrowserEventListRefresh);
    }
}

static void archive_run_in_app(ArchiveBrowserView* browser, ArchiveFile_t* selected) {
    UNUSED(browser);
    Loader* loader = furi_record_open(RECORD_LOADER);

    LoaderStatus status;
    if(selected->is_app) {
        char* param = strrchr(string_get_cstr(selected->path), '/');
        if(param != NULL) {
            param++;
        }
        status = loader_start(loader, flipper_app_name[selected->type], param);
    } else {
        status = loader_start(
            loader, flipper_app_name[selected->type], string_get_cstr(selected->path));
    }

    if(status != LoaderStatusOk) {
        FURI_LOG_E(TAG, "loader_start failed: %d", status);
    }

    furi_record_close(RECORD_LOADER);
}

void archive_scene_browser_callback(ArchiveBrowserEvent event, void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;
    view_dispatcher_send_custom_event(archive->view_dispatcher, event);
}

void archive_scene_browser_on_enter(void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;
    ArchiveBrowserView* browser = archive->browser;
    browser->is_root = true;

    archive_browser_set_callback(browser, archive_scene_browser_callback, archive);
    archive_update_focus(browser, archive->text_store);
    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewBrowser);

    Loader* loader = furi_record_open(RECORD_LOADER);
    archive->loader_stop_subscription =
        furi_pubsub_subscribe(loader_get_pubsub(loader), archive_loader_callback, archive);
    furi_record_close(RECORD_LOADER);

    uint32_t state = scene_manager_get_scene_state(archive->scene_manager, ArchiveAppSceneBrowser);

    if(state == SCENE_STATE_NEED_REFRESH) {
        view_dispatcher_send_custom_event(
            archive->view_dispatcher, ArchiveBrowserEventListRefresh);
    }

    scene_manager_set_scene_state(
        archive->scene_manager, ArchiveAppSceneBrowser, SCENE_STATE_DEFAULT);
}

bool archive_scene_browser_on_event(void* context, SceneManagerEvent event) {
    ArchiveApp* archive = (ArchiveApp*)context;
    ArchiveBrowserView* browser = archive->browser;
    ArchiveFile_t* selected = archive_get_current_file(browser);

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
            if(archive_is_known_app(selected->type)) {
                archive_run_in_app(browser, selected);
                archive_show_file_menu(browser, false);
            }
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuPin: {
            const char* name = archive_get_name(browser);
            if(favorites) {
                archive_favorites_delete(name);
                archive_file_array_rm_selected(browser);
                archive_show_file_menu(browser, false);
            } else if(archive_is_known_app(selected->type)) {
                if(archive_is_favorite("%s", name)) {
                    archive_favorites_delete("%s", name);
                } else {
                    archive_file_append(ARCHIVE_FAV_PATH, "%s\n", name);
                }
                archive_show_file_menu(browser, false);
            }
            consumed = true;
        } break;

        case ArchiveBrowserEventFileMenuRename:
            if(favorites) {
                browser->callback(ArchiveBrowserEventEnterFavMove, browser->context);
            } else if((archive_is_known_app(selected->type)) && (selected->is_app == false)) {
                archive_show_file_menu(browser, false);
                scene_manager_set_scene_state(
                    archive->scene_manager, ArchiveAppSceneBrowser, SCENE_STATE_NEED_REFRESH);
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
            archive_enter_dir(browser, selected->path);
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
            string_set(archive->fav_move_str, selected->path);
            archive_show_file_menu(browser, false);
            archive_favorites_move_mode(archive->browser, true);
            consumed = true;
            break;
        case ArchiveBrowserEventExitFavMove:
            archive_update_focus(browser, string_get_cstr(archive->fav_move_str));
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
        case ArchiveBrowserEventListRefresh:
            if(!favorites) {
                archive_refresh_dir(browser);
            } else {
                archive_favorites_read(browser);
            }
            consumed = true;
            break;

        case ArchiveBrowserEventExit:
            if(!archive_is_home(browser)) {
                archive_leave_dir(browser);
            } else {
                Loader* loader = furi_record_open(RECORD_LOADER);
                furi_pubsub_unsubscribe(
                    loader_get_pubsub(loader), archive->loader_stop_subscription);
                furi_record_close(RECORD_LOADER);

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
    ArchiveApp* archive = (ArchiveApp*)context;

    Loader* loader = furi_record_open(RECORD_LOADER);
    furi_pubsub_unsubscribe(loader_get_pubsub(loader), archive->loader_stop_subscription);
    furi_record_close(RECORD_LOADER);
}
