#pragma once

#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>
#include "desktop_events.h"

typedef struct DesktopFirstStartView DesktopFirstStartView;

typedef void (*DesktopFirstStartViewCallback)(DesktopEvent event, void* context);

DesktopFirstStartView* desktop_first_start_alloc();

void desktop_first_start_free(DesktopFirstStartView* main_view);

View* desktop_first_start_get_view(DesktopFirstStartView* main_view);

void desktop_first_start_set_callback(
    DesktopFirstStartView* main_view,
    DesktopFirstStartViewCallback callback,
    void* context);
