#pragma once

#include <desktop/desktop_settings.h>
#include "../views/desktop_events.h"
#include <gui/view.h>

typedef struct DesktopViewLocked DesktopViewLocked;

typedef void (*DesktopViewLockedCallback)(DesktopEvent event, void* context);

void desktop_view_locked_set_callback(
    DesktopViewLocked* locked_view,
    DesktopViewLockedCallback callback,
    void* context);
void desktop_view_locked_update(DesktopViewLocked* locked_view);
View* desktop_view_locked_get_view(DesktopViewLocked* locked_view);
DesktopViewLocked* desktop_view_locked_alloc();
void desktop_view_locked_free(DesktopViewLocked* locked_view);
void desktop_view_locked_lock(DesktopViewLocked* locked_view, bool pin_locked);
void desktop_view_locked_unlock(DesktopViewLocked* locked_view);
void desktop_view_locked_close_doors(DesktopViewLocked* locked_view);
bool desktop_view_locked_is_locked_hint_visible(DesktopViewLocked* locked_view);
