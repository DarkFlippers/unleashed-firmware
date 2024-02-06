#pragma once

#include <furi_hal_serial_types.h>

typedef struct ExpansionWorker ExpansionWorker;

typedef void (*ExpansionWorkerCallback)(void*);

ExpansionWorker* expansion_worker_alloc(FuriHalSerialId serial_id);

void expansion_worker_free(ExpansionWorker* instance);

void expansion_worker_set_callback(
    ExpansionWorker* instance,
    ExpansionWorkerCallback callback,
    void* context);

void expansion_worker_start(ExpansionWorker* instance);

void expansion_worker_stop(ExpansionWorker* instance);
