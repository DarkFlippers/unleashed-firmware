#pragma once

#include "nfc_i.h"
#include "nfc_worker.h"

#include <furi.h>
#include <lib/toolbox/stream/file_stream.h>

struct NfcWorker {
    FuriThread* thread;
    Storage* storage;
    Stream* dict_stream;

    NfcDeviceData* dev_data;

    NfcWorkerCallback callback;
    void* context;

    NfcWorkerState state;
};

void nfc_worker_change_state(NfcWorker* nfc_worker, NfcWorkerState state);

int32_t nfc_worker_task(void* context);

void nfc_worker_read_emv_app(NfcWorker* nfc_worker);

void nfc_worker_read_emv(NfcWorker* nfc_worker);

void nfc_worker_emulate_apdu(NfcWorker* nfc_worker);

void nfc_worker_detect(NfcWorker* nfc_worker);

void nfc_worker_emulate(NfcWorker* nfc_worker);

void nfc_worker_read_mifare_ultralight(NfcWorker* nfc_worker);

void nfc_worker_mifare_classic_dict_attack(NfcWorker* nfc_worker);

void nfc_worker_read_mifare_desfire(NfcWorker* nfc_worker);

void nfc_worker_emulate_mifare_ul(NfcWorker* nfc_worker);

void nfc_worker_emulate_mifare_classic(NfcWorker* nfc_worker);
