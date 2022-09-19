#pragma once

#include "desktop_settings_filename.h"

#include <furi_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <toolbox/saved_struct.h>
#include <storage/storage.h>

#define DESKTOP_SETTINGS_VER (5)

#define DESKTOP_SETTINGS_PATH INT_PATH(DESKTOP_SETTINGS_FILE_NAME)
#define DESKTOP_SETTINGS_MAGIC (0x17)
#define PIN_MAX_LENGTH 12

#define DESKTOP_SETTINGS_RUN_PIN_SETUP_ARG "run_pin_setup"

#define DESKTOP_SETTINGS_SAVE(x) \
    saved_struct_save(           \
        DESKTOP_SETTINGS_PATH,   \
        (x),                     \
        sizeof(DesktopSettings), \
        DESKTOP_SETTINGS_MAGIC,  \
        DESKTOP_SETTINGS_VER)

#define DESKTOP_SETTINGS_LOAD(x) \
    saved_struct_load(           \
        DESKTOP_SETTINGS_PATH,   \
        (x),                     \
        sizeof(DesktopSettings), \
        DESKTOP_SETTINGS_MAGIC,  \
        DESKTOP_SETTINGS_VER)

#define MAX_PIN_SIZE 10
#define MIN_PIN_SIZE 4

typedef struct {
    InputKey data[MAX_PIN_SIZE];
    uint8_t length;
} PinCode;

typedef struct {
    uint16_t favorite_primary;
    uint16_t favorite_secondary;
    PinCode pin_code;
    uint8_t is_locked;
    uint32_t auto_lock_delay_ms;
    uint8_t dummy_mode;
} DesktopSettings;
