#pragma once

#include "picopass.h"
#include "picopass_worker.h"
#include "picopass_device.h"

#include <rfal_picopass.h>

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>

#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/widget.h>

#include <input/input.h>

#include <picopass/scenes/picopass_scene.h>

#include <storage/storage.h>
#include <lib/toolbox/path.h>

enum PicopassCustomEvent {
    // Reserve first 100 events for button types and indexes, starting from 0
    PicopassCustomEventReserved = 100,

    PicopassCustomEventViewExit,
    PicopassCustomEventWorkerExit,
    PicopassCustomEventByteInputDone,
    PicopassCustomEventTextInputDone,
    PicopassCustomEventDictAttackDone,
};

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

struct Picopass {
    PicopassWorker* worker;
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    NotificationApp* notifications;
    SceneManager* scene_manager;
    PicopassDevice* dev;

    // Common Views
    Submenu* submenu;
    Popup* popup;
    Widget* widget;
};

typedef enum {
    PicopassViewMenu,
    PicopassViewPopup,
    PicopassViewWidget,
} PicopassView;

Picopass* picopass_alloc();

void picopass_blink_start(Picopass* picopass);

void picopass_blink_stop(Picopass* picopass);
