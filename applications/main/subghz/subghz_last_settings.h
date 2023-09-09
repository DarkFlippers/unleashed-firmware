#pragma once

#include <furi_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <storage/storage.h>
#include <lib/subghz/types.h>

#define SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_TRIGGER (-93.0f)
#define SUBGHZ_LAST_SETTING_SAVE_BIN_RAW true
#define SUBGHZ_LAST_SETTING_SAVE_PRESET true
// 1 = "AM650"
// "AM270", "AM650", "FM238", "FM476",
#define SUBGHZ_LAST_SETTING_DEFAULT_PRESET 1
#define SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY 433920000
#define SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_FEEDBACK_LEVEL 2

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
    float rssi;
} SubGhzLastSettings;

SubGhzLastSettings* subghz_last_settings_alloc(void);

void subghz_last_settings_free(SubGhzLastSettings* instance);

void subghz_last_settings_load(SubGhzLastSettings* instance, size_t preset_count);

bool subghz_last_settings_save(SubGhzLastSettings* instance);

void subghz_last_settings_log(SubGhzLastSettings* instance);
