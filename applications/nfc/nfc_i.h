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
#include <gui/modules/widget.h>

#include "views/bank_card.h"

#include <nfc/scenes/nfc_scene.h>

#define NFC_SEND_NOTIFICATION_FALSE (0UL)
#define NFC_SEND_NOTIFICATION_TRUE (1UL)
#define NFC_TEXT_STORE_SIZE 128

struct Nfc {
    NfcWorker* worker;
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    NotificationApp* notifications;
    SceneManager* scene_manager;
    NfcDevice* dev;
    NfcDeviceCommonData dev_edit_data;

    char text_store[NFC_TEXT_STORE_SIZE + 1];
    string_t text_box_store;

    // Common Views
    Submenu* submenu;
    DialogEx* dialog_ex;
    Popup* popup;
    TextInput* text_input;
    ByteInput* byte_input;
    TextBox* text_box;
    Widget* widget;
    BankCard* bank_card;
};

typedef enum {
    NfcViewMenu,
    NfcViewDialogEx,
    NfcViewPopup,
    NfcViewTextInput,
    NfcViewByteInput,
    NfcViewTextBox,
    NfcViewWidget,
    NfcViewBankCard,
} NfcView;

Nfc* nfc_alloc();

int32_t nfc_task(void* p);

void nfc_text_store_set(Nfc* nfc, const char* text, ...);

void nfc_text_store_clear(Nfc* nfc);
