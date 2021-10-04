#pragma once

#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>

#define HINT_TIMEOUT 2

typedef enum {
    DesktopLockMenuEventLock,
    DesktopLockMenuEventUnlock,
    DesktopLockMenuEventExit,
} DesktopLockMenuEvent;

typedef struct DesktopLockMenuView DesktopLockMenuView;

typedef void (*DesktopLockMenuViewCallback)(DesktopLockMenuEvent event, void* context);

struct DesktopLockMenuView {
    View* view;
    DesktopLockMenuViewCallback callback;
    void* context;
};

typedef struct {
    uint8_t idx;
    uint8_t hint_timeout;
} DesktopLockMenuViewModel;

void desktop_lock_menu_set_callback(
    DesktopLockMenuView* lock_menu,
    DesktopLockMenuViewCallback callback,
    void* context);

View* desktop_lock_menu_get_view(DesktopLockMenuView* lock_menu);
void desktop_lock_menu_reset_idx(DesktopLockMenuView* lock_menu);
DesktopLockMenuView* desktop_lock_menu_alloc();
void desktop_lock_menu_free(DesktopLockMenuView* lock_menu);
