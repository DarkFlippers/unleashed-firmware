#pragma once

#include <furi.h>
#include "mifare_nested_i.h"
#include "mifare_nested_worker.h"

struct MifareNestedWorker {
    FuriThread* thread;

    NfcDeviceData* dev_data;

    MifareNestedWorkerCallback callback;
    MifareNested* context;

    MifareNestedWorkerState state;
};

int32_t mifare_nested_worker_task(void* context);

void mifare_nested_worker_check(MifareNestedWorker* mifare_nested_worker);

void mifare_nested_worker_collect_nonces(MifareNestedWorker* mifare_nested_worker);

void mifare_nested_worker_collect_nonces_static(MifareNestedWorker* mifare_nested_worker);

void mifare_nested_worker_collect_nonces_hard(MifareNestedWorker* mifare_nested_worker);

void mifare_nested_worker_check_keys(MifareNestedWorker* mifare_nested_worker);
