#pragma once

#include "nfc_types.h"
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
    osMessageQueueId_t message_queue;
    osThreadAttr_t thread_attr;
    osThreadId_t thread;

    NfcWorkerState state;
    ReturnCode error;
};

void nfc_worker_change_state(NfcWorker* nfc_worker, NfcWorkerState state);

void nfc_worker_task(void* context);

void nfc_worker_read_emv(NfcWorker* nfc_worker);

void nfc_worker_emulate_emv(NfcWorker* nfc_worker);

void nfc_worker_poll(NfcWorker* nfc_worker);

void nfc_worker_emulate(NfcWorker* nfc_worker);

void nfc_worker_field(NfcWorker* nfc_worker);

void nfc_worker_read_mf_ultralight(NfcWorker* nfc_worker);
