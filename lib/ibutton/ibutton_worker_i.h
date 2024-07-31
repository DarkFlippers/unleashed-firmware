/**
 * @file ibutton_worker_i.h
 * 
 * iButton worker, internal definitions 
 */

#pragma once

#include <core/thread.h>
#include <core/message_queue.h>

#include "ibutton_worker.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const uint32_t quant;
    void (*const start)(iButtonWorker* worker);
    void (*const tick)(iButtonWorker* worker);
    void (*const stop)(iButtonWorker* worker);
} iButtonWorkerModeType;

typedef enum {
    iButtonWorkerModeIdle,
    iButtonWorkerModeRead,
    iButtonWorkerModeWriteId,
    iButtonWorkerModeWriteCopy,
    iButtonWorkerModeEmulate,
} iButtonWorkerMode;

struct iButtonWorker {
    iButtonKey* key;
    iButtonProtocols* protocols;
    iButtonWorkerMode mode_index;
    FuriMessageQueue* messages;
    FuriThread* thread;

    iButtonWorkerReadCallback read_cb;
    iButtonWorkerWriteCallback write_cb;
    iButtonWorkerEmulateCallback emulate_cb;

    void* cb_ctx;
};

extern const iButtonWorkerModeType ibutton_worker_modes[];

void ibutton_worker_switch_mode(iButtonWorker* worker, iButtonWorkerMode mode);
void ibutton_worker_notify_emulate(iButtonWorker* worker);

#ifdef __cplusplus
}
#endif
