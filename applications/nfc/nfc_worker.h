#pragma once

typedef struct NfcWorker NfcWorker;

typedef enum {
    // Init states
    NfcWorkerStateNone,
    NfcWorkerStateBroken,
    NfcWorkerStateReady,
    // Main worker states
    NfcWorkerStateDetect,
    NfcWorkerStateEmulate,
    NfcWorkerStateReadEMV,
    NfcWorkerStateEmulateEMV,
    NfcWorkerStateField,
    NfcWorkerStateReadMfUltralight,
    // Transition
    NfcWorkerStateStop,
} NfcWorkerState;

typedef void (*NfcWorkerCallback)(void* context);

NfcWorker* nfc_worker_alloc(osMessageQueueId_t message_queue);

NfcWorkerState nfc_worker_get_state(NfcWorker* nfc_worker);

ReturnCode nfc_worker_get_error(NfcWorker* nfc_worker);

void nfc_worker_free(NfcWorker* nfc_worker);

void nfc_worker_start(
    NfcWorker* nfc_worker,
    NfcWorkerState state,
    NfcWorkerCallback callback,
    void* context);

void nfc_worker_stop(NfcWorker* nfc_worker);
