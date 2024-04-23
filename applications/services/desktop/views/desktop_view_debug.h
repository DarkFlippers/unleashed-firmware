#pragma once

#include <stdint.h>
#include <gui/view.h>
#include "desktop_events.h"

typedef struct DesktopDebugView DesktopDebugView;

typedef void (*DesktopDebugViewCallback)(DesktopEvent event, void* context);

struct DesktopDebugView {
    View* view;
    FuriTimer* timer;
    DesktopDebugViewCallback callback;
    void* context;
};

void desktop_debug_set_callback(
    DesktopDebugView* debug_view,
    DesktopDebugViewCallback callback,
    void* context);

View* desktop_debug_get_view(DesktopDebugView* debug_view);

DesktopDebugView* desktop_debug_alloc(void);

void desktop_debug_free(DesktopDebugView* debug_view);
