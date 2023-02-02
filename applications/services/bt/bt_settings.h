#pragma once

#include "bt_settings_filename.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool enabled;
} BtSettings;

bool bt_settings_load(BtSettings* bt_settings);

bool bt_settings_save(BtSettings* bt_settings);

#ifdef __cplusplus
}
#endif