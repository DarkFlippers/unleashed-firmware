#include <furi.h>
#include "../desktop_i.h"
#include "desktop_locked.h"

void desktop_locked_set_callback(
    DesktopLockedView* locked_view,
    DesktopLockedViewCallback callback,
    void* context) {
    furi_assert(locked_view);
    furi_assert(callback);
    locked_view->callback = callback;
    locked_view->context = context;
}

void locked_view_timer_callback(void* context) {
    DesktopLockedView* locked_view = context;
    locked_view->callback(DesktopLockedEventUpdate, locked_view->context);
}

void desktop_locked_update_hint_timeout(DesktopLockedView* locked_view) {
    with_view_model(
        locked_view->view, (DesktopLockedViewModel * model) {
            model->hint_expire_at = osKernelGetTickCount() + osKernelGetTickFreq();
            return true;
        });
}

void desktop_locked_reset_door_pos(DesktopLockedView* locked_view) {
    with_view_model(
        locked_view->view, (DesktopLockedViewModel * model) {
            model->animation_seq_end = false;
            model->door_left_x = DOOR_L_POS;
            model->door_right_x = DOOR_R_POS;
            return true;
        });
}

void desktop_locked_manage_redraw(DesktopLockedView* locked_view) {
    bool animation_seq_end;

    with_view_model(
        locked_view->view, (DesktopLockedViewModel * model) {
            model->animation_seq_end = !model->door_left_x;
            animation_seq_end = model->animation_seq_end;

            if(!model->animation_seq_end) {
                model->door_left_x = CLAMP(model->door_left_x + 5, DOOR_L_POS_MAX, DOOR_L_POS);
                model->door_right_x = CLAMP(model->door_right_x - 5, DOOR_R_POS, DOOR_R_POS_MIN);
            } else {
                model->hint_expire_at = !model->hint_expire_at;
            }

            return true;
        });

    if(animation_seq_end) {
        osTimerStop(locked_view->timer);
    }
}

void desktop_locked_reset_counter(DesktopLockedView* locked_view) {
    locked_view->lock_count = 0;
    locked_view->lock_lastpress = 0;

    with_view_model(
        locked_view->view, (DesktopLockedViewModel * model) {
            model->hint_expire_at = 0;
            return true;
        });
}

void desktop_locked_with_pin(DesktopLockedView* locked_view, bool locked) {
    with_view_model(
        locked_view->view, (DesktopLockedViewModel * model) {
            model->pin_lock = locked;
            return true;
        });
}

void desktop_locked_render(Canvas* canvas, void* model) {
    DesktopLockedViewModel* m = model;
    uint32_t now = osKernelGetTickCount();
    canvas_set_color(canvas, ColorBlack);

    if(!m->animation_seq_end) {
        canvas_draw_icon(canvas, m->door_left_x, 0 + STATUS_BAR_Y_SHIFT, &I_DoorLeft_70x55);
        canvas_draw_icon(canvas, m->door_right_x, 0 + STATUS_BAR_Y_SHIFT, &I_DoorRight_70x55);
    }

    if(m->animation && m->animation_seq_end) {
        if(m->status_bar_background_black) {
            canvas_draw_box(canvas, 0, 0, GUI_STATUS_BAR_WIDTH, GUI_STATUS_BAR_HEIGHT);
        }
        canvas_draw_icon_animation(canvas, 0, 0 + STATUS_BAR_Y_SHIFT, m->animation);
    }

    if(now < m->hint_expire_at) {
        if(!m->animation_seq_end) {
            canvas_set_font(canvas, FontPrimary);
            elements_multiline_text_framed(canvas, 42, 30 + STATUS_BAR_Y_SHIFT, "Locked");

        } else if(!m->pin_lock) {
            canvas_set_font(canvas, FontSecondary);
            canvas_draw_icon(canvas, 13, 2 + STATUS_BAR_Y_SHIFT, &I_LockPopup_100x49);
            elements_multiline_text(canvas, 65, 20 + STATUS_BAR_Y_SHIFT, "To unlock\npress:");
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

    uint32_t press_time = 0;
    bool locked_with_pin = false;

    with_view_model(
        locked_view->view, (DesktopLockedViewModel * model) {
            locked_with_pin = model->pin_lock;
            return false;
        });

    if(event->type == InputTypeShort) {
        if(locked_with_pin) {
            press_time = osKernelGetTickCount();

            if(press_time - locked_view->lock_lastpress > UNLOCK_RST_TIMEOUT * 3) {
                locked_view->lock_lastpress = press_time;
                locked_view->callback(DesktopLockedEventInputReset, locked_view->context);
            }

            locked_view->callback(event->key, locked_view->context);
        } else {
            desktop_locked_update_hint_timeout(locked_view);

            if(event->key == InputKeyBack) {
                press_time = osKernelGetTickCount();
                // check if pressed sequentially
                if(press_time - locked_view->lock_lastpress < UNLOCK_RST_TIMEOUT) {
                    locked_view->lock_lastpress = press_time;
                    locked_view->lock_count++;
                }

                if(locked_view->lock_count == UNLOCK_CNT) {
                    locked_view->lock_count = 0;
                    locked_view->callback(DesktopLockedEventUnlock, locked_view->context);
                }
            }
        }

        if(press_time - locked_view->lock_lastpress > UNLOCK_RST_TIMEOUT) {
            locked_view->lock_lastpress = press_time;
            locked_view->lock_count = 0;
        }
    }
    // All events consumed
    return true;
}

void desktop_locked_enter(void* context) {
    DesktopLockedView* locked_view = context;

    with_view_model(
        locked_view->view, (DesktopLockedViewModel * model) {
            if(model->animation) icon_animation_start(model->animation);
            return false;
        });
}

void desktop_locked_exit(void* context) {
    DesktopLockedView* locked_view = context;

    with_view_model(
        locked_view->view, (DesktopLockedViewModel * model) {
            if(model->animation) icon_animation_stop(model->animation);
            return false;
        });
}

DesktopLockedView* desktop_locked_alloc() {
    DesktopLockedView* locked_view = furi_alloc(sizeof(DesktopLockedView));
    locked_view->view = view_alloc();
    locked_view->timer =
        osTimerNew(locked_view_timer_callback, osTimerPeriodic, locked_view, NULL);

    view_allocate_model(locked_view->view, ViewModelTypeLocking, sizeof(DesktopLockedViewModel));
    view_set_context(locked_view->view, locked_view);
    view_set_draw_callback(locked_view->view, (ViewDrawCallback)desktop_locked_render);
    view_set_input_callback(locked_view->view, desktop_locked_input);
    view_set_enter_callback(locked_view->view, desktop_locked_enter);
    view_set_exit_callback(locked_view->view, desktop_locked_exit);

    return locked_view;
}

void desktop_locked_free(DesktopLockedView* locked_view) {
    furi_assert(locked_view);
    osTimerDelete(locked_view->timer);
    view_free(locked_view->view);
    free(locked_view);
}
