#pragma once

#include <stdlib.h>
#include <furi/core/thread.h>
#include <furi/core/mutex.h>
#include <furi/core/kernel.h>
#include <furi/core/check.h>
#include <furi_hal_usb.h>

typedef uint8_t TotpUsbTypeCodeWorkerEvent;

typedef struct {
    char* code_buffer;
    uint8_t code_buffer_size;
    uint8_t flags;
    FuriThread* thread;
    FuriMutex* code_buffer_sync;
    FuriHalUsbInterface* usb_mode_prev;
} TotpUsbTypeCodeWorkerContext;

enum TotpUsbTypeCodeWorkerEvents {
    TotpUsbTypeCodeWorkerEventReserved = 0b00,
    TotpUsbTypeCodeWorkerEventStop = 0b01,
    TotpUsbTypeCodeWorkerEventType = 0b10
};

TotpUsbTypeCodeWorkerContext* totp_usb_type_code_worker_start(
    char* code_buffer,
    uint8_t code_buffer_size,
    FuriMutex* code_buffer_sync);
void totp_usb_type_code_worker_stop(TotpUsbTypeCodeWorkerContext* context);
void totp_usb_type_code_worker_notify(
    TotpUsbTypeCodeWorkerContext* context,
    TotpUsbTypeCodeWorkerEvent event,
    uint8_t flags);