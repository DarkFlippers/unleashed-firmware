#pragma once

#include <stdint.h>
#include <gui/view.h>
#include "desktop_events.h"

typedef struct DesktopDebugView DesktopDebugView;

typedef void (*DesktopDebugViewCallback)(DesktopEvent event, void* context);

// Debug info
typedef enum {
    DesktopViewStatsFw,
    DesktopViewStatsMeta,
    DesktopViewStatsTotalCount,
} DesktopViewStatsScreens;

struct DesktopDebugView {
    View* view;
    DesktopDebugViewCallback callback;
    void* context;
};

typedef struct {
    uint32_t icounter;
    uint32_t butthurt;
    uint64_t timestamp;
    DesktopViewStatsScreens screen;
} DesktopDebugViewModel;

void desktop_debug_set_callback(
    DesktopDebugView* debug_view,
    DesktopDebugViewCallback callback,
    void* context);

View* desktop_debug_get_view(DesktopDebugView* debug_view);

DesktopDebugView* desktop_debug_alloc();
void desktop_debug_free(DesktopDebugView* debug_view);

void desktop_debug_get_dolphin_data(DesktopDebugView* debug_view);
void desktop_debug_reset_screen_idx(DesktopDebugView* debug_view);
