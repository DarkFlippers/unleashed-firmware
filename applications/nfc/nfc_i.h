#pragma once

#include "nfc.h"
#include "nfc_types.h"
#include "nfc_worker.h"

#include <furi.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <cli/cli.h>

#include <gui/modules/submenu.h>

#include "nfc_detect.h"
#include "nfc_emulate.h"
#include "nfc_emv.h"
#include "nfc_mifare_ul.h"

struct Nfc {
    NfcCommon nfc_common;
    osMessageQueueId_t message_queue;
    Gui* gui;
    Submenu* submenu;
    NfcDetect* nfc_detect;
    NfcEmulate* nfc_emulate;
    NfcEmv* nfc_emv;
    NfcMifareUl* nfc_mifare_ul;
};

typedef enum {
    NfcViewMenu,
    NfcViewDetect,
    NfcViewEmulate,
    NfcViewEmv,
    NfcViewMifareUl,
} NfcView;

Nfc* nfc_alloc();

int32_t nfc_task(void* p);
