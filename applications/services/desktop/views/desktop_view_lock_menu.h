#pragma once

#include <gui/view.h>
#include "desktop_events.h"

#define HINT_TIMEOUT 2

typedef struct DesktopLockMenuView DesktopLockMenuView;

typedef void (*DesktopLockMenuViewCallback)(DesktopEvent event, void* context);

struct DesktopLockMenuView {
    View* view;
    DesktopLockMenuViewCallback callback;
    void* context;
};

typedef struct {
    uint8_t idx;
    bool pin_is_set;
    bool dummy_mode;
} DesktopLockMenuViewModel;

void desktop_lock_menu_set_callback(
    DesktopLockMenuView* lock_menu,
    DesktopLockMenuViewCallback callback,
    void* context);

View* desktop_lock_menu_get_view(DesktopLockMenuView* lock_menu);
void desktop_lock_menu_set_pin_state(DesktopLockMenuView* lock_menu, bool pin_is_set);
void desktop_lock_menu_set_dummy_mode_state(DesktopLockMenuView* lock_menu, bool dummy_mode);
void desktop_lock_menu_set_idx(DesktopLockMenuView* lock_menu, uint8_t idx);
DesktopLockMenuView* desktop_lock_menu_alloc();
void desktop_lock_menu_free(DesktopLockMenuView* lock_menu);
