#pragma once

#include "subbrute_device_i.h"
#include <lib/subghz/protocols/base.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/environment.h>

struct SubBruteDevice {
    SubBruteDeviceState state;
    SubBruteProtocol* protocol_info;
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
};

/*
 * PRIVATE METHODS
 */
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