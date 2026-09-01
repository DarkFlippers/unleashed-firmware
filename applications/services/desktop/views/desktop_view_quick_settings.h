#pragma once

#include <gui/view.h>
#include "desktop_events.h"

// 0% to 100% in 5% steps, the same grid the LCD & Notifications settings use
#define DESKTOP_QUICK_SETTINGS_LEVELS 21

typedef struct DesktopQuickSettingsView DesktopQuickSettingsView;

typedef void (*DesktopQuickSettingsViewCallback)(DesktopEvent event, void* context);

struct DesktopQuickSettingsView {
    View* view;
    DesktopQuickSettingsViewCallback callback;
    void* context;
};

typedef struct {
    uint8_t idx;
    bool editing;
    uint8_t brightness;
    uint8_t volume;
    bool vibro;
} DesktopQuickSettingsViewModel;

void desktop_quick_settings_set_callback(
    DesktopQuickSettingsView* quick_settings,
    DesktopQuickSettingsViewCallback callback,
    void* context);

View* desktop_quick_settings_get_view(DesktopQuickSettingsView* quick_settings);

/** Take the current values in and put the cursor back on the first item. */
void desktop_quick_settings_reset(
    DesktopQuickSettingsView* quick_settings,
    float brightness,
    float volume,
    bool vibro);

float desktop_quick_settings_get_brightness(DesktopQuickSettingsView* quick_settings);
float desktop_quick_settings_get_volume(DesktopQuickSettingsView* quick_settings);
bool desktop_quick_settings_get_vibro(DesktopQuickSettingsView* quick_settings);

DesktopQuickSettingsView* desktop_quick_settings_alloc(void);
void desktop_quick_settings_free(DesktopQuickSettingsView* quick_settings);
