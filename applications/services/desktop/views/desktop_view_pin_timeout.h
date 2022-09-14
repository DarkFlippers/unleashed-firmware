#pragma once

#include <stdint.h>
#include <gui/view.h>

typedef void (*DesktopViewPinTimeoutDoneCallback)(void*);
typedef struct DesktopViewPinTimeout DesktopViewPinTimeout;

void desktop_view_pin_timeout_set_callback(
    DesktopViewPinTimeout* instance,
    DesktopViewPinTimeoutDoneCallback callback,
    void* context);
DesktopViewPinTimeout* desktop_view_pin_timeout_alloc(void);
void desktop_view_pin_timeout_free(DesktopViewPinTimeout*);
void desktop_view_pin_timeout_start(DesktopViewPinTimeout* instance, uint32_t time_left);
View* desktop_view_pin_timeout_get_view(DesktopViewPinTimeout* instance);
