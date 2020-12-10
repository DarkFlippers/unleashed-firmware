#pragma once

#include "bt.h"

#include <cli/cli.h>
#include <flipper.h>
#include <flipper_v2.h>
#include <gui/gui.h>
#include <gui/widget.h>

typedef struct {
    Cli* cli;
    Icon* statusbar_icon;
    Widget* statusbar_widget;
} Bt;

Bt* bt_alloc();

void bt_draw_statusbar_callback(CanvasApi* canvas, void* context);
