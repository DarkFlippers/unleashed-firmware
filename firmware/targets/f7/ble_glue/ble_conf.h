#pragma once

#include "app_conf.h"

/**
 * There is one handler per service enabled
 * Note: There is no handler for the Device Information Service
 *
 * This shall take into account all registered handlers
 * (from either the provided services or the custom services)
 */
#define BLE_CFG_SVC_MAX_NBR_CB 7

#define BLE_CFG_CLT_MAX_NBR_CB 0
