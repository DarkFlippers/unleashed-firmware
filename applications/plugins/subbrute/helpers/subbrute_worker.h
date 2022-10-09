#pragma once

#include "../subbrute_protocols.h"

typedef enum {
    SubBruteWorkerStateIDLE,
    SubBruteWorkerStateReady,
    SubBruteWorkerStateTx,
    SubBruteWorkerStateFinished
} SubBruteWorkerState;

typedef void (*SubBruteWorkerCallback)(void* context, SubBruteWorkerState state);

typedef struct SubBruteWorker SubBruteWorker;

SubBruteWorker* subbrute_worker_alloc();
void subbrute_worker_free(SubBruteWorker* instance);
uint64_t subbrute_worker_get_step(SubBruteWorker* instance);
bool subbrute_worker_set_step(SubBruteWorker* instance, uint64_t step);
bool subbrute_worker_is_running(SubBruteWorker* instance);
bool subbrute_worker_init_default_attack(
    SubBruteWorker* instance,
    SubBruteAttacks attack_type,
    uint64_t step,
    const SubBruteProtocol* protocol);
bool subbrute_worker_init_file_attack(
    SubBruteWorker* instance,
    uint64_t step,
    uint8_t load_index,
    const char* file_key,
    SubBruteProtocol* protocol);
bool subbrute_worker_start(SubBruteWorker* instance);
void subbrute_worker_stop(SubBruteWorker* instance);
bool subbrute_worker_transmit_current_key(SubBruteWorker* instance, uint64_t step);
bool subbrute_worker_can_manual_transmit(SubBruteWorker* instance);
void subbrute_worker_set_callback(
    SubBruteWorker* instance,
    SubBruteWorkerCallback callback,
    void* context);