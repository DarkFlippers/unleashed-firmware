#pragma once

#include "power.h"

#include <stdint.h>
#include <gui/view_dispatcher.h>
#include <gui/gui.h>

#include <gui/modules/popup.h>
#include "views/power_off.h"

#include <notification/notification-messages.h>

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
    PubSub event_pubsub;
    PowerEvent event;

    PowerState state;
    PowerInfo info;
    osMutexId_t info_mtx;

    bool battery_low;
    uint8_t battery_level;
    uint8_t power_off_timeout;
};

typedef enum {
    PowerViewPopup,
    PowerViewOff,
} PowerView;
