#pragma once

#include <math.h>
#include <furi.h>
#include <furi_hal.h>
#include <lib/flipper_format/flipper_format.h>

typedef struct {
    string_t preset_name;
    uint32_t frequency;
    uint32_t hopping;
    //uint32_t detect_raw;
    int32_t rssi_threshold;
} SubGhzLastSetting;

SubGhzLastSetting* subghz_last_setting_alloc(void);

void subghz_last_setting_free(SubGhzLastSetting* instance);

void subghz_last_setting_load(SubGhzLastSetting* instance, const char* file_path);

bool subghz_last_setting_save(SubGhzLastSetting* instance, const char* file_path);

void subghz_last_setting_set_receiver_values(SubGhzLastSetting* instance, SubGhzReceiver* receiver);