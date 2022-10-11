#pragma once

// Enable saving detect raw setting state
// #define SUBGHZ_SAVE_DETECT_RAW_SETTING 1

#include <furi_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <storage/storage.h>
#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
#include <lib/subghz/protocols/base.h>

#define DETECT_RAW_FALSE SubGhzProtocolFlag_Decodable
#define DETECT_RAW_TRUE SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_RAW
#endif
typedef struct {
    uint32_t frequency;
#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
    uint32_t detect_raw;
#endif
    int32_t preset;
    uint32_t frequency_analyzer_feedback_level;
} SubGhzLastSettings;

SubGhzLastSettings* subghz_last_settings_alloc(void);

void subghz_last_settings_free(SubGhzLastSettings* instance);

void subghz_last_settings_load(SubGhzLastSettings* instance, size_t preset_count);

bool subghz_last_settings_save(SubGhzLastSettings* instance);
#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
void subghz_last_settings_set_detect_raw_values(void* context);
#endif