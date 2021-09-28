#include "../desktop_i.h"
#include "../views/desktop_hw_mismatch.h"

void desktop_scene_hw_mismatch_callback(DesktopHwMismatchEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

const void desktop_scene_hw_mismatch_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    desktop_hw_mismatch_set_callback(
        desktop->hw_mismatch_view, desktop_scene_hw_mismatch_callback, desktop);
    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewHwMismatch);
}

const bool desktop_scene_hw_mismatch_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopHwMismatchEventExit:
            scene_manager_previous_scene(desktop->scene_manager);
            consumed = true;
            break;

        default:
            break;
        }
    }
    return consumed;
}

const void desktop_scene_hw_mismatch_on_exit(void* context) {
    // Desktop* desktop = (Desktop*)context;
}
