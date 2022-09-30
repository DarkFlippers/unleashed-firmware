#pragma once

#include "subbrute_protocols.h"

#define SUBBRUTE_TEXT_STORE_SIZE 256

#define SUBBRUTE_MAX_LEN_NAME 64
#define SUBBRUTE_PATH EXT_PATH("subghz")
#define SUBBRUTE_FILE_EXT ".sub"

#define SUBBRUTE_PAYLOAD_SIZE 16

typedef enum {
    SubBruteFileResultUnknown,
    SubBruteFileResultOk,
    SubBruteFileResultErrorOpenFile,
    SubBruteFileResultMissingOrIncorrectHeader,
    SubBruteFileResultFrequencyNotAllowed,
    SubBruteFileResultMissingOrIncorrectFrequency,
    SubBruteFileResultPresetInvalid,
    SubBruteFileResultMissingProtocol,
    SubBruteFileResultProtocolNotSupported,
    SubBruteFileResultDynamicProtocolNotValid,
    SubBruteFileResultProtocolNotFound,
    SubBruteFileResultMissingOrIncorrectBit,
    SubBruteFileResultMissingOrIncorrectKey,
    SubBruteFileResultMissingOrIncorrectTe,
} SubBruteFileResult;

typedef enum {
    SubBruteDeviceStateIDLE,
    SubBruteDeviceStateReady,
    SubBruteDeviceStateTx,
    SubBruteDeviceStateFinished
} SubBruteDeviceState;

typedef void (*SubBruteDeviceWorkerCallback)(void* context, SubBruteDeviceState state);
typedef struct SubBruteDevice SubBruteDevice;

SubBruteDevice* subbrute_device_alloc();
void subbrute_device_free(SubBruteDevice* instance);
bool subbrute_device_save_file(SubBruteDevice* instance, const char* key_name);
const char* subbrute_device_error_get_desc(SubBruteFileResult error_id);
SubBruteFileResult subbrute_device_attack_set(SubBruteDevice* context, SubBruteAttacks type);
uint8_t subbrute_device_load_from_file(SubBruteDevice* context, string_t file_path);


bool subbrute_device_is_worker_running(SubBruteDevice* instance);
SubBruteAttacks subbrute_device_get_attack(SubBruteDevice* instance);
uint64_t subbrute_device_get_max_value(SubBruteDevice* instance);
uint64_t subbrute_device_get_step(SubBruteDevice* instance);
uint64_t subbrute_device_add_step(SubBruteDevice* instance, int8_t step);
void subbrute_device_set_load_index(SubBruteDevice* instance, uint64_t load_index);
void subbrute_device_reset_step(SubBruteDevice* instance);
const char* subbrute_device_get_file_key(SubBruteDevice* instance);

bool subbrute_worker_start(SubBruteDevice* instance);
void subbrute_worker_stop(SubBruteDevice* instance);
bool subbrute_device_transmit_current_key(SubBruteDevice* instance);
bool subbrute_device_can_manual_transmit(SubBruteDevice* instance);
void subbrute_device_set_callback(
    SubBruteDevice* instance,
    SubBruteDeviceWorkerCallback callback,
    void* context);
