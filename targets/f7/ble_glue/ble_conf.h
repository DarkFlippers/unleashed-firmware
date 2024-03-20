#pragma once

#include "app_conf.h"

/**
 * We're not using WPAN's event dispatchers
 * so both client & service max callback count is set to 0.
 */
#define BLE_CFG_SVC_MAX_NBR_CB 0

#define BLE_CFG_CLT_MAX_NBR_CB 0
