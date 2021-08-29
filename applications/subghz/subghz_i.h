#pragma once

#include "subghz.h"
#include "views/subghz_analyze.h"
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
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>
#include <gui/modules/text_input.h>

#include <subghz/scenes/subghz_scene.h>

#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/protocols/subghz_protocol.h>
#include <lib/subghz/protocols/subghz_protocol_common.h>
#include "subghz_history.h"

#define SUBGHZ_TEXT_STORE_SIZE 128

#define NOTIFICATION_STARTING_STATE 0u
#define NOTIFICATION_IDLE_STATE 1u
#define NOTIFICATION_TX_STATE 2u
#define NOTIFICATION_RX_STATE 3u

extern const uint32_t subghz_frequencies[];
extern const uint32_t subghz_frequencies_count;
extern const uint32_t subghz_frequencies_433_92;

struct SubGhz {
    Gui* gui;
    NotificationApp* notifications;

    SubGhzWorker* worker;
    SubGhzProtocol* protocol;
    SubGhzProtocolCommon* protocol_result;
    SubGhzProtocolCommonEncoder* encoder;
    uint32_t frequency;
    FuriHalSubGhzPreset preset;

    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    Submenu* submenu;
    DialogEx* dialog_ex;
    Popup* popup;
    TextInput* text_input;
    char text_store[SUBGHZ_TEXT_STORE_SIZE + 1];
    uint8_t state_notifications;

    SubghzAnalyze* subghz_analyze;
    SubghzReceiver* subghz_receiver;
    SubghzTransmitter* subghz_transmitter;

    SubghzTestStatic* subghz_test_static;
    SubghzTestCarrier* subghz_test_carrier;
    SubghzTestPacket* subghz_test_packet;
};

typedef enum {
    SubGhzViewMenu,

    SubGhzViewAnalyze,
    SubGhzViewDialogEx,
    SubGhzViewReceiver,
    SubGhzViewPopup,
    SubGhzViewTextInput,
    SubGhzViewTransmitter,

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
void subghz_transmitter_tx_start(void* context);
void subghz_transmitter_tx_stop(void* context);
bool subghz_key_load(SubGhz* subghz, const char* file_path);
bool subghz_save_protocol_to_file(void* context, const char* dev_name);
bool subghz_load_protocol_from_file(SubGhz* subghz);
uint32_t subghz_random_serial(void);
