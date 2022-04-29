#pragma once

#include <furi_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <toolbox/saved_struct.h>

#define DESKTOP_SETTINGS_VER (3)
#define DESKTOP_SETTINGS_PATH "/int/desktop.settings"
#define DESKTOP_SETTINGS_MAGIC (0x17)
#define PIN_MAX_LENGTH 12

#define DESKTOP_SETTINGS_RUN_PIN_SETUP_ARG "run_pin_setup"

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

#define MAX_PIN_SIZE 10
#define MIN_PIN_SIZE 4

typedef struct {
    InputKey data[MAX_PIN_SIZE];
    uint8_t length;
} PinCode;

typedef struct {
    uint16_t favorite;
    PinCode pin_code;
    uint8_t is_locked;
    uint32_t auto_lock_delay_ms;
} DesktopSettings;
