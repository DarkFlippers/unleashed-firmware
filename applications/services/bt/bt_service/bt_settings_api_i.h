#pragma once

#include "bt.h"
#include "../bt_settings.h"

#ifdef __cplusplus
extern "C" {
#endif

void bt_get_settings(Bt* bt, BtSettings* settings);

void bt_set_settings(Bt* bt, const BtSettings* settings);

#ifdef __cplusplus
}
#endif
