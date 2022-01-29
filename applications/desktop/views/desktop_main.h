#pragma once

#include <gui/view.h>
#include "desktop_events.h"

typedef struct DesktopMainView DesktopMainView;

typedef void (*DesktopMainViewCallback)(DesktopEvent event, void* context);

void desktop_main_set_callback(
    DesktopMainView* main_view,
    DesktopMainViewCallback callback,
    void* context);

View* desktop_main_get_view(DesktopMainView* main_view);
DesktopMainView* desktop_main_alloc();
void desktop_main_free(DesktopMainView* main_view);
