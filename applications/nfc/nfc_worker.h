#pragma once

#include "nfc_device.h"

typedef struct {
    union {
        NfcDeviceData nfc_detect_data;
        NfcEmvData nfc_emv_data;
        NfcMifareUlData nfc_mifare_ul_data;
    };
} NfcWorkerResult;

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

NfcWorker* nfc_worker_alloc();

NfcWorkerState nfc_worker_get_state(NfcWorker* nfc_worker);

ReturnCode nfc_worker_get_error(NfcWorker* nfc_worker);

void nfc_worker_set_emulation_params(NfcWorker* nfc_worker, NfcDeviceData* data);

void nfc_worker_free(NfcWorker* nfc_worker);

void nfc_worker_start(
    NfcWorker* nfc_worker,
    NfcWorkerState state,
    NfcWorkerResult* result_dest,
    NfcWorkerCallback callback,
    void* context);

void nfc_worker_stop(NfcWorker* nfc_worker);
