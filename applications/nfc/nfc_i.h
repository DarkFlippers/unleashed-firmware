#pragma once

#include "nfc.h"
#include "nfc_types.h"
#include "nfc_views.h"
#include "nfc_worker.h"

#include <furi.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <assets_icons.h>
#include <cli/cli.h>

#include <menu/menu.h>
#include <menu/menu_item.h>
#include <gui/modules/submenu.h>

struct Nfc {
    osMessageQueueId_t message_queue;

    NfcWorker* worker;

    Gui* gui;

    Submenu* submenu;

    View* view_detect;
    View* view_emulate;
    View* view_field;
    View* view_cli;
    View* view_error;
    ViewDispatcher* view_dispatcher;
};

Nfc* nfc_alloc();

void nfc_start(Nfc* nfc, NfcView view_id, NfcWorkerState worker_state);

int32_t nfc_task(void* p);