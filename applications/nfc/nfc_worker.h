#pragma once

typedef enum {
    NfcDeviceNfca,
    NfcDeviceNfcb,
    NfcDeviceNfcf,
    NfcDeviceNfcv,
} NfcDeviceType;

typedef enum {
    NfcDeviceProtocolUnknown,
    NfcDeviceProtocolEMV,
    NfcDeviceProtocolMfUltralight,
} NfcProtocol;

typedef struct {
    uint8_t uid_len;
    uint8_t uid[10];
    uint8_t atqa[2];
    uint8_t sak;
    NfcDeviceType device;
    NfcProtocol protocol;
} NfcDeviceData;

typedef struct {
    NfcDeviceData nfc_data;
    char name[32];
    uint8_t number[8];
} NfcEmvData;

typedef struct {
    NfcDeviceData nfc_data;
    uint8_t man_block[12];
    uint8_t otp[4];
} NfcMifareUlData;

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
