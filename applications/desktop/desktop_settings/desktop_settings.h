#pragma once

#include <stdint.h>
#include <stdbool.h>

#define DESKTOP_SETTINGS_VER (0)

typedef struct {
    uint8_t version;
    uint16_t favorite;
} DesktopSettings;

bool desktop_settings_load(DesktopSettings* desktop_settings);

bool desktop_settings_save(DesktopSettings* desktop_settings);
