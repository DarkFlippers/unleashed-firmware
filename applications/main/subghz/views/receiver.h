#pragma once

#include <gui/view.h>
#include "../helpers/subghz_types.h"
#include "../helpers/subghz_custom_event.h"

typedef struct SubGhzViewReceiver SubGhzViewReceiver;

typedef void (*SubGhzViewReceiverCallback)(SubGhzCustomEvent event, void* context);

void subghz_view_receiver_set_mode(
    SubGhzViewReceiver* subghz_receiver,
    SubGhzViewReceiverMode mode);

void subghz_receiver_rssi(SubGhzViewReceiver* instance, float rssi);

void subghz_view_receiver_set_lock(SubGhzViewReceiver* subghz_receiver, bool keyboard);

void subghz_view_receiver_set_callback(
    SubGhzViewReceiver* subghz_receiver,
    SubGhzViewReceiverCallback callback,
    void* context);

SubGhzViewReceiver* subghz_view_receiver_alloc();

void subghz_view_receiver_free(SubGhzViewReceiver* subghz_receiver);

View* subghz_view_receiver_get_view(SubGhzViewReceiver* subghz_receiver);

void subghz_view_receiver_add_data_statusbar(
    SubGhzViewReceiver* subghz_receiver,
    const char* frequency_str,
    const char* preset_str,
    const char* history_stat_str);

void subghz_view_receiver_set_radio_device_type(
    SubGhzViewReceiver* subghz_receiver,
    SubGhzRadioDeviceType device_type);

void subghz_view_receiver_add_data_progress(
    SubGhzViewReceiver* subghz_receiver,
    const char* progress_str);

void subghz_view_receiver_add_item_to_menu(
    SubGhzViewReceiver* subghz_receiver,
    const char* name,
    const char* time,
    uint8_t type);

uint16_t subghz_view_receiver_get_idx_menu(SubGhzViewReceiver* subghz_receiver);

void subghz_view_receiver_set_idx_menu(SubGhzViewReceiver* subghz_receiver, uint16_t idx);

void subghz_view_receiver_delete_element_callback(SubGhzViewReceiver* subghz_receiver);

void subghz_view_receiver_enable_draw_callback(SubGhzViewReceiver* subghz_receiver);

void subghz_view_receiver_disable_draw_callback(SubGhzViewReceiver* subghz_receiver);

void subghz_view_receiver_exit(void* context);
