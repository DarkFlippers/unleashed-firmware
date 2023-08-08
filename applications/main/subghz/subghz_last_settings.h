#pragma once

#include <furi_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <storage/storage.h>
#include <lib/subghz/types.h>

#define SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_TRIGGER (-93.0f)

typedef struct {
    uint32_t frequency;
    uint32_t preset_index; // AKA Modulation
    uint32_t frequency_analyzer_feedback_level;
    float frequency_analyzer_trigger;
    // TODO not using but saved so as not to change the version
    bool external_module_enabled;
    bool external_module_power_5v_disable;
    bool external_module_power_amp;
    // saved so as not to change the version
    bool timestamp_file_names;
    bool enable_hopping;
    uint32_t ignore_filter;
    uint32_t filter;
    uint32_t sound;
    float rssi;
} SubGhzLastSettings;

SubGhzLastSettings* subghz_last_settings_alloc(void);

void subghz_last_settings_free(SubGhzLastSettings* instance);

void subghz_last_settings_load(SubGhzLastSettings* instance, size_t preset_count);

bool subghz_last_settings_save(SubGhzLastSettings* instance);

void subghz_last_settings_log(SubGhzLastSettings* instance);
