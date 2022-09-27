#pragma once

#include <lib/toolbox/stream/stream.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
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
    SubBruteAttackCAME12bit307,
    SubBruteAttackCAME12bit433,
    SubBruteAttackCAME12bit868,
    SubBruteAttackChamberlain9bit300,
    SubBruteAttackChamberlain9bit315,
    SubBruteAttackChamberlain9bit390,
    SubBruteAttackLinear10bit300,
    SubBruteAttackLinear10bit310,
    SubBruteAttackNICE12bit433,
    SubBruteAttackNICE12bit868,
    SubBruteAttackLoadFile,
    SubBruteAttackTotalCount,
} SubBruteAttacks;

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
    SubBruteDeviceStateFinished,
} SubBruteDeviceState;

typedef struct {
    SubBruteDeviceState state;

    // Current step
    uint64_t key_index;
    string_t load_path;
    // Index of group to bruteforce in loaded file
    uint8_t load_index;

    SubGhzReceiver* receiver;
    SubGhzProtocolDecoderBase* decoder_result;
    SubGhzEnvironment* environment;

    // Attack state
    SubBruteAttacks attack;
    char file_template[SUBBRUTE_TEXT_STORE_SIZE];
    bool has_tail;
    char payload[SUBBRUTE_TEXT_STORE_SIZE * 2];
    uint64_t max_value;

    // Loaded info for attack type
    FuriHalSubGhzPreset preset;
    string_t preset_name;
    string_t protocol_name;
    uint32_t frequency;
    uint32_t repeat;
    uint32_t bit;
    char current_key[SUBBRUTE_PAYLOAD_SIZE];
    uint32_t te;

    char file_key[SUBBRUTE_MAX_LEN_NAME];
    char text_store[SUBBRUTE_PAYLOAD_SIZE];
} SubBruteDevice;

SubBruteDevice* subbrute_device_alloc();
void subbrute_device_free(SubBruteDevice* instance);
bool subbrute_device_save_file(SubBruteDevice* instance, const char* key_name);
const char* subbrute_device_error_get_desc(SubBruteFileResult error_id);
bool subbrute_device_create_packet_parsed(SubBruteDevice* context, uint64_t step, bool small);
SubBruteFileResult subbrute_device_attack_set(SubBruteDevice* context, SubBruteAttacks type);
uint8_t subbrute_device_load_from_file(SubBruteDevice* context, string_t file_path);
FuriHalSubGhzPreset subbrute_device_convert_preset(const char* preset);
void subbrute_device_attack_set_default_values(
    SubBruteDevice* context,
    SubBruteAttacks default_attack);