#pragma once

#include "wifi_marauder_app.h"
#include "scenes/wifi_marauder_scene.h"
#include "wifi_marauder_custom_event.h"
#include "wifi_marauder_uart.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_box.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>

#define NUM_MENU_ITEMS (15)

#define WIFI_MARAUDER_TEXT_BOX_STORE_SIZE (4096)
#define WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE (512)

struct WifiMarauderApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    char text_input_store[WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE + 1];
    string_t text_box_store;
    size_t text_box_store_strlen;
    TextBox* text_box;
    TextInput* text_input;
    //Widget* widget;

    VariableItemList* var_item_list;

    WifiMarauderUart* uart;
    int selected_menu_index;
    int selected_option_index[NUM_MENU_ITEMS];
    const char* selected_tx_string;
    bool is_command;
    bool is_custom_tx_string;
    bool focus_console_start;
    bool show_stopscan_tip;
};

// Supported commands:
// https://github.com/justcallmekoko/ESP32Marauder/wiki/cli
//   Scan
//    -> If list is empty, then start a new scanap. (Tap any button to stop.)
//    -> If there's a list, provide option to rescan and dump list of targets to select.
//    -> Press BACK to go back to top-level.
//   Attack
//    -> Beacon
//    -> Deauth
//    -> Probe
//    -> Rickroll
//   Sniff
//    -> Beacon
//    -> Deauth
//    -> ESP
//    -> PMKID
//    -> Pwnagotchi
//   Channel
//   Update
//   Reboot

typedef enum {
    WifiMarauderAppViewVarItemList,
    WifiMarauderAppViewConsoleOutput,
    WifiMarauderAppViewTextInput,
} WifiMarauderAppView;
