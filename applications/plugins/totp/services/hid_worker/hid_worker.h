#pragma once

#include <stdlib.h>
#include <furi/furi.h>
#include <furi_hal.h>

typedef struct {
    char* string;
    uint8_t string_length;
    FuriThread* thread;
    FuriMutex* string_sync;
    FuriHalUsbInterface* usb_mode_prev;
} TotpHidWorkerTypeContext;

typedef enum {
    TotpHidWorkerEvtReserved = (1 << 0),
    TotpHidWorkerEvtStop = (1 << 1),
    TotpHidWorkerEvtType = (1 << 2)
} TotpHidWorkerEvtFlags;

TotpHidWorkerTypeContext* totp_hid_worker_start();
void totp_hid_worker_stop(TotpHidWorkerTypeContext* context);
void totp_hid_worker_notify(TotpHidWorkerTypeContext* context, TotpHidWorkerEvtFlags event);