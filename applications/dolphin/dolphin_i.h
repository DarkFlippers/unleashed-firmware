#pragma once

#include "dolphin.h"
#include "dolphin_state.h"
#include "dolphin_views.h"

#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/canvas.h>
#include <menu/menu.h>

#include <assets_icons.h>
#include <stdint.h>

#define UNLOCK_RST_TIMEOUT 500 // keypress counter reset timeout (ms)
#define HINT_TIMEOUT_L 3 // low refresh rate timeout (app ticks)
#define HINT_TIMEOUT_H 40 // high refresh rate timeout (app ticks)

typedef enum {
    DolphinEventTypeDeed,
    DolphinEventTypeSave,
    DolphinEventTypeTick,
} DolphinEventType;

typedef struct {
    DolphinEventType type;
    union {
        DolphinDeed deed;
    };
} DolphinEvent;

struct Dolphin {
    // Internal message queue
    osMessageQueueId_t event_queue;
    // State
    DolphinState* state;
    // Menu
    ValueMutex* menu_vm;
    // Scene
    FuriThread* scene_thread;
    // GUI
    Gui* gui;
    ViewDispatcher* idle_view_dispatcher;
    View* idle_view_first_start;
    View* idle_view_main;
    View* idle_view_dolphin_stats;
    View* idle_view_down;
    View* idle_view_meta;
    View* view_hw_mismatch;
    View* view_lockmenu;
    ViewPort* lock_viewport;
    IconAnimation* lock_icon;

    bool locked;
    uint8_t lock_count;
    uint32_t lock_lastpress;
    osTimerId_t timeout_timer;
};

Dolphin* dolphin_alloc();

void dolphin_free(Dolphin* dolphin);

/* Save Dolphin state (write to permanent memory)
 * Thread safe
 */
void dolphin_save(Dolphin* dolphin);
