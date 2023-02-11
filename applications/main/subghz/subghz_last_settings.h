#pragma once

#include <furi_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <storage/storage.h>

typedef struct {
    uint32_t frequency;
    int32_t preset;
    uint32_t frequency_analyzer_feedback_level;
    float frequency_analyzer_trigger;
} SubGhzLastSettings;

SubGhzLastSettings* subghz_last_settings_alloc(void);

void subghz_last_settings_free(SubGhzLastSettings* instance);

void subghz_last_settings_load(SubGhzLastSettings* instance, size_t preset_count);

bool subghz_last_settings_save(SubGhzLastSettings* instance);
