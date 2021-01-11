#pragma once

#include "nfc.h"
#include "nfc_types.h"
#include "nfc_views.h"
#include "nfc_worker.h"

#include <flipper_v2.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <assets_icons.h>

#include <menu/menu.h>
#include <menu/menu_item.h>

struct Nfc {
    osMessageQueueId_t message_queue;

    NfcWorker* worker;

    ValueMutex* menu_vm;
    MenuItem* menu;
    Icon* icon;

    View* view_detect;
    View* view_emulate;
    View* view_field;
    View* view_error;
    ViewDispatcher* view_dispatcher;
};

Nfc* nfc_alloc();

void nfc_menu_detect_callback(void* context);

void nfc_menu_emulate_callback(void* context);

void nfc_menu_field_callback(void* context);

void nfc_start(Nfc* nfc, NfcView view_id, NfcWorkerState worker_state);

void nfc_task(void* p);