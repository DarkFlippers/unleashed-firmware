#pragma once

#include "subbrute_worker.h"
#include <lib/subghz/protocols/base.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/environment.h>

struct SubBruteWorker {
    SubBruteWorkerState state;
    volatile bool worker_running;
    volatile bool initiated;
    volatile bool transmit_mode;

    // Current step
    uint64_t step;

    // SubGhz
    FuriThread* thread;
    SubGhzProtocolDecoderBase* decoder_result;
    SubGhzEnvironment* environment;
    SubGhzTransmitter* transmitter;
    const char* protocol_name;

    // Initiated values
    SubBruteAttacks attack; // Attack state
    uint32_t frequency;
    FuriHalSubGhzPreset preset;
    SubBruteFileProtocol file;
    uint8_t bits;
    uint8_t te;
    uint8_t repeat;
    uint8_t load_index; // Index of group to bruteforce in loaded file
    const char* file_key;
    uint64_t max_value; // Max step

    // Manual transmit
    uint32_t last_time_tx_data;

    // Callback for changed states
    SubBruteWorkerCallback callback;
    void* context;
};

int32_t subbrute_worker_thread(void* context);
void subbrute_worker_subghz_transmit(SubBruteWorker* instance, FlipperFormat* flipper_format);
void subbrute_worker_send_callback(SubBruteWorker* instance);