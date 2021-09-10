#pragma once

#include "subghz.h"
#include "views/subghz_receiver.h"
#include "views/subghz_transmitter.h"

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
#include <lib/subghz/protocols/subghz_protocol.h>
#include <lib/subghz/protocols/subghz_protocol_common.h>
#include "subghz_history.h"

#include <gui/modules/variable-item-list.h>

#define SUBGHZ_TEXT_STORE_SIZE 128

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
    SubGhzProtocol* protocol;
    SubGhzProtocolCommon* protocol_result;
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
    char text_store[SUBGHZ_TEXT_STORE_SIZE + 1];
    uint8_t state_notifications;

    SubghzReceiver* subghz_receiver;
    SubghzTransmitter* subghz_transmitter;
    VariableItemList* variable_item_list;

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
    SubGhzViewStatic,
    SubGhzViewTestCarrier,
    SubGhzViewTestPacket,
} SubGhzView;

void subghz_begin(FuriHalSubGhzPreset preset);
uint32_t subghz_rx(void* context, uint32_t frequency);
uint32_t subghz_tx(uint32_t frequency);
void subghz_idle(void);
void subghz_rx_end(void* context);
void subghz_sleep(void);
void subghz_tx_start(void* context);
void subghz_tx_stop(void* context);
bool subghz_key_load(SubGhz* subghz, const char* file_path);
bool subghz_save_protocol_to_file(void* context, const char* dev_name);
bool subghz_load_protocol_from_file(SubGhz* subghz);
uint32_t subghz_random_serial(void);
