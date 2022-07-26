#pragma once

#include "bt_settings_filename.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool enabled;
} BtSettings;

bool bt_settings_load(BtSettings* bt_settings);

bool bt_settings_save(BtSettings* bt_settings);
