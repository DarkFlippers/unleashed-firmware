#pragma once

#include <gui/view.h>
#include <lib/subghz/protocols/subghz_protocol_common.h>

typedef enum {
    SubghzTransmitterEventSendStart,
    SubghzTransmitterEventSendStop,
    SubghzTransmitterEventBack,
} SubghzTransmitterEvent;

typedef struct SubghzTransmitter SubghzTransmitter;

typedef void (*SubghzTransmitterCallback)(SubghzTransmitterEvent event, void* context);

void subghz_transmitter_set_callback(
    SubghzTransmitter* subghz_transmitter,
    SubghzTransmitterCallback callback,
    void* context);

SubghzTransmitter* subghz_transmitter_alloc();

void subghz_transmitter_free(SubghzTransmitter* subghz_transmitter);

View* subghz_transmitter_get_view(SubghzTransmitter* subghz_transmitter);

void subghz_transmitter_set_protocol(
    SubghzTransmitter* subghz_transmitter,
    SubGhzProtocolCommon* protocol);
void subghz_transmitter_set_frequency_preset(
    SubghzTransmitter* subghz_transmitter,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);
