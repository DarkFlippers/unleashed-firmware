#pragma once

#include <stdlib.h>
#include <furi/furi.h>
#include <furi_hal.h>

typedef uint8_t TotpTypeCodeWorkerEvent;

typedef struct {
    char* string;
    uint8_t string_length;
    FuriThread* thread;
    FuriMutex* string_sync;
    FuriHalUsbInterface* usb_mode_prev;
} TotpTypeCodeWorkerContext;

enum TotpTypeCodeWorkerEvents {
    TotpTypeCodeWorkerEventReserved = (1 << 0),
    TotpTypeCodeWorkerEventStop = (1 << 1),
    TotpTypeCodeWorkerEventType = (1 << 2)
};

TotpTypeCodeWorkerContext* totp_type_code_worker_start();
void totp_type_code_worker_stop(TotpTypeCodeWorkerContext* context);
void totp_type_code_worker_notify(
    TotpTypeCodeWorkerContext* context,
    TotpTypeCodeWorkerEvent event);