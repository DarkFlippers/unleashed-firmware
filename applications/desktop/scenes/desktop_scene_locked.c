#include "../desktop_i.h"
#include "../views/desktop_locked.h"
#include "desktop/views/desktop_main.h"

void desktop_scene_locked_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

static void desktop_scene_locked_new_idle_animation_callback(void* context) {
    furi_assert(context);
    Desktop* desktop = context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopLockedEventCheckAnimation);
}

void desktop_scene_locked_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;
    DesktopLockedView* locked_view = desktop->locked_view;

    animation_manager_set_new_idle_callback(
        desktop->animation_manager, desktop_scene_locked_new_idle_animation_callback);
    desktop_locked_set_callback(locked_view, desktop_scene_locked_callback, desktop);
    desktop_locked_reset_door_pos(locked_view);
    desktop_locked_update_hint_timeout(locked_view);

    uint32_t state = scene_manager_get_scene_state(desktop->scene_manager, DesktopViewLocked);

    desktop_locked_with_pin(desktop->locked_view, state == DesktopLockedWithPin);

    view_port_enabled_set(desktop->lock_viewport, true);
    osTimerStart(locked_view->timer, osKernelGetTickFreq() / 16);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewLocked);
}

static bool desktop_scene_locked_check_pin(Desktop* desktop, DesktopEvent event) {
    bool match = false;

    size_t length = desktop->pincode_buffer.length;
    length = code_input_push(desktop->pincode_buffer.data, length, event);
    desktop->pincode_buffer.length = length;

    match = code_input_compare(
        desktop->pincode_buffer.data,
        length,
        desktop->settings.pincode.data,
        desktop->settings.pincode.length);

    if(match) {
        desktop->pincode_buffer.length = 0;
        furi_hal_usb_enable();
        furi_hal_rtc_reset_flag(FuriHalRtcFlagLock);
        desktop_main_unlocked(desktop->main_view);
    }

    return match;
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
            break;
        case DesktopLockedEventInputReset:
            desktop->pincode_buffer.length = 0;
            break;
        case DesktopLockedEventCheckAnimation:
            animation_manager_check_blocking_process(desktop->animation_manager);
            consumed = true;
            break;
        default:
            if(desktop_scene_locked_check_pin(desktop, event.event)) {
                scene_manager_set_scene_state(
                    desktop->scene_manager, DesktopSceneMain, DesktopMainEventUnlocked);
                scene_manager_next_scene(desktop->scene_manager, DesktopSceneMain);
                consumed = true;
            }
            break;
        }
    }

    return consumed;
}

void desktop_scene_locked_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;
    animation_manager_set_new_idle_callback(desktop->animation_manager, NULL);
    desktop_locked_reset_counter(desktop->locked_view);
    osTimerStop(desktop->locked_view->timer);
}
