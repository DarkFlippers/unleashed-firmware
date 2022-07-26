
#include <dolphin/dolphin.h>
#include <dolphin/helpers/dolphin_deed.h>

#include "../desktop_i.h"
#include "../views/desktop_view_debug.h"
#include "desktop_scene.h"

void desktop_scene_debug_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

void desktop_scene_debug_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    desktop_debug_get_dolphin_data(desktop->debug_view);

    desktop_debug_set_callback(desktop->debug_view, desktop_scene_debug_callback, desktop);
    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdDebug);
}

bool desktop_scene_debug_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopDebugEventExit:
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneMain);
            dolphin_flush(dolphin);
            consumed = true;
            break;

        case DesktopDebugEventDeed:
            dolphin_deed(dolphin, DolphinDeedTestRight);
            desktop_debug_get_dolphin_data(desktop->debug_view);
            consumed = true;
            break;

        case DesktopDebugEventWrongDeed:
            dolphin_deed(dolphin, DolphinDeedTestLeft);
            desktop_debug_get_dolphin_data(desktop->debug_view);
            consumed = true;
            break;

        case DesktopDebugEventSaveState:
            dolphin_flush(dolphin);
            consumed = true;
            break;

        default:
            break;
        }
    }

    furi_record_close(RECORD_DOLPHIN);
    return consumed;
}

void desktop_scene_debug_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;
    desktop_debug_reset_screen_idx(desktop->debug_view);
}
