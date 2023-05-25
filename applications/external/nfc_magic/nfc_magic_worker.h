#pragma once

#include <lib/nfc/nfc_device.h>
#include "nfc_magic.h"

typedef struct NfcMagicWorker NfcMagicWorker;

typedef enum {
    NfcMagicWorkerStateReady,

    NfcMagicWorkerStateCheck,
    NfcMagicWorkerStateWrite,
    NfcMagicWorkerStateRekey,
    NfcMagicWorkerStateWipe,

    NfcMagicWorkerStateStop,
} NfcMagicWorkerState;

typedef enum {
    NfcMagicWorkerEventSuccess,
    NfcMagicWorkerEventFail,
    NfcMagicWorkerEventCardDetected,
    NfcMagicWorkerEventNoCardDetected,
    NfcMagicWorkerEventWrongCard,
} NfcMagicWorkerEvent;

typedef bool (*NfcMagicWorkerCallback)(NfcMagicWorkerEvent event, void* context);

NfcMagicWorker* nfc_magic_worker_alloc();

void nfc_magic_worker_free(NfcMagicWorker* nfc_magic_worker);

void nfc_magic_worker_stop(NfcMagicWorker* nfc_magic_worker);

void nfc_magic_worker_start(
    NfcMagicWorker* nfc_magic_worker,
    NfcMagicWorkerState state,
    NfcMagicDevice* magic_dev,
    NfcDeviceData* dev_data,
    uint32_t new_password,
    NfcMagicWorkerCallback callback,
    void* context);
