#include <furi.h>
#include <gui/elements.h>

#include "../desktop_i.h"
#include "desktop_view_lock_menu.h"

#define LOCK_MENU_ITEMS_NB 3

void desktop_lock_menu_set_callback(
    DesktopLockMenuView* lock_menu,
    DesktopLockMenuViewCallback callback,
    void* context) {
    furi_assert(lock_menu);
    furi_assert(callback);
    lock_menu->callback = callback;
    lock_menu->context = context;
}

void desktop_lock_menu_pin_set(DesktopLockMenuView* lock_menu, bool pin_is_set) {
    with_view_model(
        lock_menu->view, (DesktopLockMenuViewModel * model) {
            model->pin_set = pin_is_set;
            return true;
        });
}

void desktop_lock_menu_set_idx(DesktopLockMenuView* lock_menu, uint8_t idx) {
    furi_assert(idx < LOCK_MENU_ITEMS_NB);
    with_view_model(
        lock_menu->view, (DesktopLockMenuViewModel * model) {
            model->idx = idx;
            return true;
        });
}

static void lock_menu_callback(void* context, uint8_t index) {
    furi_assert(context);
    DesktopLockMenuView* lock_menu = context;
    switch(index) {
    case 0: // lock
        lock_menu->callback(DesktopLockMenuEventLock, lock_menu->context);
        break;
    case 1: // lock
        lock_menu->callback(DesktopLockMenuEventPinLock, lock_menu->context);
        break;
    default: // wip message
        with_view_model(
            lock_menu->view, (DesktopLockMenuViewModel * model) {
                model->hint_timeout = HINT_TIMEOUT;
                return true;
            });
        break;
    }
}

void desktop_lock_menu_render(Canvas* canvas, void* model) {
    const char* Lockmenu_Items[LOCK_MENU_ITEMS_NB] = {"Lock", "Lock with PIN", "DUMB mode"};

    DesktopLockMenuViewModel* m = model;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon(canvas, -57, 0 + STATUS_BAR_Y_SHIFT, &I_DoorLeft_70x55);
    canvas_draw_icon(canvas, 116, 0 + STATUS_BAR_Y_SHIFT, &I_DoorRight_70x55);
    canvas_set_font(canvas, FontSecondary);

    for(uint8_t i = 0; i < LOCK_MENU_ITEMS_NB; ++i) {
        const char* str = Lockmenu_Items[i];

        if(i == 1 && !m->pin_set) str = "Set PIN";
        if(m->hint_timeout && m->idx == 2 && m->idx == i) str = "Not Implemented";

        if(str != NULL)
            canvas_draw_str_aligned(
                canvas, 64, 9 + (i * 17) + STATUS_BAR_Y_SHIFT, AlignCenter, AlignCenter, str);

        if(m->idx == i) elements_frame(canvas, 15, 1 + (i * 17) + STATUS_BAR_Y_SHIFT, 98, 15);
    }
}

View* desktop_lock_menu_get_view(DesktopLockMenuView* lock_menu) {
    furi_assert(lock_menu);
    return lock_menu->view;
}

bool desktop_lock_menu_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopLockMenuView* lock_menu = context;
    uint8_t idx;

    if(event->type != InputTypeShort) return false;
    with_view_model(
        lock_menu->view, (DesktopLockMenuViewModel * model) {
            model->hint_timeout = 0; // clear hint timeout
            if(event->key == InputKeyUp) {
                model->idx = CLAMP(model->idx - 1, LOCK_MENU_ITEMS_NB - 1, 0);
            } else if(event->key == InputKeyDown) {
                model->idx = CLAMP(model->idx + 1, LOCK_MENU_ITEMS_NB - 1, 0);
            }
            idx = model->idx;
            return true;
        });

    if(event->key == InputKeyBack) {
        lock_menu->callback(DesktopLockMenuEventExit, lock_menu->context);
    } else if(event->key == InputKeyOk) {
        lock_menu_callback(lock_menu, idx);
    }

    return true;
}

DesktopLockMenuView* desktop_lock_menu_alloc() {
    DesktopLockMenuView* lock_menu = malloc(sizeof(DesktopLockMenuView));
    lock_menu->view = view_alloc();
    view_allocate_model(lock_menu->view, ViewModelTypeLocking, sizeof(DesktopLockMenuViewModel));
    view_set_context(lock_menu->view, lock_menu);
    view_set_draw_callback(lock_menu->view, (ViewDrawCallback)desktop_lock_menu_render);
    view_set_input_callback(lock_menu->view, desktop_lock_menu_input);

    return lock_menu;
}

void desktop_lock_menu_free(DesktopLockMenuView* lock_menu_view) {
    furi_assert(lock_menu_view);

    view_free(lock_menu_view->view);
    free(lock_menu_view);
}
