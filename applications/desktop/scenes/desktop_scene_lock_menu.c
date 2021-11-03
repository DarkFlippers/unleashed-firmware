#include "../desktop_i.h"
#include "../views/desktop_lock_menu.h"
#include <toolbox/saved_struct.h>
#include <stdbool.h>
#include <furi-hal-lock.h>

void desktop_scene_lock_menu_callback(DesktopLockMenuEvent event, void* context) {
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
                desktop->scene_manager, DesktopSceneLocked, DesktopLockedNoPin);
            scene_manager_next_scene(desktop->scene_manager, DesktopSceneLocked);
            consumed = true;
            break;
        case DesktopLockMenuEventPinLock:
            if(desktop->settings.pincode.length > 0) {
                furi_hal_lock_set(true);
                furi_hal_usb_disable();
                scene_manager_set_scene_state(
                    desktop->scene_manager, DesktopSceneLocked, DesktopLockedWithPin);
                scene_manager_next_scene(desktop->scene_manager, DesktopSceneLocked);
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
