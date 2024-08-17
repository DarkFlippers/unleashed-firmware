#pragma once

#include "app_conf.h"

/**
 * We're not using WPAN's event dispatchers
 * so both client & service max callback count is set to 0.
 */
#define BLE_CFG_SVC_MAX_NBR_CB 0

#define BLE_CFG_CLT_MAX_NBR_CB 0

/* Various defines for compatibility with -Wundef - thanks, ST */
#define BLE_CFG_BLS_INTERMEDIATE_CUFF_PRESSURE    0
#define BLE_CFG_BLS_TIME_STAMP_FLAG               0
#define BLE_CFG_BLS_PULSE_RATE_FLAG               0
#define BLE_CFG_BLS_USER_ID_FLAG                  0
#define BLE_CFG_BLS_MEASUREMENT_STATUS_FLAG       0
#define BLE_CFG_HRS_ENERGY_EXPENDED_INFO_FLAG     0
#define BLE_CFG_HRS_ENERGY_RR_INTERVAL_FLAG       0
#define BLE_CFG_HTS_MEASUREMENT_INTERVAL          0
#define BLE_CFG_HTS_TIME_STAMP_FLAG               0
#define BLE_CFG_HTS_TEMPERATURE_TYPE_VALUE_STATIC 0
