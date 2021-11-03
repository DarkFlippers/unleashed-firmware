#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <toolbox/saved_struct.h>

#define DESKTOP_SETTINGS_VER (1)
#define DESKTOP_SETTINGS_PATH "/int/desktop.settings"
#define DESKTOP_SETTINGS_MAGIC (0x17)
#define PIN_MAX_LENGTH 12

#define SAVE_DESKTOP_SETTINGS(x) \
    saved_struct_save(           \
        DESKTOP_SETTINGS_PATH,   \
        (x),                     \
        sizeof(DesktopSettings), \
        DESKTOP_SETTINGS_MAGIC,  \
        DESKTOP_SETTINGS_VER)

#define LOAD_DESKTOP_SETTINGS(x) \
    saved_struct_load(           \
        DESKTOP_SETTINGS_PATH,   \
        (x),                     \
        sizeof(DesktopSettings), \
        DESKTOP_SETTINGS_MAGIC,  \
        DESKTOP_SETTINGS_VER)

typedef struct {
    uint8_t length;
    uint8_t data[PIN_MAX_LENGTH];
} PinCode;

typedef struct {
    uint16_t favorite;
    PinCode pincode;
} DesktopSettings;
