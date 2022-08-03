#pragma once

#include "power.h"

#include <stdint.h>
#include <gui/view_dispatcher.h>
#include <gui/gui.h>

#include <gui/modules/popup.h>
#include "views/power_off.h"

#include <notification/notification_messages.h>

#define POWER_BATTERY_HEALTHY_LEVEL 70

typedef enum {
    PowerStateNotCharging,
    PowerStateCharging,
    PowerStateCharged,
} PowerState;

struct Power {
    ViewDispatcher* view_dispatcher;
    Popup* popup;
    PowerOff* power_off;

    ViewPort* battery_view_port;
    Gui* gui;
    NotificationApp* notification;
    FuriPubSub* event_pubsub;
    PowerEvent event;

    PowerState state;
    PowerInfo info;

    bool battery_low;
    bool show_low_bat_level_message;
    uint8_t battery_level;
    uint8_t power_off_timeout;

    FuriMutex* api_mtx;
};

typedef enum {
    PowerViewPopup,
    PowerViewOff,
} PowerView;
