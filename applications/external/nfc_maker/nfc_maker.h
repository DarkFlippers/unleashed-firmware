#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/modules/validators.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include "nfc_maker_icons.h"
#include <gui/modules/submenu.h>
#include "nfc_maker_text_input.h"
#include <gui/modules/byte_input.h>
#include <gui/modules/popup.h>
#include "scenes/nfc_maker_scene.h"
#include <lib/flipper_format/flipper_format.h>
#include <lib/toolbox/random_name.h>
#include <applications/main/nfc/nfc_i.h>
#include <furi_hal_bt.h>
#include "strnlen.h"

#define TEXT_INPUT_LEN 248
#define WIFI_INPUT_LEN 90

typedef enum {
    WifiAuthenticationOpen = 0x01,
    WifiAuthenticationWpa2Personal = 0x20,
    WifiAuthenticationWpa2Enterprise = 0x10,
    WifiAuthenticationWpaPersonal = 0x02,
    WifiAuthenticationWpaEnterprise = 0x08,
    WifiAuthenticationShared = 0x04,
} WifiAuthentication;

typedef enum {
    WifiEncryptionAes = 0x08,
    WifiEncryptionWep = 0x02,
    WifiEncryptionTkip = 0x04,
    WifiEncryptionNone = 0x01,
} WifiEncryption;

typedef struct {
    Gui* gui;
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    NFCMaker_TextInput* text_input;
    ByteInput* byte_input;
    Popup* popup;

    uint8_t mac_buf[GAP_MAC_ADDR_SIZE];
    char text_buf[TEXT_INPUT_LEN];
    char pass_buf[WIFI_INPUT_LEN];
    char name_buf[TEXT_INPUT_LEN];
} NfcMaker;

typedef enum {
    NfcMakerViewSubmenu,
    NfcMakerViewTextInput,
    NfcMakerViewByteInput,
    NfcMakerViewPopup,
} NfcMakerView;
