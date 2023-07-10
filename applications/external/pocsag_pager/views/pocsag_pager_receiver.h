#pragma once

#include <gui/view.h>
#include "../helpers/pocsag_pager_types.h"
#include "../helpers/pocsag_pager_event.h"

typedef struct PCSGReceiver PCSGReceiver;

typedef void (*PCSGReceiverCallback)(PCSGCustomEvent event, void* context);

void pcsg_receiver_rssi(PCSGReceiver* instance, float rssi);

void pcsg_view_receiver_set_lock(PCSGReceiver* pcsg_receiver, PCSGLock keyboard);

void pcsg_view_receiver_set_ext_module_state(PCSGReceiver* pcsg_receiver, bool is_external);

void pcsg_view_receiver_set_callback(
    PCSGReceiver* pcsg_receiver,
    PCSGReceiverCallback callback,
    void* context);

PCSGReceiver* pcsg_view_receiver_alloc();

void pcsg_view_receiver_free(PCSGReceiver* pcsg_receiver);

View* pcsg_view_receiver_get_view(PCSGReceiver* pcsg_receiver);

void pcsg_view_receiver_add_data_statusbar(
    PCSGReceiver* pcsg_receiver,
    const char* frequency_str,
    const char* preset_str,
    const char* history_stat_str);

void pcsg_view_receiver_add_item_to_menu(
    PCSGReceiver* pcsg_receiver,
    const char* name,
    uint8_t type);

uint16_t pcsg_view_receiver_get_idx_menu(PCSGReceiver* pcsg_receiver);

void pcsg_view_receiver_set_idx_menu(PCSGReceiver* pcsg_receiver, uint16_t idx);

void pcsg_view_receiver_exit(void* context);
