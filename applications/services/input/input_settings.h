#pragma once

#include <stdint.h>

typedef struct {
    uint8_t vibro_touch_level;
    uint8_t vibro_touch_trigger_mask;
} InputSettings;

#ifdef __cplusplus
extern "C" {
#endif

void input_settings_load(InputSettings* settings);
void input_settings_save(const InputSettings* settings);

#ifdef __cplusplus
}
#endif
