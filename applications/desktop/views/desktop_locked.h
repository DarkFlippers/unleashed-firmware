#pragma once

#include <desktop/desktop_settings/desktop_settings.h>
#include <gui/view.h>
#include "desktop_events.h"

#define UNLOCK_RST_TIMEOUT 300
#define UNLOCK_CNT 3

#define DOOR_L_POS -57
#define DOOR_L_POS_MAX 0
#define DOOR_R_POS 115
#define DOOR_R_POS_MIN 60

typedef enum {
    DesktopLockedWithPin,
    DesktopLockedNoPin,
} DesktopLockedSceneState;

typedef struct DesktopLockedView DesktopLockedView;

typedef void (*DesktopLockedViewCallback)(DesktopEvent event, void* context);

void desktop_locked_set_callback(
    DesktopLockedView* locked_view,
    DesktopLockedViewCallback callback,
    void* context);

void desktop_locked_update(DesktopLockedView* locked_view);

View* desktop_locked_get_view(DesktopLockedView* locked_view);
DesktopLockedView* desktop_locked_alloc();
void desktop_locked_free(DesktopLockedView* locked_view);

void desktop_locked_lock_pincode(DesktopLockedView* locked_view, PinCode pincode);
void desktop_locked_lock(DesktopLockedView* locked_view);
