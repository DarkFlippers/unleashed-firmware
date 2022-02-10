#include <projdefs.h>
#include <stdint.h>
#include <furi.h>
#include <gui/elements.h>
#include <gui/icon.h>
#include <gui/view.h>
#include <portmacro.h>

#include "../desktop_settings/desktop_settings.h"
#include "../desktop_i.h"
#include "desktop_view_locked.h"

#define DOOR_MOVING_INTERVAL_MS (1000 / 16)
#define UNLOCKED_HINT_TIMEOUT_MS (2000)

#define DOOR_OFFSET_START -55
#define DOOR_OFFSET_END 0

#define DOOR_L_FINAL_POS 0
#define DOOR_R_FINAL_POS 60

#define UNLOCK_CNT 3
#define UNLOCK_RST_TIMEOUT 600

struct DesktopViewLocked {
    View* view;
    DesktopViewLockedCallback callback;
    void* context;

    TimerHandle_t timer;
    uint8_t lock_count;
    uint32_t lock_lastpress;
};

typedef struct {
    uint32_t hint_icon_expire_at;
    bool unlocked_hint;
    bool locked;
    bool pin_locked;

    int8_t door_offset;
    bool doors_closing;
} DesktopViewLockedModel;

void desktop_view_locked_set_callback(
    DesktopViewLocked* locked_view,
    DesktopViewLockedCallback callback,
    void* context) {
    furi_assert(locked_view);
    furi_assert(callback);
    locked_view->callback = callback;
    locked_view->context = context;
}

static void locked_view_timer_callback(TimerHandle_t timer) {
    DesktopViewLocked* locked_view = pvTimerGetTimerID(timer);
    locked_view->callback(DesktopLockedEventUpdate, locked_view->context);
}

static void desktop_view_locked_doors_draw(Canvas* canvas, DesktopViewLockedModel* model) {
    int8_t offset = model->door_offset;
    uint8_t door_left_x = DOOR_L_FINAL_POS + offset;
    uint8_t door_right_x = DOOR_R_FINAL_POS - offset;
    uint8_t height = icon_get_height(&I_DoorLeft_70x55);
    canvas_draw_icon(canvas, door_left_x, canvas_height(canvas) - height, &I_DoorLeft_70x55);
    canvas_draw_icon(canvas, door_right_x, canvas_height(canvas) - height, &I_DoorRight_70x55);
}

static bool desktop_view_locked_doors_move(DesktopViewLockedModel* model) {
    bool stop = false;
    if(model->door_offset < DOOR_OFFSET_END) {
        model->door_offset = CLAMP(model->door_offset + 5, DOOR_OFFSET_END, DOOR_OFFSET_START);
        stop = true;
    }

    return stop;
}

static void desktop_view_locked_update_hint_icon_timeout(DesktopViewLocked* locked_view) {
    DesktopViewLockedModel* model = view_get_model(locked_view->view);
    model->hint_icon_expire_at = osKernelGetTickCount() + osKernelGetTickFreq();
    view_commit_model(locked_view->view, true);
}

void desktop_view_locked_update(DesktopViewLocked* locked_view) {
    bool stop_timer = false;

    DesktopViewLockedModel* model = view_get_model(locked_view->view);
    if(model->locked) {
        model->doors_closing = desktop_view_locked_doors_move(model);
        stop_timer = !model->doors_closing;
    } else {
        model->unlocked_hint = false;
        stop_timer = true;
    }
    view_commit_model(locked_view->view, true);

    if(stop_timer) {
        xTimerStop(locked_view->timer, portMAX_DELAY);
    }
}

static void desktop_view_locked_draw(Canvas* canvas, void* model) {
    DesktopViewLockedModel* m = model;
    uint32_t now = osKernelGetTickCount();
    canvas_set_color(canvas, ColorBlack);

    if(m->locked) {
        if(m->doors_closing) {
            desktop_view_locked_doors_draw(canvas, m);
            canvas_set_font(canvas, FontPrimary);
            elements_multiline_text_framed(canvas, 42, 30 + STATUS_BAR_Y_SHIFT, "Locked");
        } else if((now < m->hint_icon_expire_at) && !m->pin_locked) {
            canvas_set_font(canvas, FontSecondary);
            elements_bold_rounded_frame(canvas, 14, 2 + STATUS_BAR_Y_SHIFT, 99, 48);
            elements_multiline_text(canvas, 65, 20 + STATUS_BAR_Y_SHIFT, "To unlock\npress:");
            canvas_draw_icon(canvas, 65, 36 + STATUS_BAR_Y_SHIFT, &I_Back3_45x8);
            canvas_draw_icon(canvas, 16, 7 + STATUS_BAR_Y_SHIFT, &I_WarningDolphin_45x42);
            canvas_draw_dot(canvas, 17, 61);
        }
    } else {
        if(m->unlocked_hint) {
            canvas_set_font(canvas, FontPrimary);
            elements_multiline_text_framed(canvas, 42, 30 + STATUS_BAR_Y_SHIFT, "Unlocked");
        }
    }
}

View* desktop_view_locked_get_view(DesktopViewLocked* locked_view) {
    furi_assert(locked_view);
    return locked_view->view;
}

static bool desktop_view_locked_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    DesktopViewLocked* locked_view = context;
    bool locked = false;
    bool locked_with_pin = false;
    bool doors_closing = false;
    uint32_t press_time = xTaskGetTickCount();

    {
        DesktopViewLockedModel* model = view_get_model(locked_view->view);
        bool changed = false;
        locked = model->locked;
        locked_with_pin = model->pin_locked;
        doors_closing = model->doors_closing;
        if(!locked && model->unlocked_hint && event->type == InputTypePress) {
            model->unlocked_hint = false;
            changed = true;
        }
        view_commit_model(locked_view->view, changed);
    }

    if(!locked || doors_closing || (event->type != InputTypeShort)) {
        return locked;
    }

    if(locked_with_pin) {
        locked_view->callback(DesktopLockedEventShowPinInput, locked_view->context);
    } else {
        if(press_time - locked_view->lock_lastpress > UNLOCK_RST_TIMEOUT) {
            locked_view->lock_lastpress = press_time;
            locked_view->lock_count = 0;
        }

        desktop_view_locked_update_hint_icon_timeout(locked_view);
        if(event->key == InputKeyBack) {
            locked_view->lock_lastpress = press_time;
            locked_view->lock_count++;
            if(locked_view->lock_count == UNLOCK_CNT) {
                desktop_view_locked_unlock(locked_view);
                locked_view->callback(DesktopLockedEventUnlocked, locked_view->context);
            }
        } else {
            locked_view->lock_count = 0;
        }

        locked_view->lock_lastpress = press_time;
    }

    return locked;
}

DesktopViewLocked* desktop_view_locked_alloc() {
    DesktopViewLocked* locked_view = furi_alloc(sizeof(DesktopViewLocked));
    locked_view->view = view_alloc();
    locked_view->timer =
        xTimerCreate(NULL, 1000 / 16, pdTRUE, locked_view, locked_view_timer_callback);

    locked_view->view = view_alloc();
    view_allocate_model(locked_view->view, ViewModelTypeLocking, sizeof(DesktopViewLockedModel));
    view_set_context(locked_view->view, locked_view);
    view_set_draw_callback(locked_view->view, desktop_view_locked_draw);
    view_set_input_callback(locked_view->view, desktop_view_locked_input);

    return locked_view;
}

void desktop_view_locked_free(DesktopViewLocked* locked_view) {
    furi_assert(locked_view);
    osTimerDelete(locked_view->timer);
    view_free(locked_view->view);
    free(locked_view);
}

void desktop_view_locked_close_doors(DesktopViewLocked* locked_view) {
    DesktopViewLockedModel* model = view_get_model(locked_view->view);
    model->doors_closing = true;
    model->door_offset = DOOR_OFFSET_START;
    view_commit_model(locked_view->view, true);
    xTimerChangePeriod(locked_view->timer, pdMS_TO_TICKS(DOOR_MOVING_INTERVAL_MS), portMAX_DELAY);
}

void desktop_view_locked_lock(DesktopViewLocked* locked_view, bool pin_locked) {
    DesktopViewLockedModel* model = view_get_model(locked_view->view);
    model->locked = true;
    model->pin_locked = pin_locked;
    view_commit_model(locked_view->view, true);
}

void desktop_view_locked_unlock(DesktopViewLocked* locked_view) {
    furi_assert(locked_view);

    locked_view->lock_count = 0;
    DesktopViewLockedModel* model = view_get_model(locked_view->view);
    model->locked = false;
    model->pin_locked = false;
    model->unlocked_hint = true;
    view_commit_model(locked_view->view, true);
    xTimerChangePeriod(locked_view->timer, pdMS_TO_TICKS(UNLOCKED_HINT_TIMEOUT_MS), portMAX_DELAY);
}
