#include "../desktop_i.h"
#include "../views/desktop_locked.h"

void desktop_scene_locked_callback(DesktopLockedEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

void desktop_scene_locked_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;
    DesktopLockedView* locked_view = desktop->locked_view;

    desktop_locked_set_callback(locked_view, desktop_scene_locked_callback, desktop);
    desktop_locked_reset_door_pos(locked_view);
    desktop_locked_update_hint_timeout(locked_view);

    view_port_enabled_set(desktop->lock_viewport, true);
    osTimerStart(locked_view->timer, 63);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewLocked);
}

bool desktop_scene_locked_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;

    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopLockedEventUnlock:
            scene_manager_set_scene_state(
                desktop->scene_manager, DesktopSceneMain, DesktopMainEventUnlocked);
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneMain);
            consumed = true;
            break;
        case DesktopLockedEventUpdate:
            desktop_locked_manage_redraw(desktop->locked_view);
            consumed = true;
        default:
            break;
        }
    }

    return consumed;
}

void desktop_scene_locked_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;
    desktop_locked_reset_counter(desktop->locked_view);
    osTimerStop(desktop->locked_view->timer);
}
