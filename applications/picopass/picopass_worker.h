#pragma once

#include "picopass_device.h"

typedef struct PicopassWorker PicopassWorker;

typedef enum {
    // Init states
    PicopassWorkerStateNone,
    PicopassWorkerStateBroken,
    PicopassWorkerStateReady,
    // Main worker states
    PicopassWorkerStateDetect,
    PicopassWorkerStateWrite,
    // Transition
    PicopassWorkerStateStop,
} PicopassWorkerState;

typedef enum {
    // Reserve first 50 events for application events
    PicopassWorkerEventReserved = 50,

    // Picopass worker common events
    PicopassWorkerEventSuccess,
    PicopassWorkerEventFail,
    PicopassWorkerEventNoCardDetected,

    PicopassWorkerEventStartReading,
} PicopassWorkerEvent;

typedef void (*PicopassWorkerCallback)(PicopassWorkerEvent event, void* context);

PicopassWorker* picopass_worker_alloc();

PicopassWorkerState picopass_worker_get_state(PicopassWorker* picopass_worker);

void picopass_worker_free(PicopassWorker* picopass_worker);

void picopass_worker_start(
    PicopassWorker* picopass_worker,
    PicopassWorkerState state,
    PicopassDeviceData* dev_data,
    PicopassWorkerCallback callback,
    void* context);

void picopass_worker_stop(PicopassWorker* picopass_worker);
