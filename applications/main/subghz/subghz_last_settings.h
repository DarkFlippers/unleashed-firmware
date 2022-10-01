#pragma once

#include <furi_hal.h>
#include <stdint.h>
#include <stdbool.h>
#include <storage/storage.h>
#include <lib/subghz/protocols/base.h>

#define DETECT_RAW_FALSE SubGhzProtocolFlag_Decodable
#define DETECT_RAW_TRUE SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_RAW

typedef struct {
    uint32_t frequency;
    uint32_t detect_raw;
    int32_t preset;
} SubGhzLastSettings;

SubGhzLastSettings* subghz_last_settings_alloc(void);

void subghz_last_settings_free(SubGhzLastSettings* instance);

void subghz_last_settings_load(SubGhzLastSettings* instance, size_t preset_count);

bool subghz_last_settings_save(SubGhzLastSettings* instance);

void subghz_last_settings_set_detect_raw_values(void* context);