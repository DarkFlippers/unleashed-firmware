#pragma once

#include <stdlib.h>
#include <furi/core/thread.h>
#include <furi/core/mutex.h>
#include <furi/core/string.h>
#include <furi/core/kernel.h>
#include <bt/bt_service/bt.h>
#include "../../features_config.h"

#if TOTP_TARGET_FIRMWARE == TOTP_FIRMWARE_XTREME
#define TOTP_BT_WORKER_BT_ADV_NAME_MAX_LEN FURI_HAL_BT_ADV_NAME_LENGTH
#define TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN GAP_MAC_ADDR_SIZE
#endif

typedef uint8_t TotpBtTypeCodeWorkerEvent;

typedef struct {
    char* code_buffer;
    uint8_t code_buffer_size;
    uint8_t flags;
    FuriThread* thread;
    FuriMutex* code_buffer_sync;
    Bt* bt;
    bool is_advertising;
    bool is_connected;
#if TOTP_TARGET_FIRMWARE == TOTP_FIRMWARE_XTREME
    char previous_bt_name[TOTP_BT_WORKER_BT_ADV_NAME_MAX_LEN];
    uint8_t previous_bt_mac[TOTP_BT_WORKER_BT_MAC_ADDRESS_LEN];
#endif
} TotpBtTypeCodeWorkerContext;

enum TotpBtTypeCodeWorkerEvents {
    TotpBtTypeCodeWorkerEventReserved = 0b00,
    TotpBtTypeCodeWorkerEventStop = 0b01,
    TotpBtTypeCodeWorkerEventType = 0b10
};

TotpBtTypeCodeWorkerContext* totp_bt_type_code_worker_init();
void totp_bt_type_code_worker_free(TotpBtTypeCodeWorkerContext* context);
void totp_bt_type_code_worker_start(
    TotpBtTypeCodeWorkerContext* context,
    char* code_buffer,
    uint8_t code_buffer_size,
    FuriMutex* code_buffer_sync);
void totp_bt_type_code_worker_stop(TotpBtTypeCodeWorkerContext* context);
void totp_bt_type_code_worker_notify(
    TotpBtTypeCodeWorkerContext* context,
    TotpBtTypeCodeWorkerEvent event,
    uint8_t flags);
