#pragma once

#include <gui/gui_i.h>
#include <gui/view.h>
#include <gui/canvas.h>
#include <gui/elements.h>
#include <furi.h>

typedef enum {
    DesktopHwMismatchEventExit,
} DesktopHwMismatchEvent;

typedef struct DesktopHwMismatchView DesktopHwMismatchView;

typedef void (*DesktopHwMismatchViewCallback)(DesktopHwMismatchEvent event, void* context);

struct DesktopHwMismatchView {
    View* view;
    DesktopHwMismatchViewCallback callback;
    void* context;
};

typedef struct {
    IconAnimation* animation;
    uint8_t scene_num;
    uint8_t hint_timeout;
    bool locked;
} DesktopHwMismatchViewModel;

void desktop_hw_mismatch_set_callback(
    DesktopHwMismatchView* hw_mismatch_view,
    DesktopHwMismatchViewCallback callback,
    void* context);

View* desktop_hw_mismatch_get_view(DesktopHwMismatchView* hw_mismatch_view);

DesktopHwMismatchView* desktop_hw_mismatch_alloc();
void desktop_hw_mismatch_free(DesktopHwMismatchView* hw_mismatch_view);
