#include "../archive_i.h"
#include "../views/archive_main_view.h"

void archive_scene_browser_callback(ArchiveBrowserEvent event, void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;
    view_dispatcher_send_custom_event(archive->view_dispatcher, event);
}

const void archive_scene_browser_on_enter(void* context) {
    ArchiveApp* archive = (ArchiveApp*)context;
    ArchiveMainView* main_view = archive->main_view;

    archive_browser_set_callback(main_view, archive_scene_browser_callback, archive);
    archive_browser_update(main_view);
    view_dispatcher_switch_to_view(archive->view_dispatcher, ArchiveViewBrowser);
}

const bool archive_scene_browser_on_event(void* context, SceneManagerEvent event) {
    ArchiveApp* archive = (ArchiveApp*)context;
    bool consumed;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case ArchiveBrowserEventRename:
            scene_manager_next_scene(archive->scene_manager, ArchiveAppSceneRename);
            consumed = true;
            break;
        case ArchiveBrowserEventExit:
            view_dispatcher_stop(archive->view_dispatcher);
            consumed = true;
            break;

        default:
            break;
        }
    }
    return consumed;
}

const void archive_scene_browser_on_exit(void* context) {
    // ArchiveApp* archive = (ArchiveApp*)context;
}
