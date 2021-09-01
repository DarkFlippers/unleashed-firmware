#pragma once

#include <gui/view.h>
#include <lib/subghz/protocols/subghz_protocol_common.h>
#include <lib/subghz/protocols/subghz_protocol.h>
#include <lib/subghz/subghz_worker.h>
#include "../subghz_history.h"

typedef enum {
    SubghzReceverEventOK,
    SubghzReceverEventConfig,
    SubghzReceverEventMain,
    SubghzReceverEventSave,
    SubghzReceverEventBack,
    SubghzReceverEventMore,
    SubghzReceverEventSendStart,
    SubghzReceverEventSendStop,
    SubghzReceverEventSendHistoryFull,
} SubghzReceverEvent;

typedef struct SubghzReceiver SubghzReceiver;

typedef void (*SubghzReceiverCallback)(SubghzReceverEvent event, void* context);

void subghz_receiver_set_callback(
    SubghzReceiver* subghz_receiver,
    SubghzReceiverCallback callback,
    void* context);

SubghzReceiver* subghz_receiver_alloc();

void subghz_receiver_free(SubghzReceiver* subghz_receiver);

View* subghz_receiver_get_view(SubghzReceiver* subghz_receiver);

void subghz_receiver_set_protocol(
    SubghzReceiver* subghz_receiver,
    SubGhzProtocolCommon* protocol_result,
    SubGhzProtocol* protocol);

SubGhzProtocolCommon* subghz_receiver_get_protocol(SubghzReceiver* subghz_receiver);

void subghz_receiver_set_worker(SubghzReceiver* subghz_receiver, SubGhzWorker* worker);

uint32_t subghz_receiver_get_frequency(SubghzReceiver* subghz_receiver);

FuriHalSubGhzPreset subghz_receiver_get_preset(SubghzReceiver* subghz_receiver);

void subghz_receiver_frequency_preset_to_str(SubghzReceiver* subghz_receiver, string_t output);
