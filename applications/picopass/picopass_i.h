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
#include <gui/modules/loading.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>

#include <input/input.h>

#include <picopass/scenes/picopass_scene.h>

#include <storage/storage.h>
#include <lib/toolbox/path.h>

#define PICOPASS_TEXT_STORE_SIZE 128

enum PicopassCustomEvent {
    // Reserve first 100 events for button types and indexes, starting from 0
    PicopassCustomEventReserved = 100,

    PicopassCustomEventViewExit,
    PicopassCustomEventWorkerExit,
    PicopassCustomEventByteInputDone,
    PicopassCustomEventTextInputDone,
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

    char text_store[PICOPASS_TEXT_STORE_SIZE + 1];
    string_t text_box_store;

    // Common Views
    Submenu* submenu;
    Popup* popup;
    Loading* loading;
    TextInput* text_input;
    Widget* widget;
};

typedef enum {
    PicopassViewMenu,
    PicopassViewPopup,
    PicopassViewLoading,
    PicopassViewTextInput,
    PicopassViewWidget,
} PicopassView;

Picopass* picopass_alloc();

void picopass_text_store_set(Picopass* picopass, const char* text, ...);

void picopass_text_store_clear(Picopass* picopass);

void picopass_blink_start(Picopass* picopass);

void picopass_blink_stop(Picopass* picopass);

void picopass_show_loading_popup(void* context, bool show);
