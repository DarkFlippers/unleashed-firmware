#pragma once

#include <furi.h>

#include "protocol.h"

typedef enum {
    FuzzerWorkerAttackTypeDefaultDict = 0,
    FuzzerWorkerAttackTypeLoadFile,
    FuzzerWorkerAttackTypeLoadFileCustomUids,

    FuzzerWorkerAttackTypeMax,
} FuzzerWorkerAttackType;

typedef void (*FuzzerWorkerUidChagedCallback)(void* context);
typedef void (*FuzzerWorkerEndCallback)(void* context);

typedef struct FuzzerWorker FuzzerWorker;

FuzzerWorker* fuzzer_worker_alloc();

void fuzzer_worker_free(FuzzerWorker* worker);

bool fuzzer_worker_start(FuzzerWorker* worker, uint8_t timer_dellay);

void fuzzer_worker_stop(FuzzerWorker* worker);

bool fuzzer_worker_attack_dict(FuzzerWorker* worker, FuzzerProtos protocol_index);

bool fuzzer_worker_attack_bf_byte(
    FuzzerWorker* worker,
    FuzzerProtos protocol_index,
    const uint8_t* uid,
    uint8_t chusen);

bool fuzzer_worker_attack_file_dict(
    FuzzerWorker* worker,
    FuzzerProtos protocol_index,
    FuriString* file_path);

void fuzzer_worker_get_current_key(FuzzerWorker* worker, uint8_t* key);

bool fuzzer_worker_load_key_from_file(
    FuzzerWorker* worker,
    FuzzerProtos protocol_index,
    const char* filename);

void fuzzer_worker_set_uid_chaged_callback(
    FuzzerWorker* worker,
    FuzzerWorkerUidChagedCallback callback,
    void* context);

void fuzzer_worker_set_end_callback(
    FuzzerWorker* worker,
    FuzzerWorkerEndCallback callback,
    void* context);