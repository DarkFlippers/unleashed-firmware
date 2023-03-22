#pragma once

#include <stdlib.h>
#include <furi/furi.h>
#include <furi_hal.h>
#include <bt/bt_service/bt.h>

typedef uint8_t TotpBtTypeCodeWorkerEvent;

typedef struct {
    char* string;
    uint8_t string_length;
    FuriThread* thread;
    FuriMutex* string_sync;
    Bt* bt;
    bool is_advertising;
} TotpBtTypeCodeWorkerContext;

enum TotpBtTypeCodeWorkerEvents {
    TotpBtTypeCodeWorkerEventReserved = (1 << 0),
    TotpBtTypeCodeWorkerEventStop = (1 << 1),
    TotpBtTypeCodeWorkerEventType = (1 << 2)
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
    TotpBtTypeCodeWorkerEvent event);