#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Device information service. 
 * Holds Flipper name, version and other information.
 */

typedef struct BleServiceDevInfo BleServiceDevInfo;

BleServiceDevInfo* ble_svc_dev_info_start(void);

void ble_svc_dev_info_stop(BleServiceDevInfo* service);

#ifdef __cplusplus
}
#endif
