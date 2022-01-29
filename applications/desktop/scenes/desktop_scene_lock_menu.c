#include <toolbox/saved_struct.h>
#include <stdbool.h>

#include "../desktop_i.h"
#include "../views/desktop_lock_menu.h"
#include "desktop_scene_i.h"
#include "desktop_scene.h"

void desktop_scene_lock_menu_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

void desktop_scene_lock_menu_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    LOAD_DESKTOP_SETTINGS(&desktop->settings);

    desktop_lock_menu_set_callback(desktop->lock_menu, desktop_scene_lock_menu_callback, desktop);
    desktop_lock_menu_pin_set(desktop->lock_menu, desktop->settings.pincode.length > 0);
    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewLockMenu);
}

bool desktop_scene_lock_menu_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopLockMenuEventLock:
            scene_manager_set_scene_state(
                desktop->scene_manager, DesktopSceneMain, DesktopMainSceneStateLockedNoPin);
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneMain);
            consumed = true;
            break;
        case DesktopLockMenuEventPinLock:
            if(desktop->settings.pincode.length > 0) {
                scene_manager_set_scene_state(
                    desktop->scene_manager, DesktopSceneMain, DesktopMainSceneStateLockedWithPin);
                scene_manager_next_scene(desktop->scene_manager, DesktopSceneMain);
            } else {
                scene_manager_next_scene(desktop->scene_manager, DesktopScenePinSetup);
            }

            consumed = true;
            break;
        case DesktopLockMenuEventExit:
            scene_manager_search_and_switch_to_previous_scene(
                desktop->scene_manager, DesktopSceneMain);
            consumed = true;
            break;
        default:
            break;
        }
    }
    return consumed;
}

void desktop_scene_lock_menu_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;
    desktop_lock_menu_reset_idx(desktop->lock_menu);
}
