#pragma once

#include <furi_hal_subghz.h>

typedef struct SubBruteWorker SubBruteWorker;
/**
 * Same like SubGhzTxRxWorkerStatus in subghz_tx_rx_worker.h
 * using just to not include that file

typedef enum {
    SubBruteWorkerStatusIDLE,
    SubBruteWorkerStatusTx,
    // SubBruteWorkerStatusRx,
} SubBruteWorkerStatus;

//typedef void (*SubBruteWorkerCallback)(SubBruteWorkerStatus event, void* context);
*/
SubBruteWorker* subbrute_worker_alloc();
void subbrute_worker_free(SubBruteWorker* instance);
bool subbrute_worker_start(
    SubBruteWorker* instance,
    uint32_t frequency,
    FuriHalSubGhzPreset preset,
    const char* protocol_name);
void subbrute_worker_stop(SubBruteWorker* instance);
bool subbrute_worker_get_continuous_worker(SubBruteWorker* instance);
void subbrute_worker_set_continuous_worker(SubBruteWorker* instance, bool is_continuous_worker);
//bool subbrute_worker_write(SubBruteWorker* instance, uint8_t* data, size_t size);
bool subbrute_worker_is_running(SubBruteWorker* instance);
bool subbrute_worker_can_transmit(SubBruteWorker* instance);
bool subbrute_worker_can_manual_transmit(SubBruteWorker* instance, bool is_button_pressed);
bool subbrute_worker_transmit(SubBruteWorker* instance, const char* payload);
bool subbrute_worker_init_manual_transmit(
    SubBruteWorker* instance,
    uint32_t frequency,
    FuriHalSubGhzPreset preset,
    const char* protocol_name);
bool subbrute_worker_manual_transmit(SubBruteWorker* instance, const char* payload);
void subbrute_worker_manual_transmit_stop(SubBruteWorker* instance);