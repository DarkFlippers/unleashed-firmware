#pragma once

#include <gui/view.h>
#include "../helpers/subghz_custom_event.h"

typedef struct SubghzTransmitter SubghzTransmitter;

typedef void (*SubghzTransmitterCallback)(SubghzCustomEvent event, void* context);

void subghz_transmitter_set_callback(
    SubghzTransmitter* subghz_transmitter,
    SubghzTransmitterCallback callback,
    void* context);

SubghzTransmitter* subghz_transmitter_alloc();

void subghz_transmitter_free(SubghzTransmitter* subghz_transmitter);

View* subghz_transmitter_get_view(SubghzTransmitter* subghz_transmitter);

void subghz_transmitter_add_data_to_show(
    SubghzTransmitter* subghz_transmitter,
    const char* key_str,
    const char* frequency_str,
    const char* preset_str,
    uint8_t show_button);
