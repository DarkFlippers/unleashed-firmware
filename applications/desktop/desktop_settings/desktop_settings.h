#pragma once

#include <stdint.h>
#include <stdbool.h>

#define DESKTOP_SETTINGS_VER (1)
#define PIN_MAX_LENGTH 12

typedef struct {
    uint8_t length;
    uint8_t data[PIN_MAX_LENGTH];
} PinCode;

typedef struct {
    uint8_t version;
    uint16_t favorite;

    PinCode pincode;
    bool locked;
} DesktopSettings;

bool desktop_settings_load(DesktopSettings* desktop_settings);

bool desktop_settings_save(DesktopSettings* desktop_settings);
