#pragma once

#include "nfc.h"
#include "nfc_types.h"
#include "nfc_worker.h"
#include "nfc_device.h"

#include <furi.h>

#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <cli/cli.h>
#include <notification/notification-messages.h>

#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/text_box.h>

#include "views/nfc_detect.h"
#include "views/nfc_emulate.h"
#include "views/nfc_emv.h"
#include "views/nfc_mifare_ul.h"

#include "scenes/nfc_scene_start.h"
#include "scenes/nfc_scene_read_card.h"
#include "scenes/nfc_scene_read_card_success.h"
#include "scenes/nfc_scene_card_menu.h"
#include "scenes/nfc_scene_emulate_uid.h"
#include "scenes/nfc_scene_not_implemented.h"
#include "scenes/nfc_scene_save_name.h"
#include "scenes/nfc_scene_save_success.h"
#include "scenes/nfc_scene_file_select.h"
#include "scenes/nfc_scene_saved_menu.h"
#include "scenes/nfc_scene_set_type.h"
#include "scenes/nfc_scene_set_sak.h"
#include "scenes/nfc_scene_set_atqa.h"
#include "scenes/nfc_scene_set_uid.h"
#include "scenes/nfc_scene_scripts_menu.h"
#include "scenes/nfc_scene_read_mifare_ul.h"
#include "scenes/nfc_scene_read_mifare_ul_success.h"
#include "scenes/nfc_scene_mifare_ul_menu.h"

// TODO delete debug scenes
#include "scenes/nfc_scene_debug_menu.h"
#include "scenes/nfc_scene_debug_detect.h"
#include "scenes/nfc_scene_debug_emulate.h"
#include "scenes/nfc_scene_debug_read_emv.h"
#include "scenes/nfc_scene_debug_read_mifare_ul.h"

#define NFC_TEXT_STORE_SIZE 128

struct Nfc {
    NfcCommon nfc_common;
    Gui* gui;
    NotificationApp* notifications;
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

    // Scenes
    AppScene* scene_start;
    AppScene* scene_read_card;
    AppScene* scene_read_card_success;
    AppScene* scene_card_menu;
    AppScene* scene_not_implemented;
    AppScene* scene_emulate_uid;
    AppScene* scene_save_name;
    AppScene* scene_save_success;
    AppScene* scene_file_select;
    AppScene* scene_saved_menu;
    AppScene* scene_set_type;
    AppScene* scene_set_sak;
    AppScene* scene_set_atqa;
    AppScene* scene_set_uid;
    AppScene* scene_scripts_menu;
    AppScene* scene_read_mifare_ul;
    AppScene* scene_read_mifare_ul_success;
    AppScene* scene_mifare_ul_menu;

    // TODO delete debug scenes
    AppScene* scene_debug_menu;
    AppScene* scene_debug_detect;
    AppScene* scene_debug_emulate;
    AppScene* scene_debug_read_emv;
    AppScene* scene_debug_read_mifare_ul;
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

typedef enum {
    NfcSceneStart,
    NfcSceneReadCard,
    NfcSceneReadCardSuccess,
    NfcSceneCardMenu,
    NfcSceneEmulateUID,
    NfcSceneNotImplemented,
    NfcSceneDebugMenu,
    NfcSceneDebugDetect,
    NfcSceneDebugEmulate,
    NfcSceneDebugReadEmv,
    NfcSceneDebugReadMifareUl,
    NfcSceneSaveName,
    NfcSceneSaveSuccess,
    NfcSceneFileSelect,
    NfcSceneSavedMenu,
    NfcSceneSetType,
    NfcSceneSetSak,
    NfcSceneSetAtqa,
    NfcSceneSetUid,
    NfcSceneScriptsMenu,
    NfcSceneReadMifareUl,
    NfcSceneReadMifareUlSuccess,
    NfcSceneReadMifareUlMenu,
} NfcScene;

Nfc* nfc_alloc();

int32_t nfc_task(void* p);

void nfc_set_text_store(Nfc* nfc, const char* text, ...);
