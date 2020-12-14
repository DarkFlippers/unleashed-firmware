#pragma once

#include "bt.h"

#include <cli/cli.h>

#include <flipper.h>
#include <flipper_v2.h>

#include <gui/gui.h>
#include <gui/widget.h>

#include <menu/menu.h>
#include <menu/menu_item.h>

typedef struct {
    Cli* cli;
    // Status bar
    Icon* statusbar_icon;
    Widget* statusbar_widget;
    // Menu
    Icon* menu_icon;
    MenuItem* menu_item;
} Bt;

Bt* bt_alloc();

void bt_draw_statusbar_callback(Canvas* canvas, void* context);
