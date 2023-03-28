//** Includes sniffbt and sniffskim for compatible ESP32-WROOM hardware.
// wifi_marauder_scene_start.c also changed **//
#pragma once

#include "wifi_marauder_app.h"
#include "scenes/wifi_marauder_scene.h"
#include "wifi_marauder_custom_event.h"
#include "wifi_marauder_uart.h"
#include "wifi_marauder_pcap.h"

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/text_box.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/widget.h>

#include <storage/storage.h>
#include <dialogs/dialogs.h>

#define NUM_MENU_ITEMS (17)

#define WIFI_MARAUDER_TEXT_BOX_STORE_SIZE (4096)
#define WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE (512)

#define MARAUDER_APP_FOLDER_USER "apps_data/marauder"
#define MARAUDER_APP_FOLDER EXT_PATH(MARAUDER_APP_FOLDER_USER)
#define MARAUDER_APP_FOLDER_PCAPS MARAUDER_APP_FOLDER "/pcaps"
#define MARAUDER_APP_FOLDER_LOGS MARAUDER_APP_FOLDER "/logs"
#define MARAUDER_APP_FOLDER_USER_PCAPS MARAUDER_APP_FOLDER_USER "/pcaps"
#define MARAUDER_APP_FOLDER_USER_LOGS MARAUDER_APP_FOLDER_USER "/logs"
#define SAVE_PCAP_SETTING_FILEPATH MARAUDER_APP_FOLDER "/save_pcaps_here.setting"
#define SAVE_LOGS_SETTING_FILEPATH MARAUDER_APP_FOLDER "/save_logs_here.setting"

struct WifiMarauderApp {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;

    char text_input_store[WIFI_MARAUDER_TEXT_INPUT_STORE_SIZE + 1];
    FuriString* text_box_store;
    size_t text_box_store_strlen;
    TextBox* text_box;
    TextInput* text_input;
    Storage* storage;
    File* capture_file;
    File* log_file;
    char log_file_path[100];
    File* save_pcap_setting_file;
    File* save_logs_setting_file;
    bool need_to_prompt_settings_init;
    int which_prompt;
    bool ok_to_save_pcaps;
    bool ok_to_save_logs;
    bool has_saved_logs_this_session;
    DialogsApp* dialogs;

    VariableItemList* var_item_list;
    Widget* widget;
    int open_log_file_page;
    int open_log_file_num_pages;

    WifiMarauderUart* uart;
    WifiMarauderUart* lp_uart;
    int selected_menu_index;
    int selected_option_index[NUM_MENU_ITEMS];
    const char* selected_tx_string;
    bool is_command;
    bool is_custom_tx_string;
    bool focus_console_start;
    bool show_stopscan_tip;
    bool is_writing_pcap;
    bool is_writing_log;

    // For input source and destination MAC in targeted deauth attack
    int special_case_input_step;
    char special_case_input_src_addr[20];
    char special_case_input_dst_addr[20];
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
    WifiMarauderAppViewWidget,
} WifiMarauderAppView;
