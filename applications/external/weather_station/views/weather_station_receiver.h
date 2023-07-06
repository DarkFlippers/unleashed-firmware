#pragma once

#include <gui/view.h>
#include "../helpers/weather_station_types.h"
#include "../helpers/weather_station_event.h"

typedef struct WSReceiver WSReceiver;

typedef void (*WSReceiverCallback)(WSCustomEvent event, void* context);

void ws_view_receiver_set_rssi(WSReceiver* instance, float rssi);

void ws_view_receiver_set_lock(WSReceiver* ws_receiver, WSLock keyboard);

void ws_view_receiver_set_callback(
    WSReceiver* ws_receiver,
    WSReceiverCallback callback,
    void* context);

WSReceiver* ws_view_receiver_alloc();

void ws_view_receiver_free(WSReceiver* ws_receiver);

View* ws_view_receiver_get_view(WSReceiver* ws_receiver);

void ws_view_receiver_add_data_statusbar(
    WSReceiver* ws_receiver,
    const char* frequency_str,
    const char* preset_str,
    const char* history_stat_str,
    bool external);

void ws_view_receiver_add_item_to_menu(WSReceiver* ws_receiver, const char* name, uint8_t type);

uint16_t ws_view_receiver_get_idx_menu(WSReceiver* ws_receiver);

void ws_view_receiver_set_idx_menu(WSReceiver* ws_receiver, uint16_t idx);

void ws_view_receiver_exit(void* context);
