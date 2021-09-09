#pragma once

#include "bt.h"

#include <furi.h>
#include <furi-hal.h>

#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/view.h>

#include "../bt_settings.h"

typedef enum {
    BtMessageTypeUpdateStatusbar,
    BtMessageTypeUpdateBatteryLevel,
} BtMessageType;

typedef union {
    uint8_t battery_level;
} BtMessageData;

typedef struct {
    BtMessageType type;
    BtMessageData data;
} BtMessage;

struct Bt {
    BtSettings bt_settings;
    osMessageQueueId_t message_queue;
    osTimerId_t update_status_timer;
    Gui* gui;
    ViewPort* statusbar_view_port;
};
