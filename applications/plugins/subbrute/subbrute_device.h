#pragma once

#include "subbrute_protocols.h"
#include <lib/subghz/protocols/base.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/environment.h>

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
typedef struct {
    SubBruteDeviceState state;
    const SubBruteProtocol* protocol_info;
    SubBruteProtocol* file_protocol_info;
    volatile bool worker_running;

    // Current step
    uint64_t key_index;
    // Index of group to bruteforce in loaded file
    uint8_t load_index;

    // SubGhz
    FuriThread* thread;
    SubGhzReceiver* receiver;
    SubGhzProtocolDecoderBase* decoder_result;
    SubGhzEnvironment* environment;
    SubGhzTransmitter* transmitter;

    // Attack state
    SubBruteAttacks attack;
    char file_template[SUBBRUTE_TEXT_STORE_SIZE];

    uint64_t max_value;

    // Loaded info for attack type
    char current_key[SUBBRUTE_PAYLOAD_SIZE];
    char file_key[SUBBRUTE_MAX_LEN_NAME];

    // Manual transmit
    uint32_t last_time_tx_data;

    // Callback for changed states
    SubBruteDeviceWorkerCallback callback;
    void* context;
} SubBruteDevice;

SubBruteDevice* subbrute_device_alloc();
void subbrute_device_free(SubBruteDevice* instance);

bool subbrute_device_save_file(SubBruteDevice* instance, const char* key_name);
const char* subbrute_device_error_get_desc(SubBruteFileResult error_id);
SubBruteFileResult subbrute_device_attack_set(SubBruteDevice* context, SubBruteAttacks type);
uint8_t subbrute_device_load_from_file(SubBruteDevice* context, const char* file_path);

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

void subbrute_device_free_protocol_info(SubBruteDevice* instance);
int32_t subbrute_worker_thread(void* context);
void subbrute_device_attack_set_default_values(
    SubBruteDevice* context,
    SubBruteAttacks default_attack);
bool subbrute_device_create_packet_parsed(
    SubBruteDevice* instance,
    FlipperFormat* flipper_format,
    uint64_t step,
    bool small);
void subbrute_device_send_callback(SubBruteDevice* instance);
void subbrute_device_subghz_transmit(SubBruteDevice* instance, FlipperFormat* flipper_format);
