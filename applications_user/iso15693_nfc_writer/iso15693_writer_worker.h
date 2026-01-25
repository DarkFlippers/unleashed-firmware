#pragma once
#include <furi.h>
#include <nfc/nfc.h>
#include <nfc/protocols/iso15693_3/iso15693_3_poller.h>

typedef struct Iso15693WriterWorker Iso15693WriterWorker;

typedef enum {
    Iso15693WriterWorkerModeWrite,
    Iso15693WriterWorkerModeFormat,
    Iso15693WriterWorkerModeWriteFull,
    Iso15693WriterWorkerModeRead,
    Iso15693WriterWorkerModeReadAll,
    Iso15693WriterWorkerModeLock,
    Iso15693WriterWorkerModeLockAll,
    Iso15693WriterWorkerModeWriteAFI,
    Iso15693WriterWorkerModeLockAFI,
    Iso15693WriterWorkerModeWriteDSFID,
    Iso15693WriterWorkerModeLockDSFID,
} Iso15693WriterWorkerMode;

typedef enum {
    Iso15693WriterWorkerEventCardDetected,
    Iso15693WriterWorkerEventWriteSuccess,
    Iso15693WriterWorkerEventWriteFail,
    Iso15693WriterWorkerEventReadSuccess,
    Iso15693WriterWorkerEventProgress,
    Iso15693WriterWorkerEventAborted,
} Iso15693WriterWorkerEvent;

typedef void (*Iso15693WriterWorkerCallback)(Iso15693WriterWorkerEvent event, void* context);

Iso15693WriterWorker* iso15693_writer_worker_alloc();

void iso15693_writer_worker_free(Iso15693WriterWorker* instance);

void iso15693_writer_worker_start(
    Iso15693WriterWorker* instance,
    Iso15693WriterWorkerMode mode,
    uint8_t block_number,
    uint8_t* full_data_buffer,
    uint8_t* lock_status_buffer,
    const uint8_t* single_data,
    uint8_t afi,
    uint8_t dsfid,
    Iso15693WriterWorkerCallback callback,
    void* context);

void iso15693_writer_worker_stop(Iso15693WriterWorker* instance);

uint8_t iso15693_writer_worker_get_progress(Iso15693WriterWorker* instance);

uint16_t iso15693_writer_worker_get_block_count(Iso15693WriterWorker* instance);

uint8_t iso15693_writer_worker_get_block_size(Iso15693WriterWorker* instance);
