#pragma once

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <toolbox/stream/file_stream.h>
#include <notification/notification_messages.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    int x;
    int y;
} PluginState;

struct FOUND {
    uint8_t addr_size;
    uint8_t addr[5];
    uint16_t total;
};

typedef struct {
    Gui* gui;
    FuriMessageQueue* event_queue;
    PluginState* plugin_state;
    ViewPort* view_port;
    Storage* storage;
    NotificationApp* notification;
    uint8_t* log_arr;
    struct FOUND* found;
} Nrf24Scan;
