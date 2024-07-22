#include "../archive_i.h"
#include "../helpers/archive_files.h"
#include "../helpers/archive_favorites.h"
#include "../helpers/archive_browser.h"
#include "../views/archive_browser_view.h"
#include "archive/scenes/archive_scene.h"
#include <applications.h>

#define TAG "ArchiveSceneBrowser"

#define SCENE_STATE_DEFAULT      (0)
#define SCENE_STATE_NEED_REFRESH (1)

static const char* archive_get_flipper_app_name(ArchiveFileTypeEnum file_type) {
    switch(file_type) {
    case ArchiveFileTypeIButton:
        return "iButton";
    case ArchiveFileTypeNFC:
        return "NFC";
    case ArchiveFileTypeSubGhz:
        return "Sub-GHz";
    case ArchiveFileTypeSubGhzRemote:
        return "Sub-GHz Remote";
    case ArchiveFileTypeLFRFID:
        return "125 kHz RFID";
    case ArchiveFileTypeInfrared:
        return "Infrared";
    case ArchiveFileTypeBadUsb:
        return "Bad USB";
    case ArchiveFileTypeU2f:
        return "U2F";
    case ArchiveFileTypeUpdateManifest:
        return "UpdaterApp";
    case ArchiveFileTypeJS:
        return "JS Runner";
    default:
        return NULL;
    }
}

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

    const char* app_name = archive_get_flipper_app_name(selected->type);

    if(app_name) {
        if(selected->is_app) {
            char* param = strrchr(furi_string_get_cstr(selected->path), '/');
            if(param != NULL) {
                param++;
            }
            loader_start_with_gui_error(loader, app_name, param);
        } else {
            loader_start_with_gui_error(loader, app_name, furi_string_get_cstr(selected->path));
        }
    } else {
        loader_start_with_gui_error(loader, furi_string_get_cstr(selected->path), NULL);
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

    archive->loader_stop_subscription = furi_pubsub_subscribe(
        loader_get_pubsub(archive->loader), archive_loader_callback, archive);

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
                archive_favorites_delete("%s", name);
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
                //} else if((archive_is_known_app(selected->type)) && (selected->is_app == false)) {
            } else {
                // Added ability to rename files and folders
                archive_show_file_menu(browser, false);
                scene_manager_set_scene_state(
                    archive->scene_manager, ArchiveAppSceneBrowser, SCENE_STATE_NEED_REFRESH);
                scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneRename);
            }
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuNewDir:
            archive_show_file_menu(browser, false);
            if(!favorites) {
                scene_manager_set_scene_state(
                    archive->scene_manager, ArchiveAppSceneBrowser, SCENE_STATE_NEED_REFRESH);
                scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneNewDir);
            }
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuCut:
        case ArchiveBrowserEventFileMenuCopy:
            archive_show_file_menu(browser, false);
            furi_string_set(archive->fav_move_str, selected->path);

            archive_browser_clipboard_set_mode(
                browser,
                (event.event == ArchiveBrowserEventFileMenuCut) ? CLIPBOARD_MODE_CUT :
                                                                  CLIPBOARD_MODE_COPY);
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuPaste_Cut:
        case ArchiveBrowserEventFileMenuPaste_Copy:
            archive_show_file_menu(browser, false);

            FuriString* path_src = archive->fav_move_str;
            FuriString* path_dst = furi_string_alloc();
            FuriString* base = furi_string_alloc();

            const bool copy = (event.event == ArchiveBrowserEventFileMenuPaste_Copy);

            path_extract_basename(furi_string_get_cstr(path_src), base);
            path_concat(furi_string_get_cstr(browser->path), furi_string_get_cstr(base), path_dst);

            if(path_src && path_dst) {
                view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewStack);
                archive_show_loading_popup(archive, true);
                FS_Error error = archive_rename_copy_file_or_dir(
                    archive->browser,
                    furi_string_get_cstr(path_src),
                    furi_string_get_cstr(path_dst),
                    copy);
                archive_show_loading_popup(archive, false);

                if(error != FSE_OK) {
                    FuriString* dialog_msg;
                    dialog_msg = furi_string_alloc();
                    furi_string_cat_printf(
                        dialog_msg,
                        "Cannot %s:\n%s",
                        copy ? "copy" : "move",
                        storage_error_get_desc(error));
                    dialog_message_show_storage_error(
                        archive->dialogs, furi_string_get_cstr(dialog_msg));
                    furi_string_free(dialog_msg);
                } else {
                    ArchiveFile_t* current = archive_get_current_file(archive->browser);
                    if(current != NULL) furi_string_set(current->path, path_dst);
                    view_dispatcher_send_custom_event(
                        archive->view_dispatcher, ArchiveBrowserEventListRefresh);
                }

                view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewBrowser);
            }

            furi_string_free(base);
            furi_string_free(path_dst);

            archive_browser_clipboard_reset(browser);
            furi_string_reset(path_src);

            break;
        case ArchiveBrowserEventFileMenuInfo:
            archive_show_file_menu(browser, false);
            scene_manager_set_scene_state(
                archive->scene_manager, ArchiveAppSceneBrowser, SCENE_STATE_DEFAULT);
            scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneInfo);
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuShow:
            archive_show_file_menu(browser, false);
            scene_manager_set_scene_state(
                archive->scene_manager, ArchiveAppSceneBrowser, SCENE_STATE_DEFAULT);
            scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneShow);
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuDelete:
            if(archive_get_tab(browser) != ArchiveTabFavorites) {
                archive_show_file_menu(browser, false);
                scene_manager_set_scene_state(
                    archive->scene_manager, ArchiveAppSceneBrowser, SCENE_STATE_NEED_REFRESH);
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
            furi_string_set(archive->fav_move_str, selected->path);
            archive_show_file_menu(browser, false);
            archive_favorites_move_mode(archive->browser, true);
            consumed = true;
            break;
        case ArchiveBrowserEventExitFavMove:
            archive_update_focus(browser, furi_string_get_cstr(archive->fav_move_str));
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
                if(archive->loader_stop_subscription) {
                    furi_pubsub_unsubscribe(
                        loader_get_pubsub(archive->loader), archive->loader_stop_subscription);
                    archive->loader_stop_subscription = NULL;
                }

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
    if(archive->loader_stop_subscription) {
        furi_pubsub_unsubscribe(
            loader_get_pubsub(archive->loader), archive->loader_stop_subscription);
        archive->loader_stop_subscription = NULL;
    }
}
