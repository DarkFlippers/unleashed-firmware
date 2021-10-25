#pragma once

#include "subghz.h"
#include "views/subghz_receiver.h"
#include "views/subghz_transmitter.h"
#include "views/subghz_frequency_analyzer.h"
#include "views/subghz_save_raw.h"

#include "views/subghz_test_static.h"
#include "views/subghz_test_carrier.h"
#include "views/subghz_test_packet.h"

#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <notification/notification-messages.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>

#include <subghz/scenes/subghz_scene.h>

#include <lib/subghz/subghz_worker.h>

#include <lib/subghz/subghz_parser.h>
#include <lib/subghz/protocols/subghz_protocol_common.h>
#include "subghz_history.h"

#include <gui/modules/variable-item-list.h>

#define SUBGHZ_TEXT_STORE_SIZE 40

#define NOTIFICATION_STARTING_STATE 0u
#define NOTIFICATION_IDLE_STATE 1u
#define NOTIFICATION_TX_STATE 2u
#define NOTIFICATION_RX_STATE 3u

extern const char* const subghz_frequencies_text[];
extern const uint32_t subghz_frequencies[];
extern const uint32_t subghz_hopper_frequencies[];
extern const uint32_t subghz_frequencies_count;
extern const uint32_t subghz_hopper_frequencies_count;
extern const uint32_t subghz_frequencies_433_92;

/** SubGhzTxRx state */
typedef enum {
    SubGhzTxRxStateIdle,
    SubGhzTxRxStateRx,
    SubGhzTxRxStateTx,
    SubGhzTxRxStateSleep,
} SubGhzTxRxState;

/** SubGhzHopperState state */
typedef enum {
    SubGhzHopperStateOFF,
    SubGhzHopperStateRunnig,
    SubGhzHopperStatePause,
    SubGhzHopperStateRSSITimeOut,
} SubGhzHopperState;

struct SubGhzTxRx {
    SubGhzWorker* worker;
    SubGhzParser* parser;
    SubGhzProtocolCommon* protocol_result;
    //SubGhzProtocolCommon* protocol_save_raw;
    SubGhzProtocolCommonEncoder* encoder;
    uint32_t frequency;
    FuriHalSubGhzPreset preset;
    SubGhzHistory* history;
    uint16_t idx_menu_chosen;
    SubGhzTxRxState txrx_state;
    //bool hopper_runing;
    SubGhzHopperState hopper_state;
    uint8_t hopper_timeout;
    uint8_t hopper_idx_frequency;
};

typedef struct SubGhzTxRx SubGhzTxRx;

struct SubGhz {
    Gui* gui;
    NotificationApp* notifications;

    SubGhzTxRx* txrx;

    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    Submenu* submenu;
    Popup* popup;
    TextInput* text_input;
    Widget* widget;
    char file_name[SUBGHZ_TEXT_STORE_SIZE + 1];
    char file_name_tmp[SUBGHZ_TEXT_STORE_SIZE + 1];
    uint8_t state_notifications;

    SubghzReceiver* subghz_receiver;
    SubghzTransmitter* subghz_transmitter;
    VariableItemList* variable_item_list;

    SubghzFrequencyAnalyzer* subghz_frequency_analyzer;
    SubghzSaveRAW* subghz_save_raw;
    SubghzTestStatic* subghz_test_static;
    SubghzTestCarrier* subghz_test_carrier;
    SubghzTestPacket* subghz_test_packet;
    string_t error_str;
};

typedef enum {
    SubGhzViewMenu,

    SubGhzViewReceiver,
    SubGhzViewPopup,
    SubGhzViewTextInput,
    SubGhzViewWidget,
    SubGhzViewTransmitter,
    SubGhzViewVariableItemList,
    SubGhzViewFrequencyAnalyzer,
    SubGhzViewSaveRAW,
    SubGhzViewStatic,
    SubGhzViewTestCarrier,
    SubGhzViewTestPacket,
} SubGhzView;

void subghz_begin(SubGhz* subghz, FuriHalSubGhzPreset preset);
uint32_t subghz_rx(SubGhz* subghz, uint32_t frequency);
void subghz_rx_end(SubGhz* subghz);
void subghz_sleep(SubGhz* subghz);
bool subghz_tx_start(SubGhz* subghz);
void subghz_tx_stop(SubGhz* subghz);
bool subghz_key_load(SubGhz* subghz, const char* file_path);
bool subghz_save_protocol_to_file(SubGhz* subghz, const char* dev_name);
bool subghz_load_protocol_from_file(SubGhz* subghz);
bool subghz_rename_file(SubGhz* subghz);
bool subghz_delete_file(SubGhz* subghz);
void subghz_file_name_clear(SubGhz* subghz);
uint32_t subghz_random_serial(void);
void subghz_hopper_update(SubGhz* subghz);
