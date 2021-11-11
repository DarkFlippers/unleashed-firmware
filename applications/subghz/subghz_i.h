#pragma once

#include "subghz.h"
#include "views/subghz_receiver.h"
#include "views/subghz_transmitter.h"
#include "views/subghz_frequency_analyzer.h"
#include "views/subghz_read_raw.h"

#include "views/subghz_test_static.h"
#include "views/subghz_test_carrier.h"
#include "views/subghz_test_packet.h"

#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
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

extern const char* const subghz_frequencies_text[];
extern const uint32_t subghz_frequencies[];
extern const uint32_t subghz_hopper_frequencies[];
extern const uint32_t subghz_frequencies_count;
extern const uint32_t subghz_hopper_frequencies_count;
extern const uint32_t subghz_frequencies_433_92;

/** SubGhzNotification state */
typedef enum {
    SubGhzNotificationStateStarting,
    SubGhzNotificationStateIDLE,
    SubGhzNotificationStateTX,
    SubGhzNotificationStateRX,
} SubGhzNotificationState;

/** SubGhzTxRx state */
typedef enum {
    SubGhzTxRxStateIDLE,
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

/** SubGhzRxKeyState state */
typedef enum {
    SubGhzRxKeyStateIDLE,
    SubGhzRxKeyStateNoSave,
    SubGhzRxKeyStateNeedSave,
    SubGhzRxKeyStateAddKey,
    SubGhzRxKeyStateExit,
} SubGhzRxKeyState;

struct SubGhzTxRx {
    SubGhzWorker* worker;
    SubGhzParser* parser;
    SubGhzProtocolCommon* protocol_result;
    SubGhzProtocolCommonEncoder* encoder;
    uint32_t frequency;
    FuriHalSubGhzPreset preset;
    SubGhzHistory* history;
    uint16_t idx_menu_chosen;
    SubGhzTxRxState txrx_state;
    SubGhzHopperState hopper_state;
    uint8_t hopper_timeout;
    uint8_t hopper_idx_frequency;
    SubGhzRxKeyState rx_key_state;
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
    DialogsApp* dialogs;
    char file_name[SUBGHZ_TEXT_STORE_SIZE + 1];
    char file_name_tmp[SUBGHZ_TEXT_STORE_SIZE + 1];
    SubGhzNotificationState state_notifications;

    SubghzReceiver* subghz_receiver;
    SubghzTransmitter* subghz_transmitter;
    VariableItemList* variable_item_list;

    SubghzFrequencyAnalyzer* subghz_frequency_analyzer;
    SubghzReadRAW* subghz_read_raw;
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
    SubGhzViewReadRAW,
    SubGhzViewStatic,
    SubGhzViewTestCarrier,
    SubGhzViewTestPacket,
} SubGhzView;

bool subghz_set_pteset(SubGhz* subghz, const char* preset);
bool subghz_get_preset_name(SubGhz* subghz, string_t preset);
void subghz_get_frequency_modulation(SubGhz* subghz, string_t frequency, string_t modulation);
void subghz_begin(SubGhz* subghz, FuriHalSubGhzPreset preset);
uint32_t subghz_rx(SubGhz* subghz, uint32_t frequency);
void subghz_rx_end(SubGhz* subghz);
void subghz_sleep(SubGhz* subghz);
bool subghz_tx_start(SubGhz* subghz);
void subghz_tx_stop(SubGhz* subghz);
bool subghz_key_load(SubGhz* subghz, const char* file_path);
bool subghz_get_next_name_file(SubGhz* subghz);
bool subghz_save_protocol_to_file(SubGhz* subghz, const char* dev_name);
bool subghz_load_protocol_from_file(SubGhz* subghz);
bool subghz_rename_file(SubGhz* subghz);
bool subghz_delete_file(SubGhz* subghz);
void subghz_file_name_clear(SubGhz* subghz);
uint32_t subghz_random_serial(void);
void subghz_hopper_update(SubGhz* subghz);
