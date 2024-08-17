#pragma once

#include "bt.h"
#include "../bt_settings.h"

void bt_get_settings(Bt* bt, BtSettings* settings);

void bt_set_settings(Bt* bt, const BtSettings* settings);
