#pragma once

#include <furi.h>
#include "spi_mem_chip.h"

typedef struct SPIMemWorker SPIMemWorker;

typedef struct {
    void (*const process)(SPIMemWorker* worker);
} SPIMemWorkerModeType;

typedef enum {
    SPIMemCustomEventWorkerChipIdentified,
    SPIMemCustomEventWorkerChipUnknown,
    SPIMemCustomEventWorkerBlockReaded,
    SPIMemCustomEventWorkerChipFail,
    SPIMemCustomEventWorkerFileFail,
    SPIMemCustomEventWorkerDone,
    SPIMemCustomEventWorkerVerifyFail,
} SPIMemCustomEventWorker;

typedef void (*SPIMemWorkerCallback)(void* context, SPIMemCustomEventWorker event);

SPIMemWorker* spi_mem_worker_alloc();
void spi_mem_worker_free(SPIMemWorker* worker);
void spi_mem_worker_start_thread(SPIMemWorker* worker);
void spi_mem_worker_stop_thread(SPIMemWorker* worker);
bool spi_mem_worker_check_for_stop(SPIMemWorker* worker);
void spi_mem_worker_chip_detect_start(
    SPIMemChip* chip_info,
    found_chips_t* found_chips,
    SPIMemWorker* worker,
    SPIMemWorkerCallback callback,
    void* context);
void spi_mem_worker_read_start(
    SPIMemChip* chip_info,
    SPIMemWorker* worker,
    SPIMemWorkerCallback callback,
    void* context);
void spi_mem_worker_verify_start(
    SPIMemChip* chip_info,
    SPIMemWorker* worker,
    SPIMemWorkerCallback callback,
    void* context);
void spi_mem_worker_erase_start(
    SPIMemChip* chip_info,
    SPIMemWorker* worker,
    SPIMemWorkerCallback callback,
    void* context);
void spi_mem_worker_write_start(
    SPIMemChip* chip_info,
    SPIMemWorker* worker,
    SPIMemWorkerCallback callback,
    void* context);
