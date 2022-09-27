#pragma once

#include "subghz_last_settings_filename.h"

#include <furi_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <toolbox/saved_struct.h>
#include <storage/storage.h>

#define SUBGHZ_LAST_SETTINGS_VER (1)
#define SUBGHZ_LAST_SETTINGS_PATH EXT_PATH(SUBGHZ_LAST_SETTINGS_FILE_NAME)
#define SUBGHZ_LAST_SETTINGS_MAGIC (0xCC)

#define SAVE_SUBGHZ_LAST_SETTINGS(x) \
    saved_struct_save(               \
        SUBGHZ_LAST_SETTINGS_PATH,   \
        (x),                         \
        sizeof(SubGhzLastSettings),  \
        SUBGHZ_LAST_SETTINGS_MAGIC,  \
        SUBGHZ_LAST_SETTINGS_VER)

#define LOAD_SUBGHZ_LAST_SETTINGS(x) \
    saved_struct_load(               \
        SUBGHZ_LAST_SETTINGS_PATH,   \
        (x),                         \
        sizeof(SubGhzLastSettings),  \
        SUBGHZ_LAST_SETTINGS_MAGIC,  \
        SUBGHZ_LAST_SETTINGS_VER)

typedef struct {
    uint32_t frequency;
    uint8_t preset;
} SubGhzLastSettings;

void subghz_last_settings_check_struct(void);