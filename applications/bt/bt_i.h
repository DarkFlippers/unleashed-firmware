#pragma once

#include "bt.h"
#include "bt_views.h"
#include "bt_types.h"

#include <furi.h>
#include <furi-hal.h>

#include <cli/cli.h>

#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>

#include <menu/menu.h>
#include <menu/menu_item.h>

struct Bt {
    osMessageQueueId_t message_queue;
    BtState state;
    osTimerId_t update_status_timer;
    osTimerId_t update_param_timer;
    Gui* gui;
    ValueMutex* menu;
    // Status bar
    ViewPort* statusbar_view_port;
    // Menu
    IconAnimation* menu_icon;
    MenuItem* menu_item;
    View* view_test_carrier;
    View* view_test_packet_tx;
    View* view_test_packet_rx;
    View* view_start_app;
    ViewDispatcher* view_dispatcher;
};

Bt* bt_alloc();

void bt_draw_statusbar_callback(Canvas* canvas, void* context);

BtTestChannel bt_switch_channel(InputKey key, BtTestChannel inst_chan);

void bt_draw_statusbar_callback(Canvas* canvas, void* context);

void bt_menu_test_carrier(void* context);

void bt_menu_test_packet_tx(void* context);

void bt_menu_test_packet_rx(void* context);

void bt_menu_start_app(void* context);
