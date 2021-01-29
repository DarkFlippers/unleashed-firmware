#pragma once

#include "bt.h"

#include <furi.h>

#include <cli/cli.h>

#include <gui/gui.h>
#include <gui/view_port.h>

#include <menu/menu.h>
#include <menu/menu_item.h>

typedef struct {
    Cli* cli;
    Gui* gui;
    ValueMutex* menu;
    // Status bar
    Icon* statusbar_icon;
    ViewPort* statusbar_view_port;
    // Menu
    Icon* menu_icon;
    MenuItem* menu_item;
} Bt;

Bt* bt_alloc();

void bt_draw_statusbar_callback(Canvas* canvas, void* context);

void bt_cli_info(string_t args, void* context);

void bt_draw_statusbar_callback(Canvas* canvas, void* context);
