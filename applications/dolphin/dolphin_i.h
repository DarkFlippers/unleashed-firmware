#pragma once

#include "dolphin.h"
#include "dolphin_state.h"

#include <flipper_v2.h>

#include <gui/gui.h>
#include <gui/widget.h>
#include <gui/canvas.h>
#include <menu/menu.h>

#include <assets_icons.h>

#include <stdint.h>

typedef enum {
    DolphinEventTypeDeed,
} DolphinEventType;

typedef struct {
    DolphinEventType type;
    union {
        DolphinDeed deed;
    };
} DolphinEvent;

typedef enum {
    DolphinScreenDebug,
    DolphinScreenIdle,
    DolphinScreenStats,
} DolphinScreen;

struct Dolphin {
    Icon* icon;
    Widget* widget;
    ValueMutex* menu_vm;
    // State
    DolphinState* state;
    DolphinScreen screen;
    // Internal message queue
    osMessageQueueId_t event_queue;
};

void dolphin_draw_callback(Canvas* canvas, void* context);
void dolphin_draw_idle(Canvas* canvas, Dolphin* dolphin);
void dolphin_draw_debug(Canvas* canvas, Dolphin* dolphin);
void dolphin_draw_stats(Canvas* canvas, Dolphin* dolphin);
void dolphin_input_callback(InputEvent* event, void* context);

Dolphin* dolphin_alloc();
