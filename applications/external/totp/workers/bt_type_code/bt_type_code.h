#pragma once

#include <stdlib.h>
#include <furi/furi.h>
#include <furi_hal.h>
#include <bt/bt_service/bt.h>
#include "../../features_config.h"

#if TOTP_TARGET_FIRMWARE == TOTP_FIRMWARE_XTREME
#define TOTP_BT_WORKER_BT_ADV_NAME_MAX_LEN 18
#define TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN GAP_MAC_ADDR_SIZE
#endif

typedef uint8_t TotpBtTypeCodeWorkerEvent;

typedef struct {
    char* string;
    uint8_t string_length;
    uint8_t flags;
    FuriThread* thread;
    FuriMutex* string_sync;
    Bt* bt;
    bool is_advertising;
    bool is_connected;
#if TOTP_TARGET_FIRMWARE == TOTP_FIRMWARE_XTREME
    uint8_t bt_mac[TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN];
    char previous_bt_name[TOTP_BT_WORKER_BT_ADV_NAME_MAX_LEN + 1];
    uint8_t previous_bt_mac[TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN];
#endif
} TotpBtTypeCodeWorkerContext;

enum TotpBtTypeCodeWorkerEvents {
    TotpBtTypeCodeWorkerEventReserved = 0b0000,
    TotpBtTypeCodeWorkerEventStop = 0b0100,
    TotpBtTypeCodeWorkerEventType = 0b1000
};

TotpBtTypeCodeWorkerContext* totp_bt_type_code_worker_init();
void totp_bt_type_code_worker_free(TotpBtTypeCodeWorkerContext* context);
void totp_bt_type_code_worker_start(
    TotpBtTypeCodeWorkerContext* context,
    char* code_buf,
    uint8_t code_buf_length,
    FuriMutex* code_buf_update_sync);
void totp_bt_type_code_worker_stop(TotpBtTypeCodeWorkerContext* context);
void totp_bt_type_code_worker_notify(
    TotpBtTypeCodeWorkerContext* context,
    TotpBtTypeCodeWorkerEvent event,
    uint8_t flags);