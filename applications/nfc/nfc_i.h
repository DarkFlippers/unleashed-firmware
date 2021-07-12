#pragma once

#include "nfc.h"
#include "nfc_types.h"
#include "nfc_worker.h"
#include "nfc_device.h"

#include <furi.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <cli/cli.h>
#include <notification/notification-messages.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/text_box.h>

#include <nfc/scenes/nfc_scene.h>

#include "views/nfc_detect.h"
#include "views/nfc_emulate.h"
#include "views/nfc_emv.h"
#include "views/nfc_mifare_ul.h"

#define NFC_TEXT_STORE_SIZE 128

struct Nfc {
    NfcCommon nfc_common;
    Gui* gui;
    NotificationApp* notifications;
    SceneManager* scene_manager;
    NfcDevice device;

    char text_store[NFC_TEXT_STORE_SIZE + 1];
    string_t text_box_store;

    // Nfc Views
    NfcDetect* nfc_detect;
    NfcEmulate* nfc_emulate;
    NfcEmv* nfc_emv;
    NfcMifareUl* nfc_mifare_ul;

    // Common Views
    Submenu* submenu;
    DialogEx* dialog_ex;
    Popup* popup;
    TextInput* text_input;
    ByteInput* byte_input;
    TextBox* text_box;
};

typedef enum {
    NfcViewMenu,
    NfcViewDialogEx,
    NfcViewPopup,
    NfcViewTextInput,
    NfcViewByteInput,
    NfcViewTextBox,
    NfcViewDetect,
    NfcViewEmulate,
    NfcViewEmv,
    NfcViewMifareUl,
} NfcView;

Nfc* nfc_alloc();

int32_t nfc_task(void* p);

void nfc_text_store_set(Nfc* nfc, const char* text, ...);

void nfc_text_store_clear(Nfc* nfc);
