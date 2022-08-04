#pragma once

#include "nfc_worker.h"

#include <furi.h>
#include <lib/toolbox/stream/file_stream.h>

#include <lib/nfc/protocols/nfc_util.h>
#include <lib/nfc/protocols/emv.h>
#include <lib/nfc/protocols/mifare_common.h>
#include <lib/nfc/protocols/mifare_ultralight.h>
#include <lib/nfc/protocols/mifare_classic.h>
#include <lib/nfc/protocols/mifare_desfire.h>
#include <lib/nfc/protocols/nfca.h>

#include "helpers/nfc_debug_pcap.h"

struct NfcWorker {
    FuriThread* thread;
    Storage* storage;
    Stream* dict_stream;

    NfcDeviceData* dev_data;

    NfcWorkerCallback callback;
    void* context;

    NfcWorkerState state;

    NfcDebugPcapWorker* debug_pcap_worker;
};

void nfc_worker_change_state(NfcWorker* nfc_worker, NfcWorkerState state);

int32_t nfc_worker_task(void* context);

void nfc_worker_read(NfcWorker* nfc_worker);

void nfc_worker_emulate_uid(NfcWorker* nfc_worker);

void nfc_worker_emulate_mf_ultralight(NfcWorker* nfc_worker);

void nfc_worker_emulate_mf_classic(NfcWorker* nfc_worker);

void nfc_worker_mf_classic_dict_attack(NfcWorker* nfc_worker);

void nfc_worker_emulate_apdu(NfcWorker* nfc_worker);
