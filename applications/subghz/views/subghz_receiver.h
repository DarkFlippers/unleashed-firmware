#pragma once

#include <gui/view.h>

typedef enum {
    SubghzReceverEventOK,
    SubghzReceverEventConfig,
    SubghzReceverEventBack,
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

void subghz_receiver_add_data_statusbar(
    SubghzReceiver* subghz_receiver,
    const char* frequency_str,
    const char* preset_str,
    const char* history_stat_str);

void subghz_receiver_add_item_to_menu(
    SubghzReceiver* subghz_receiver,
    const char* name,
    uint8_t type);

uint16_t subghz_receiver_get_idx_menu(SubghzReceiver* subghz_receiver);

void subghz_receiver_set_idx_menu(SubghzReceiver* subghz_receiver, uint16_t idx);

void subghz_receiver_exit(void* context);