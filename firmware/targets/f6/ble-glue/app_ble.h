#pragma once 

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "hci_tl.h"

typedef enum {
    APP_BLE_IDLE,
    APP_BLE_FAST_ADV,
    APP_BLE_LP_ADV,
    APP_BLE_SCAN,
    APP_BLE_LP_CONNECTING,
    APP_BLE_CONNECTED_SERVER,
    APP_BLE_CONNECTED_CLIENT
} APP_BLE_ConnStatus_t;

bool APP_BLE_Init();
bool APP_BLE_Start();

APP_BLE_ConnStatus_t APP_BLE_Get_Server_Connection_Status();

#ifdef __cplusplus
}
#endif
