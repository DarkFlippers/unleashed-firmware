#pragma once

#include "nfc_i.h"
#include "nfc_worker.h"

#include <furi.h>
#include <stdbool.h>

#include <rfal_analogConfig.h>
#include <rfal_rf.h>
#include <rfal_nfc.h>
#include <rfal_nfca.h>
#include <rfal_nfcb.h>
#include <rfal_nfcf.h>
#include <rfal_nfcv.h>
#include <st25r3916.h>
#include <st25r3916_irq.h>

struct NfcWorker {
    osThreadAttr_t thread_attr;
    osThreadId_t thread;

    NfcDeviceData* dev_data;

    NfcWorkerCallback callback;
    void* context;

    NfcWorkerState state;
    ReturnCode error;
};

void nfc_worker_change_state(NfcWorker* nfc_worker, NfcWorkerState state);

void nfc_worker_task(void* context);

void nfc_worker_read_emv_app(NfcWorker* nfc_worker);

void nfc_worker_read_emv(NfcWorker* nfc_worker);

void nfc_worker_emulate_emv(NfcWorker* nfc_worker);

void nfc_worker_detect(NfcWorker* nfc_worker);

void nfc_worker_emulate(NfcWorker* nfc_worker);

void nfc_worker_field(NfcWorker* nfc_worker);

void nfc_worker_read_mifare_ul(NfcWorker* nfc_worker);

void nfc_worker_emulate_mifare_ul(NfcWorker* nfc_worker);
