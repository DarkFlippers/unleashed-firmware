#pragma once

#include <stdlib.h>
#include <furi/furi.h>
#include <furi_hal.h>

typedef uint8_t TotpUsbTypeCodeWorkerEvent;

typedef struct {
    char* string;
    uint8_t string_length;
    FuriThread* thread;
    FuriMutex* string_sync;
    FuriHalUsbInterface* usb_mode_prev;
} TotpUsbTypeCodeWorkerContext;

enum TotpUsbTypeCodeWorkerEvents {
    TotpUsbTypeCodeWorkerEventReserved = (1 << 0),
    TotpUsbTypeCodeWorkerEventStop = (1 << 1),
    TotpUsbTypeCodeWorkerEventType = (1 << 2)
};

TotpUsbTypeCodeWorkerContext* totp_usb_type_code_worker_start(
    char* code_buf,
    uint8_t code_buf_length,
    FuriMutex* code_buf_update_sync);
void totp_usb_type_code_worker_stop(TotpUsbTypeCodeWorkerContext* context);
void totp_usb_type_code_worker_notify(
    TotpUsbTypeCodeWorkerContext* context,
    TotpUsbTypeCodeWorkerEvent event);