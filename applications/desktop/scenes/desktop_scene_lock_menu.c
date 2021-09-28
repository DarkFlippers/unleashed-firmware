#include "../desktop_i.h"
#include "../views/desktop_lock_menu.h"

void desktop_scene_lock_menu_callback(DesktopLockMenuEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

const void desktop_scene_lock_menu_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    desktop_lock_menu_set_callback(desktop->lock_menu, desktop_scene_lock_menu_callback, desktop);
    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewLockMenu);
}

const bool desktop_scene_lock_menu_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopLockMenuEventLock:
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneLocked);
            consumed = true;
            break;

        case DesktopLockMenuEventExit:
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneMain);
            consumed = true;
            break;

        default:
            break;
        }
    }
    return consumed;
}

const void desktop_scene_lock_menu_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;
    desktop_lock_menu_reset_idx(desktop->lock_menu);
}
