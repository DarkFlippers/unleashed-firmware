#include <furi.h>
#include <furi_hal.h>
#include <gui/scene_manager.h>
#include <gui/view_stack.h>
#include <stdint.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include "../desktop.h"
#include "../desktop_i.h"
#include "../views/desktop_events.h"
#include "../views/desktop_view_pin_input.h"
#include "../helpers/pin_code.h"
#include "desktop_scene.h"

#define WRONG_PIN_HEADER_TIMEOUT 3000
#define INPUT_PIN_VIEW_TIMEOUT   15000

typedef struct {
    FuriTimer* timer;
    FuriString* enter_pin_string;
} DesktopScenePinInputState;

static void desktop_scene_locked_light_red(bool value) {
    NotificationApp* app = furi_record_open(RECORD_NOTIFICATION);
    if(value) {
        notification_message(app, &sequence_set_only_red_255);
    } else {
        notification_message(app, &sequence_reset_red);
    }
    furi_record_close(RECORD_NOTIFICATION);
}

static void desktop_scene_pin_input_set_timer(Desktop* desktop, bool enable, uint32_t new_period) {
    furi_assert(desktop);

    DesktopScenePinInputState* state = (DesktopScenePinInputState*)scene_manager_get_scene_state(
        desktop->scene_manager, DesktopScenePinInput);
    furi_assert(state);
    if(enable) {
        furi_timer_start(state->timer, new_period);
    } else {
        furi_timer_stop(state->timer);
    }
}

static void desktop_scene_pin_input_back_callback(void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopPinInputEventBack);
}

static void desktop_scene_pin_input_done_callback(const DesktopPinCode* pin_code, void* context) {
    Desktop* desktop = (Desktop*)context;

    if(desktop_pin_code_check(pin_code)) {
        view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopPinInputEventUnlocked);

    } else {
        uint32_t pin_fails = furi_hal_rtc_get_pin_fails();
        furi_hal_rtc_set_pin_fails(pin_fails + 1);
        view_dispatcher_send_custom_event(
            desktop->view_dispatcher, DesktopPinInputEventUnlockFailed);
    }
}

static void desktop_scene_pin_input_timer_callback(void* context) {
    Desktop* desktop = context;

    view_dispatcher_send_custom_event(
        desktop->view_dispatcher, DesktopPinInputEventResetWrongPinLabel);
}

static void
    desktop_scene_pin_input_update_wrong_count(DesktopScenePinInputState* state, Desktop* desktop) {
    uint32_t attempts = furi_hal_rtc_get_pin_fails();
    if(attempts > 0) {
        furi_string_printf(state->enter_pin_string, "Wrong Attempts: %lu", attempts);
        desktop_view_pin_input_set_label_tertiary(
            desktop->pin_input_view, 64, 60, furi_string_get_cstr(state->enter_pin_string));
    } else {
        desktop_view_pin_input_set_label_tertiary(desktop->pin_input_view, 64, 60, NULL);
    }
}

void desktop_scene_pin_input_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    desktop_view_pin_input_set_context(desktop->pin_input_view, desktop);
    desktop_view_pin_input_set_back_callback(
        desktop->pin_input_view, desktop_scene_pin_input_back_callback);
    desktop_view_pin_input_set_timeout_callback(
        desktop->pin_input_view, desktop_scene_pin_input_back_callback);
    desktop_view_pin_input_set_done_callback(
        desktop->pin_input_view, desktop_scene_pin_input_done_callback);

    DesktopScenePinInputState* state = malloc(sizeof(DesktopScenePinInputState));
    state->enter_pin_string = furi_string_alloc();
    state->timer =
        furi_timer_alloc(desktop_scene_pin_input_timer_callback, FuriTimerTypeOnce, desktop);
    scene_manager_set_scene_state(desktop->scene_manager, DesktopScenePinInput, (uint32_t)state);

    desktop_view_pin_input_hide_pin(desktop->pin_input_view, true);
    desktop_view_pin_input_set_label_button(desktop->pin_input_view, "OK");
    desktop_view_pin_input_set_label_secondary(desktop->pin_input_view, 44, 25, "Enter PIN:");
    desktop_scene_pin_input_update_wrong_count(state, desktop);
    desktop_view_pin_input_set_pin_position(desktop->pin_input_view, 64, 37);
    desktop_view_pin_input_reset_pin(desktop->pin_input_view);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdPinInput);
}

bool desktop_scene_pin_input_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;
    uint32_t pin_timeout = 0;
    DesktopScenePinInputState* state = (DesktopScenePinInputState*)scene_manager_get_scene_state(
        desktop->scene_manager, DesktopScenePinInput);
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopPinInputEventUnlockFailed:
            pin_timeout = desktop_pin_lock_get_fail_timeout();
            if(pin_timeout > 0) {
                desktop_pin_lock_error_notify();
                scene_manager_set_scene_state(
                    desktop->scene_manager, DesktopScenePinTimeout, pin_timeout);
                scene_manager_next_scene(desktop->scene_manager, DesktopScenePinTimeout);
            } else {
                desktop_scene_locked_light_red(true);
                desktop_view_pin_input_set_label_primary(desktop->pin_input_view, 0, 0, NULL);
                desktop_view_pin_input_set_label_secondary(
                    desktop->pin_input_view, 25, 25, "Wrong PIN try again:");
                desktop_scene_pin_input_set_timer(desktop, true, WRONG_PIN_HEADER_TIMEOUT);
                desktop_scene_pin_input_update_wrong_count(state, desktop);
                desktop_view_pin_input_reset_pin(desktop->pin_input_view);
            }
            consumed = true;
            break;
        case DesktopPinInputEventResetWrongPinLabel:
            desktop_scene_locked_light_red(false);
            desktop_view_pin_input_set_label_primary(desktop->pin_input_view, 0, 0, NULL);
            desktop_view_pin_input_set_label_secondary(
                desktop->pin_input_view, 44, 25, "Enter PIN:");
            desktop_scene_pin_input_update_wrong_count(state, desktop);
            consumed = true;
            break;
        case DesktopPinInputEventUnlocked:
        case DesktopGlobalApiUnlock:
            desktop_unlock(desktop);
            consumed = true;
            break;
        case DesktopPinInputEventBack:
            scene_manager_search_and_switch_to_previous_scene(
                desktop->scene_manager, DesktopSceneLocked);
            notification_message(desktop->notification, &sequence_display_backlight_off);
            consumed = true;
            break;
        }
    }

    return consumed;
}

void desktop_scene_pin_input_on_exit(void* context) {
    Desktop* desktop = (Desktop*)context;
    desktop_scene_locked_light_red(false);

    DesktopScenePinInputState* state = (DesktopScenePinInputState*)scene_manager_get_scene_state(
        desktop->scene_manager, DesktopScenePinInput);

    furi_timer_free(state->timer);
    furi_string_free(state->enter_pin_string);
    free(state);
}
