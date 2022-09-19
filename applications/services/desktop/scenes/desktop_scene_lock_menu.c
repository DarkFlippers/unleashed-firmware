#include <gui/scene_manager.h>
#include <applications.h>
#include <furi_hal.h>
#include <toolbox/saved_struct.h>
#include <stdbool.h>
#include <loader/loader.h>

#include "../desktop_i.h"
#include <desktop/desktop_settings.h>
#include "../views/desktop_view_lock_menu.h"
#include "desktop_scene_i.h"
#include "desktop_scene.h"
#include "../helpers/pin_lock.h"

#define TAG "DesktopSceneLock"

void desktop_scene_lock_menu_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

void desktop_scene_lock_menu_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    DESKTOP_SETTINGS_LOAD(&desktop->settings);
    scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneLockMenu, 0);
    desktop_lock_menu_set_callback(desktop->lock_menu, desktop_scene_lock_menu_callback, desktop);
    desktop_lock_menu_set_pin_state(desktop->lock_menu, desktop->settings.pin_code.length > 0);
    desktop_lock_menu_set_dummy_mode_state(desktop->lock_menu, desktop->settings.dummy_mode);
    desktop_lock_menu_set_idx(desktop->lock_menu, 0);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdLockMenu);
}

bool desktop_scene_lock_menu_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        bool check_pin_changed =
            scene_manager_get_scene_state(desktop->scene_manager, DesktopSceneLockMenu);
        if(check_pin_changed) {
            DESKTOP_SETTINGS_LOAD(&desktop->settings);
            if(desktop->settings.pin_code.length > 0) {
                desktop_lock_menu_set_pin_state(desktop->lock_menu, true);
                scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneLockMenu, 0);
            }
        }
    } else if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopLockMenuEventLock:
            scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneLockMenu, 0);
            desktop_lock(desktop);
            consumed = true;
            break;
        case DesktopLockMenuEventPinLock:
            if(desktop->settings.pin_code.length > 0) {
                desktop_pin_lock(&desktop->settings);
                desktop_lock(desktop);
            } else {
                LoaderStatus status =
                    loader_start(desktop->loader, "Desktop", DESKTOP_SETTINGS_RUN_PIN_SETUP_ARG);
                if(status == LoaderStatusOk) {
                    scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneLockMenu, 1);
                } else {
                    FURI_LOG_E(TAG, "Unable to start desktop settings");
                }
            }
            consumed = true;
            break;
        case DesktopLockMenuEventDummyModeOn:
            desktop_set_dummy_mode_state(desktop, true);
            scene_manager_search_and_switch_to_previous_scene(
                desktop->scene_manager, DesktopSceneMain);
            break;
        case DesktopLockMenuEventDummyModeOff:
            desktop_set_dummy_mode_state(desktop, false);
            scene_manager_search_and_switch_to_previous_scene(
                desktop->scene_manager, DesktopSceneMain);
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneLockMenu, 0);
    }
    return consumed;
}

void desktop_scene_lock_menu_on_exit(void* context) {
    UNUSED(context);
}
