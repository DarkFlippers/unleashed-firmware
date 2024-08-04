#include <furi.h>
#include <furi_hal.h>
#include <gui/scene_manager.h>
#include <gui/view_stack.h>
#include <stdint.h>

#include "../desktop.h"
#include "../desktop_i.h"
#include "../helpers/pin_code.h"
#include "../animations/animation_manager.h"
#include "../views/desktop_events.h"
#include "../views/desktop_view_locked.h"
#include "desktop_scene.h"
#include "desktop_scene_locked.h"

#define WRONG_PIN_HEADER_TIMEOUT 3000
#define INPUT_PIN_VIEW_TIMEOUT   15000

static void desktop_scene_locked_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

static void desktop_scene_locked_new_idle_animation_callback(void* context) {
    furi_assert(context);
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(
        desktop->view_dispatcher, DesktopAnimationEventNewIdleAnimation);
}

void desktop_scene_locked_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    // callbacks for 1-st layer
    animation_manager_set_new_idle_callback(
        desktop->animation_manager, desktop_scene_locked_new_idle_animation_callback);
    animation_manager_set_check_callback(desktop->animation_manager, NULL);
    animation_manager_set_interact_callback(desktop->animation_manager, NULL);

    // callbacks for 2-nd layer
    desktop_view_locked_set_callback(desktop->locked_view, desktop_scene_locked_callback, desktop);

    bool switch_to_timeout_scene = false;
    uint32_t state = scene_manager_get_scene_state(desktop->scene_manager, DesktopSceneLocked);
    if(state == DesktopSceneLockedStateFirstEnter) {
        view_port_enabled_set(desktop->lock_icon_viewport, true);
        Gui* gui = furi_record_open(RECORD_GUI);
        gui_set_lockdown(gui, true);
        furi_record_close(RECORD_GUI);

        if(desktop_pin_code_is_set()) {
            desktop_view_locked_lock(desktop->locked_view, true);
            uint32_t pin_timeout = desktop_pin_lock_get_fail_timeout();
            if(pin_timeout > 0) {
                scene_manager_set_scene_state(
                    desktop->scene_manager, DesktopScenePinTimeout, pin_timeout);
                switch_to_timeout_scene = true;
            } else {
                desktop_view_locked_close_doors(desktop->locked_view);
            }
        } else {
            desktop_view_locked_lock(desktop->locked_view, false);
            desktop_view_locked_close_doors(desktop->locked_view);
        }
        scene_manager_set_scene_state(
            desktop->scene_manager, DesktopSceneLocked, DesktopSceneLockedStateRepeatEnter);
    }

    if(switch_to_timeout_scene) {
        scene_manager_next_scene(desktop->scene_manager, DesktopScenePinTimeout);
    } else {
        view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdLocked);
    }
}

bool desktop_scene_locked_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopLockedEventUnlocked:
        case DesktopGlobalApiUnlock:
            desktop_unlock(desktop);
            consumed = true;
            break;
        case DesktopLockedEventDoorsClosed:
            notification_message(desktop->notification, &sequence_display_backlight_off);
            consumed = true;
            break;
        case DesktopLockedEventUpdate:
            if(desktop_view_locked_is_locked_hint_visible(desktop->locked_view)) {
                notification_message(desktop->notification, &sequence_display_backlight_off);
            }
            desktop_view_locked_update(desktop->locked_view);
            consumed = true;
            break;
        case DesktopLockedEventShowPinInput:
            scene_manager_next_scene(desktop->scene_manager, DesktopScenePinInput);
            consumed = true;
            break;
        case DesktopAnimationEventNewIdleAnimation:
            animation_manager_new_idle_process(desktop->animation_manager);
            consumed = true;
            break;
        }
    }

    return consumed;
}

void desktop_scene_locked_on_exit(void* context) {
    UNUSED(context);
}
