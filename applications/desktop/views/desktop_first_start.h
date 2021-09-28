#pragma once

#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>

typedef enum {
    DesktopFirstStartCompleted,
} DesktopFirstStartEvent;

typedef struct DesktopFirstStartView DesktopFirstStartView;

typedef void (*DesktopFirstStartViewCallback)(DesktopFirstStartEvent event, void* context);

struct DesktopFirstStartView {
    View* view;
    DesktopFirstStartViewCallback callback;
    void* context;
};

typedef struct {
    uint8_t page;
} DesktopFirstStartViewModel;

void desktop_first_start_set_callback(
    DesktopFirstStartView* main_view,
    DesktopFirstStartViewCallback callback,
    void* context);

View* desktop_first_start_get_view(DesktopFirstStartView* main_view);

DesktopFirstStartView* desktop_first_start_alloc();
void desktop_first_start_free(DesktopFirstStartView* main_view);
