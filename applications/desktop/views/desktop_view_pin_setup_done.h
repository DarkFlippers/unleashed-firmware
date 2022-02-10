#pragma once

#include <gui/view.h>

typedef struct DesktopViewPinSetupDone DesktopViewPinSetupDone;

typedef void (*DesktopViewPinSetupDoneDoneCallback)(void*);

void desktop_view_pin_done_set_callback(
    DesktopViewPinSetupDone* instance,
    DesktopViewPinSetupDoneDoneCallback callback,
    void* context);
DesktopViewPinSetupDone* desktop_view_pin_done_alloc();
void desktop_view_pin_done_free(DesktopViewPinSetupDone* instance);
View* desktop_view_pin_done_get_view(DesktopViewPinSetupDone* instance);
