#include "../archive_i.h"
#include "../helpers/archive_files.h"
#include "../helpers/archive_favorites.h"
#include "../helpers/archive_browser.h"
#include "../views/archive_browser_view.h"

static const char* flipper_app_name[] = {
    [ArchiveFileTypeIButton] = "iButton",
    [ArchiveFileTypeNFC] = "NFC",
    [ArchiveFileTypeSubGhz] = "Sub-GHz",
    [ArchiveFileTypeLFRFID] = "125 kHz RFID",
    [ArchiveFileTypeIrda] = "Infrared",
};

static void archive_run_in_app(
    ArchiveBrowserView* browser,
    ArchiveFile_t* selected,
    bool full_path_provided) {
    Loader* loader = furi_record_open("loader");

    string_t full_path;
    if(!full_path_provided) {
        string_init_printf(
            full_path, "%s/%s", string_get_cstr(browser->path), string_get_cstr(selected->name));
    } else {
        string_init_set(full_path, selected->name);
    }
    loader_start(loader, flipper_app_name[selected->type], string_get_cstr(full_path));

    string_clear(full_path);
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

    const char* path = archive_get_path(browser);
    const char* name = archive_get_name(browser);
    bool known_app = is_known_app(selected->type);
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
                archive_run_in_app(browser, selected, favorites);
            }
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuPin:
            if(favorites) {
                archive_favorites_delete(name);
                archive_file_array_rm_selected(browser);
            } else if(known_app) {
                if(archive_is_favorite("%s/%s", path, name)) {
                    archive_favorites_delete("%s/%s", path, name);
                } else {
                    archive_file_append(ARCHIVE_FAV_PATH, "%s/%s\r\n", path, name);
                }
            }
            archive_show_file_menu(browser, false);
            consumed = true;
            break;

        case ArchiveBrowserEventFileMenuRename:
            if(known_app && !favorites) {
                scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneRename);
            }
            consumed = true;
            break;
        case ArchiveBrowserEventFileMenuDelete:
            archive_delete_file(browser, browser->path, selected->name);
            archive_show_file_menu(browser, false);
            consumed = true;
            break;
        case ArchiveBrowserEventEnterDir:
            archive_enter_dir(browser, selected->name);
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
