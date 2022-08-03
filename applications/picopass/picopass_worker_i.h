#pragma once

#include "picopass_worker.h"
#include "picopass_i.h"

#include <furi.h>
#include <lib/toolbox/stream/file_stream.h>

struct PicopassWorker {
    FuriThread* thread;
    Storage* storage;

    PicopassDeviceData* dev_data;
    PicopassWorkerCallback callback;
    void* context;

    PicopassWorkerState state;
};

void picopass_worker_change_state(PicopassWorker* picopass_worker, PicopassWorkerState state);

int32_t picopass_worker_task(void* context);

void picopass_worker_detect(PicopassWorker* picopass_worker);
