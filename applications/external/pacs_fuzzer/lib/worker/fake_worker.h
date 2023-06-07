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

/**
 * Allocate FuzzerWorker
 * 
 * @return FuzzerWorker* pointer to FuzzerWorker
 */
FuzzerWorker* fuzzer_worker_alloc();

/**
 * Free FuzzerWorker
 * 
 * @param instance Pointer to a FuzzerWorker
 */
void fuzzer_worker_free(FuzzerWorker* instance);

/**
 * Start or continue emulation
 * 
 * @param instance Pointer to a FuzzerWorker
 * @param idle_time Delay between emulations in tenths of a second
 * @param emu_time Emulation time of one UID in tenths of a second
 * @return bool True if emulation has started
 */
bool fuzzer_worker_start(FuzzerWorker* instance, uint8_t idle_time, uint8_t emu_time);

/**
 * Stop emulation and deinit worker
 * 
 * @param instance Pointer to a FuzzerWorker
 */
void fuzzer_worker_stop(FuzzerWorker* instance);

/**
 * Suspend emulation
 * 
 * @param instance Pointer to a FuzzerWorker
 */
void fuzzer_worker_pause(FuzzerWorker* instance);

/**
 * Init attack by default dictionary
 * 
 * @param instance Pointer to a FuzzerWorker
 * @param protocol_index index of the selected protocol
 * @return bool True if initialization is successful
 */
bool fuzzer_worker_init_attack_dict(FuzzerWorker* instance, FuzzerProtocolsID protocol_index);

/**
 * Init attack by custom dictionary
 * 
 * @param instance Pointer to a FuzzerWorker
 * @param protocol_index index of the selected protocol
 * @param file_path file path to the dictionary
 * @return bool True if initialization is successful
 */
bool fuzzer_worker_init_attack_file_dict(
    FuzzerWorker* instance,
    FuzzerProtocolsID protocol_index,
    FuriString* file_path);

/**
 * Init attack brute force one of byte
 * 
 * @param instance Pointer to a FuzzerWorker
 * @param protocol_index index of the selected protocol
 * @param new_uid Pointer to a FuzzerPayload with UID for brute force
 * @param chosen index of chusen byte
 * @return bool True if initialization is successful
 */
bool fuzzer_worker_init_attack_bf_byte(
    FuzzerWorker* instance,
    FuzzerProtocolsID protocol_index,
    const FuzzerPayload* new_uid,
    uint8_t chusen);

/**
 * Get current UID
 * 
 * @param instance Pointer to a FuzzerWorker
 * @param output_key Pointer to a FuzzerPayload
 */
void fuzzer_worker_get_current_key(FuzzerWorker* instance, FuzzerPayload* output_key);

/**
 * Load UID from Flipper Format Key file
 * 
 * @param instance Pointer to a FuzzerWorker
 * @param protocol_index index of the selected protocol
 * @param filename file path to the key file
 * @return bool True if loading is successful
 */
bool fuzzer_worker_load_key_from_file(
    FuzzerWorker* instance,
    FuzzerProtocolsID protocol_index,
    const char* filename);

/**
 * Set callback for uid changed
 * 
 * @param instance Pointer to a FuzzerWorker
 * @param callback Callback for uid changed
 * @param context Context for callback
 */
void fuzzer_worker_set_uid_chaged_callback(
    FuzzerWorker* instance,
    FuzzerWorkerUidChagedCallback callback,
    void* context);

/**
 * Set callback for end of emulation
 * 
 * @param instance Pointer to a FuzzerWorker
 * @param callback Callback for end of emulation
 * @param context Context for callback
 */
void fuzzer_worker_set_end_callback(
    FuzzerWorker* instance,
    FuzzerWorkerEndCallback callback,
    void* context);