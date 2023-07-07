#pragma once

#include <furi.h>

#include "nfc_magic_worker.h"
#include "lib/magic/common.h"

struct NfcMagicWorker {
    FuriThread* thread;

    NfcMagicDevice* magic_dev;
    NfcDeviceData* dev_data;
    uint32_t new_password;

    NfcMagicWorkerCallback callback;
    void* context;

    NfcMagicWorkerState state;
};

int32_t nfc_magic_worker_task(void* context);

void nfc_magic_worker_check(NfcMagicWorker* nfc_magic_worker);

void nfc_magic_worker_write(NfcMagicWorker* nfc_magic_worker);

void nfc_magic_worker_rekey(NfcMagicWorker* nfc_magic_worker);

void nfc_magic_worker_wipe(NfcMagicWorker* nfc_magic_worker);
