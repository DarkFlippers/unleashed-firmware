#include "spi_mem_worker_i.h"
#include "spi_mem_chip.h"
#include "spi_mem_tools.h"
#include "../../spi_mem_files.h"

static void spi_mem_worker_chip_detect_process(SPIMemWorker* worker);
static void spi_mem_worker_read_process(SPIMemWorker* worker);
static void spi_mem_worker_verify_process(SPIMemWorker* worker);
static void spi_mem_worker_erase_process(SPIMemWorker* worker);
static void spi_mem_worker_write_process(SPIMemWorker* worker);

const SPIMemWorkerModeType spi_mem_worker_modes[] = {
    [SPIMemWorkerModeIdle] = {.process = NULL},
    [SPIMemWorkerModeChipDetect] = {.process = spi_mem_worker_chip_detect_process},
    [SPIMemWorkerModeRead] = {.process = spi_mem_worker_read_process},
    [SPIMemWorkerModeVerify] = {.process = spi_mem_worker_verify_process},
    [SPIMemWorkerModeErase] = {.process = spi_mem_worker_erase_process},
    [SPIMemWorkerModeWrite] = {.process = spi_mem_worker_write_process}};

static void spi_mem_worker_run_callback(SPIMemWorker* worker, SPIMemCustomEventWorker event) {
    if(worker->callback) {
        worker->callback(worker->cb_ctx, event);
    }
}

static bool spi_mem_worker_await_chip_busy(SPIMemWorker* worker) {
    while(true) {
        furi_delay_tick(10); // to give some time to OS
        if(spi_mem_worker_check_for_stop(worker)) return true;
        SPIMemChipStatus chip_status = spi_mem_tools_get_chip_status(worker->chip_info);
        if(chip_status == SPIMemChipStatusError) return false;
        if(chip_status == SPIMemChipStatusBusy) continue;
        return true;
    }
}

static size_t spi_mem_worker_modes_get_total_size(SPIMemWorker* worker) {
    size_t chip_size = spi_mem_chip_get_size(worker->chip_info);
    size_t file_size = spi_mem_file_get_size(worker->cb_ctx);
    size_t total_size = chip_size;
    if(chip_size > file_size) total_size = file_size;
    return total_size;
}

// ChipDetect
static void spi_mem_worker_chip_detect_process(SPIMemWorker* worker) {
    SPIMemCustomEventWorker event;
    while(!spi_mem_tools_read_chip_info(worker->chip_info)) {
        furi_delay_tick(10); // to give some time to OS
        if(spi_mem_worker_check_for_stop(worker)) return;
    }
    if(spi_mem_chip_find_all(worker->chip_info, *worker->found_chips)) {
        event = SPIMemCustomEventWorkerChipIdentified;
    } else {
        event = SPIMemCustomEventWorkerChipUnknown;
    }
    spi_mem_worker_run_callback(worker, event);
}

// Read
static bool spi_mem_worker_read(SPIMemWorker* worker, SPIMemCustomEventWorker* event) {
    uint8_t data_buffer[SPI_MEM_FILE_BUFFER_SIZE];
    size_t chip_size = spi_mem_chip_get_size(worker->chip_info);
    size_t offset = 0;
    bool success = true;
    while(true) {
        furi_delay_tick(10); // to give some time to OS
        size_t block_size = SPI_MEM_FILE_BUFFER_SIZE;
        if(spi_mem_worker_check_for_stop(worker)) break;
        if(offset >= chip_size) break;
        if((offset + block_size) > chip_size) block_size = chip_size - offset;
        if(!spi_mem_tools_read_block(worker->chip_info, offset, data_buffer, block_size)) {
            *event = SPIMemCustomEventWorkerChipFail;
            success = false;
            break;
        }
        if(!spi_mem_file_write_block(worker->cb_ctx, data_buffer, block_size)) {
            success = false;
            break;
        }
        offset += block_size;
        spi_mem_worker_run_callback(worker, SPIMemCustomEventWorkerBlockReaded);
    }
    if(success) *event = SPIMemCustomEventWorkerDone;
    return success;
}

static void spi_mem_worker_read_process(SPIMemWorker* worker) {
    SPIMemCustomEventWorker event = SPIMemCustomEventWorkerFileFail;
    do {
        if(!spi_mem_worker_await_chip_busy(worker)) break;
        if(!spi_mem_file_create_open(worker->cb_ctx)) break;
        if(!spi_mem_worker_read(worker, &event)) break;
    } while(0);
    spi_mem_file_close(worker->cb_ctx);
    spi_mem_worker_run_callback(worker, event);
}

// Verify
static bool
    spi_mem_worker_verify(SPIMemWorker* worker, size_t total_size, SPIMemCustomEventWorker* event) {
    uint8_t data_buffer_chip[SPI_MEM_FILE_BUFFER_SIZE];
    uint8_t data_buffer_file[SPI_MEM_FILE_BUFFER_SIZE];
    size_t offset = 0;
    bool success = true;
    while(true) {
        furi_delay_tick(10); // to give some time to OS
        size_t block_size = SPI_MEM_FILE_BUFFER_SIZE;
        if(spi_mem_worker_check_for_stop(worker)) break;
        if(offset >= total_size) break;
        if((offset + block_size) > total_size) block_size = total_size - offset;
        if(!spi_mem_tools_read_block(worker->chip_info, offset, data_buffer_chip, block_size)) {
            *event = SPIMemCustomEventWorkerChipFail;
            success = false;
            break;
        }
        if(!spi_mem_file_read_block(worker->cb_ctx, data_buffer_file, block_size)) {
            success = false;
            break;
        }
        if(memcmp(data_buffer_chip, data_buffer_file, block_size) != 0) {
            *event = SPIMemCustomEventWorkerVerifyFail;
            success = false;
            break;
        }
        offset += block_size;
        spi_mem_worker_run_callback(worker, SPIMemCustomEventWorkerBlockReaded);
    }
    if(success) *event = SPIMemCustomEventWorkerDone;
    return success;
}

static void spi_mem_worker_verify_process(SPIMemWorker* worker) {
    SPIMemCustomEventWorker event = SPIMemCustomEventWorkerFileFail;
    size_t total_size = spi_mem_worker_modes_get_total_size(worker);
    do {
        if(!spi_mem_worker_await_chip_busy(worker)) break;
        if(!spi_mem_file_open(worker->cb_ctx)) break;
        if(!spi_mem_worker_verify(worker, total_size, &event)) break;
    } while(0);
    spi_mem_file_close(worker->cb_ctx);
    spi_mem_worker_run_callback(worker, event);
}

// Erase
static void spi_mem_worker_erase_process(SPIMemWorker* worker) {
    SPIMemCustomEventWorker event = SPIMemCustomEventWorkerChipFail;
    do {
        if(!spi_mem_worker_await_chip_busy(worker)) break;
        if(!spi_mem_tools_erase_chip(worker->chip_info)) break;
        if(!spi_mem_worker_await_chip_busy(worker)) break;
        event = SPIMemCustomEventWorkerDone;
    } while(0);
    spi_mem_worker_run_callback(worker, event);
}

// Write
static bool spi_mem_worker_write_block_by_page(
    SPIMemWorker* worker,
    size_t offset,
    uint8_t* data,
    size_t block_size,
    size_t page_size) {
    for(size_t i = 0; i < block_size; i += page_size) {
        if(!spi_mem_worker_await_chip_busy(worker)) return false;
        if(!spi_mem_tools_write_bytes(worker->chip_info, offset, data, page_size)) return false;
        offset += page_size;
        data += page_size;
    }
    return true;
}

static bool
    spi_mem_worker_write(SPIMemWorker* worker, size_t total_size, SPIMemCustomEventWorker* event) {
    bool success = true;
    uint8_t data_buffer[SPI_MEM_FILE_BUFFER_SIZE];
    size_t page_size = spi_mem_chip_get_page_size(worker->chip_info);
    size_t offset = 0;
    while(true) {
        furi_delay_tick(10); // to give some time to OS
        size_t block_size = SPI_MEM_FILE_BUFFER_SIZE;
        if(spi_mem_worker_check_for_stop(worker)) break;
        if(offset >= total_size) break;
        if((offset + block_size) > total_size) block_size = total_size - offset;
        if(!spi_mem_file_read_block(worker->cb_ctx, data_buffer, block_size)) {
            *event = SPIMemCustomEventWorkerFileFail;
            success = false;
            break;
        }
        if(!spi_mem_worker_write_block_by_page(
               worker, offset, data_buffer, block_size, page_size)) {
            success = false;
            break;
        }
        offset += block_size;
        spi_mem_worker_run_callback(worker, SPIMemCustomEventWorkerBlockReaded);
    }
    return success;
}

static void spi_mem_worker_write_process(SPIMemWorker* worker) {
    SPIMemCustomEventWorker event = SPIMemCustomEventWorkerChipFail;
    size_t total_size =
        spi_mem_worker_modes_get_total_size(worker); // need to be executed before opening file
    do {
        if(!spi_mem_file_open(worker->cb_ctx)) break;
        if(!spi_mem_worker_await_chip_busy(worker)) break;
        if(!spi_mem_worker_write(worker, total_size, &event)) break;
        if(!spi_mem_worker_await_chip_busy(worker)) break;
        event = SPIMemCustomEventWorkerDone;
    } while(0);
    spi_mem_file_close(worker->cb_ctx);
    spi_mem_worker_run_callback(worker, event);
}
