#pragma once

#include "nfc_device.h"

typedef struct NfcWorker NfcWorker;

typedef enum {
    // Init states
    NfcWorkerStateNone,
    NfcWorkerStateBroken,
    NfcWorkerStateReady,
    // Main worker states
    NfcWorkerStateDetect,
    NfcWorkerStateEmulate,
    NfcWorkerStateReadEMVApp,
    NfcWorkerStateReadEMV,
    NfcWorkerStateEmulateApdu,
    NfcWorkerStateField,
    NfcWorkerStateReadMifareUl,
    NfcWorkerStateEmulateMifareUl,
    NfcWorkerStateReadMifareClassic,
    NfcWorkerStateReadMifareDesfire,
    // Transition
    NfcWorkerStateStop,
} NfcWorkerState;

typedef enum {
    // Reserve first 50 events for application events
    NfcWorkerEventReserved = 50,

    // Nfc worker common events
    NfcWorkerEventSuccess,
    NfcWorkerEventFail,
    NfcWorkerEventNoCardDetected,
    // Mifare Classic events
    NfcWorkerEventNoDictFound,
    NfcWorkerEventDetectedClassic1k,
    NfcWorkerEventDetectedClassic4k,
    NfcWorkerEventNewSector,
    NfcWorkerEventFoundKeyA,
    NfcWorkerEventFoundKeyB,
    NfcWorkerEventStartReading,
} NfcWorkerEvent;

typedef void (*NfcWorkerCallback)(NfcWorkerEvent event, void* context);

NfcWorker* nfc_worker_alloc();

NfcWorkerState nfc_worker_get_state(NfcWorker* nfc_worker);

void nfc_worker_free(NfcWorker* nfc_worker);

void nfc_worker_start(
    NfcWorker* nfc_worker,
    NfcWorkerState state,
    NfcDeviceData* dev_data,
    NfcWorkerCallback callback,
    void* context);

void nfc_worker_stop(NfcWorker* nfc_worker);
