#include "../desktop_i.h"
#include "../views/desktop_first_start.h"
#include "applications/dolphin/dolphin.h"

void desktop_scene_first_start_callback(DesktopFirstStartEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

const void desktop_scene_first_start_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;
    DesktopFirstStartView* first_start_view = desktop->first_start_view;

    desktop_first_start_set_callback(
        first_start_view, desktop_scene_first_start_callback, desktop);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewFirstStart);
}

const bool desktop_scene_first_start_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopFirstStartCompleted:
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneMain);
            consumed = true;
            break;

        default:
            break;
        }
    }
    return consumed;
}

const void desktop_scene_first_start_on_exit(void* context) {
    // Desktop* desktop = (Desktop*)context;
    Dolphin* dolphin = furi_record_open("dolphin");
    dolphin_save(dolphin);
    furi_record_close("dolphin");
}
