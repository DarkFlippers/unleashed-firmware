#include "desktop/desktop_settings/desktop_settings.h"
#include "furi/check.h"
#include "gui/view.h"
#include "portmacro.h"
#include <furi.h>
#include <gui/gui_i.h>
#include <gui/elements.h>
#include "../desktop_i.h"
#include "desktop_locked.h"
#include <stdint.h>

#define DOOR_MOVING_INTERVAL_MS (1000 / 16)
#define UNLOCKED_HINT_TIMEOUT_MS (2000)

struct DesktopLockedView {
    View* view;
    DesktopLockedViewCallback callback;
    void* context;

    TimerHandle_t timer;
    uint8_t lock_count;
    uint32_t lock_lastpress;

    PinCode pincode;
    PinCode pincode_input;
};

typedef struct {
    uint32_t hint_icon_expire_at;
    bool unlocked_hint;
    bool locked;
    bool pin_locked;

    int8_t door_left_x;
    int8_t door_right_x;
    bool animation_seq_end;
} DesktopLockedViewModel;

static void desktop_locked_unlock(DesktopLockedView* locked_view);

void desktop_locked_set_callback(
    DesktopLockedView* locked_view,
    DesktopLockedViewCallback callback,
    void* context) {
    furi_assert(locked_view);
    furi_assert(callback);
    locked_view->callback = callback;
    locked_view->context = context;
}

void locked_view_timer_callback(TimerHandle_t timer) {
    DesktopLockedView* locked_view = pvTimerGetTimerID(timer);
    locked_view->callback(DesktopMainEventUpdate, locked_view->context);
}

static void desktop_locked_update_hint_icon_timeout(DesktopLockedView* locked_view) {
    DesktopLockedViewModel* model = view_get_model(locked_view->view);
    model->hint_icon_expire_at = osKernelGetTickCount() + osKernelGetTickFreq();
    view_commit_model(locked_view->view, true);
}

static void desktop_locked_reset_door_pos(DesktopLockedView* locked_view) {
    DesktopLockedViewModel* model = view_get_model(locked_view->view);
    model->animation_seq_end = false;
    model->door_left_x = DOOR_L_POS;
    model->door_right_x = DOOR_R_POS;
    view_commit_model(locked_view->view, true);
}

void desktop_locked_update(DesktopLockedView* locked_view) {
    bool stop_timer = false;

    DesktopLockedViewModel* model = view_get_model(locked_view->view);
    if(model->locked) {
        if(model->door_left_x != DOOR_L_POS_MAX) {
            model->door_left_x = CLAMP(model->door_left_x + 5, DOOR_L_POS_MAX, DOOR_L_POS);
            model->door_right_x = CLAMP(model->door_right_x - 5, DOOR_R_POS, DOOR_R_POS_MIN);
        } else {
            model->animation_seq_end = true;
        }
        stop_timer = model->animation_seq_end;
    } else {
        model->unlocked_hint = false;
        stop_timer = true;
    }
    view_commit_model(locked_view->view, true);

    if(stop_timer) {
        xTimerStop(locked_view->timer, portMAX_DELAY);
    }
}

void desktop_locked_draw(Canvas* canvas, void* model) {
    DesktopLockedViewModel* m = model;
    uint32_t now = osKernelGetTickCount();
    canvas_set_color(canvas, ColorBlack);

    if(m->locked) {
        if(!m->animation_seq_end) {
            canvas_draw_icon(canvas, m->door_left_x, 0 + STATUS_BAR_Y_SHIFT, &I_DoorLeft_70x55);
            canvas_draw_icon(canvas, m->door_right_x, 0 + STATUS_BAR_Y_SHIFT, &I_DoorRight_70x55);
            canvas_set_font(canvas, FontPrimary);
            elements_multiline_text_framed(canvas, 42, 30 + STATUS_BAR_Y_SHIFT, "Locked");
        } else if((now < m->hint_icon_expire_at) && !m->pin_locked) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_icon(canvas, 13, 2 + STATUS_BAR_Y_SHIFT, &I_LockPopup_100x49);
            elements_multiline_text(canvas, 65, 20 + STATUS_BAR_Y_SHIFT, "To unlock\npress:");
        }
    } else {
        if(m->unlocked_hint) {
            canvas_set_font(canvas, FontPrimary);
            elements_multiline_text_framed(canvas, 42, 30 + STATUS_BAR_Y_SHIFT, "Unlocked");
        }
    }
}

View* desktop_locked_get_view(DesktopLockedView* locked_view) {
    furi_assert(locked_view);
    return locked_view->view;
}

bool desktop_locked_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    DesktopLockedView* locked_view = context;
    bool locked = false;
    bool locked_with_pin = false;
    uint32_t press_time = xTaskGetTickCount();

    {
        DesktopLockedViewModel* model = view_get_model(locked_view->view);
        bool changed = false;
        locked = model->locked;
        locked_with_pin = model->pin_locked;
        if(!locked && model->unlocked_hint && event->type == InputTypePress) {
            model->unlocked_hint = false;
            changed = true;
        }
        view_commit_model(locked_view->view, changed);
    }

    if(!locked || (event->type != InputTypeShort)) {
        return locked;
    }

    if(press_time - locked_view->lock_lastpress > UNLOCK_RST_TIMEOUT) {
        locked_view->lock_lastpress = press_time;
        locked_view->lock_count = 0;
        locked_view->pincode_input.length = 0;
    }

    if(locked_with_pin) {
        locked_view->pincode_input.length = code_input_push(
            locked_view->pincode_input.data, locked_view->pincode_input.length, event->key);
        bool match = code_input_compare(
            locked_view->pincode_input.data,
            locked_view->pincode_input.length,
            locked_view->pincode.data,
            locked_view->pincode.length);

        if(match) {
            desktop_locked_unlock(locked_view);
        }
    } else {
        if(event->key == InputKeyBack) {
            locked_view->lock_lastpress = press_time;
            locked_view->lock_count++;
            if(locked_view->lock_count == UNLOCK_CNT) {
                desktop_locked_unlock(locked_view);
            }
        } else {
            desktop_locked_update_hint_icon_timeout(locked_view);
            locked_view->lock_count = 0;
        }
    }

    locked_view->lock_lastpress = press_time;

    return locked;
}

DesktopLockedView* desktop_locked_alloc() {
    DesktopLockedView* locked_view = furi_alloc(sizeof(DesktopLockedView));
    locked_view->view = view_alloc();
    locked_view->timer =
        xTimerCreate("Locked view", 1000 / 16, pdTRUE, locked_view, locked_view_timer_callback);

    view_allocate_model(locked_view->view, ViewModelTypeLocking, sizeof(DesktopLockedViewModel));
    view_set_context(locked_view->view, locked_view);
    view_set_draw_callback(locked_view->view, (ViewDrawCallback)desktop_locked_draw);
    view_set_input_callback(locked_view->view, desktop_locked_input);

    return locked_view;
}

void desktop_locked_free(DesktopLockedView* locked_view) {
    furi_assert(locked_view);
    osTimerDelete(locked_view->timer);
    view_free(locked_view->view);
    free(locked_view);
}

void desktop_locked_lock(DesktopLockedView* locked_view) {
    locked_view->pincode.length = 0;
    DesktopLockedViewModel* model = view_get_model(locked_view->view);
    model->locked = true;
    model->pin_locked = false;
    view_commit_model(locked_view->view, true);
    desktop_locked_reset_door_pos(locked_view);
    xTimerChangePeriod(locked_view->timer, DOOR_MOVING_INTERVAL_MS, portMAX_DELAY);
}

void desktop_locked_lock_pincode(DesktopLockedView* locked_view, PinCode pincode) {
    locked_view->pincode = pincode;
    locked_view->pincode_input.length = 0;
    DesktopLockedViewModel* model = view_get_model(locked_view->view);
    model->locked = true;
    model->pin_locked = true;
    view_commit_model(locked_view->view, true);
    desktop_locked_reset_door_pos(locked_view);
    xTimerChangePeriod(locked_view->timer, DOOR_MOVING_INTERVAL_MS, portMAX_DELAY);
}

static void desktop_locked_unlock(DesktopLockedView* locked_view) {
    furi_assert(locked_view);

    locked_view->lock_count = 0;
    DesktopLockedViewModel* model = view_get_model(locked_view->view);
    model->locked = false;
    model->pin_locked = false;
    model->unlocked_hint = true;
    view_commit_model(locked_view->view, true);
    locked_view->callback(DesktopMainEventUnlocked, locked_view->context);
    xTimerChangePeriod(locked_view->timer, UNLOCKED_HINT_TIMEOUT_MS, portMAX_DELAY);
}
