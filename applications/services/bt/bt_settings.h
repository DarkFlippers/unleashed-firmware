#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool enabled;
} BtSettings;

void bt_settings_load(BtSettings* bt_settings);

void bt_settings_save(const BtSettings* bt_settings);

#ifdef __cplusplus
}
#endif
