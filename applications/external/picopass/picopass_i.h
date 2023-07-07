#pragma once

#include "picopass.h"
#include "picopass_worker.h"
#include "picopass_device.h"

#include "rfal_picopass.h"

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

#include "scenes/picopass_scene.h"
#include "views/dict_attack.h"

#include <storage/storage.h>
#include <lib/toolbox/path.h>
#include <picopass_icons.h>

#define PICOPASS_TEXT_STORE_SIZE 128

enum PicopassCustomEvent {
    // Reserve first 100 events for button types and indexes, starting from 0
    PicopassCustomEventReserved = 100,

    PicopassCustomEventViewExit,
    PicopassCustomEventWorkerExit,
    PicopassCustomEventByteInputDone,
    PicopassCustomEventTextInputDone,
    PicopassCustomEventDictAttackSkip,
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
    FuriString* text_box_store;

    // Common Views
    Submenu* submenu;
    Popup* popup;
    Loading* loading;
    TextInput* text_input;
    Widget* widget;
    DictAttack* dict_attack;
};

typedef enum {
    PicopassViewMenu,
    PicopassViewPopup,
    PicopassViewLoading,
    PicopassViewTextInput,
    PicopassViewWidget,
    PicopassViewDictAttack,
} PicopassView;

Picopass* picopass_alloc();

void picopass_text_store_set(Picopass* picopass, const char* text, ...);

void picopass_text_store_clear(Picopass* picopass);

void picopass_blink_start(Picopass* picopass);

void picopass_blink_stop(Picopass* picopass);

void picopass_show_loading_popup(void* context, bool show);

/** Check if memory is set to pattern
 *
 * @warning    zero size will return false
 *
 * @param[in]  data     Pointer to the byte array
 * @param[in]  pattern  The pattern
 * @param[in]  size     The byte array size
 *
 * @return     True if memory is set to pattern, false otherwise
 */
bool picopass_is_memset(const uint8_t* data, const uint8_t pattern, size_t size);
