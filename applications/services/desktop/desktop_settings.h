#pragma once

#include <stdint.h>

typedef enum {
    FavoriteAppLeftShort,
    FavoriteAppLeftLong,
    FavoriteAppRightShort,
    FavoriteAppRightLong,
    FavoriteAppNumber,
} FavoriteAppShortcut;

typedef enum {
    DummyAppLeft,
    DummyAppRight,
    DummyAppDown,
    DummyAppOk,
    DummyAppNumber,
} DummyAppShortcut;

typedef struct {
    char name_or_path[128];
} FavoriteApp;

typedef struct {
    uint32_t auto_lock_delay_ms;
    uint8_t dummy_mode;
    uint8_t display_clock;
    FavoriteApp favorite_apps[FavoriteAppNumber];
    FavoriteApp dummy_apps[DummyAppNumber];
} DesktopSettings;

void desktop_settings_load(DesktopSettings* settings);
void desktop_settings_save(const DesktopSettings* settings);
