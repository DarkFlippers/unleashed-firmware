#pragma once

#include "bt.h"
#include "bt_views.h"
#include "bt_types.h"

#include <furi.h>
#include <api-hal.h>

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
    osTimerId_t hopping_mode_timer;
    Cli* cli;
    Gui* gui;
    ValueMutex* menu;
    // Status bar
    ViewPort* statusbar_view_port;
    // Menu
    Icon* menu_icon;
    MenuItem* menu_item;
    View* view_test_tone_tx;
    View* view_test_packet_tx;
    View* view_test_tone_rx;
    View* view_start_app;
    ViewDispatcher* view_dispatcher;
};

Bt* bt_alloc();

void bt_draw_statusbar_callback(Canvas* canvas, void* context);

BtTestChannel bt_switch_channel(InputKey key, BtTestChannel inst_chan);

void bt_cli_info(Cli* cli, string_t args, void* context);

void bt_draw_statusbar_callback(Canvas* canvas, void* context);

void bt_menu_test_tone_tx(void* context);

void bt_menu_test_packet_tx(void* context);

void bt_menu_test_tone_rx(void* context);

void bt_menu_start_app(void* context);
