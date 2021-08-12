#pragma once

#include "subghz.h"
#include "views/subghz_analyze.h"
#include "views/subghz_receiver.h"
#include "views/subghz_transmitter.h"
#include "views/subghz_static.h"

#include "views/subghz_test_carrier.h"
#include "views/subghz_test_packet.h"

#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <gui/modules/popup.h>
#include <gui/modules/text_input.h>

#include <subghz/scenes/subghz_scene.h>

#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/protocols/subghz_protocol.h>
#include <lib/subghz/protocols/subghz_protocol_common.h>

#define SUBGHZ_TEXT_STORE_SIZE 128

extern const uint32_t subghz_frequencies[];
extern const uint32_t subghz_frequencies_count;
extern const uint32_t subghz_frequencies_433_92;

struct SubGhz {
    Gui* gui;

    SubGhzWorker* worker;
    SubGhzProtocol* protocol;
    SubGhzProtocolCommon* protocol_result;

    SceneManager* scene_manager;

    ViewDispatcher* view_dispatcher;

    Submenu* submenu;
    DialogEx* dialog_ex;
    Popup* popup;
    TextInput* text_input;
    char text_store[SUBGHZ_TEXT_STORE_SIZE + 1];

    SubghzAnalyze* subghz_analyze;
    SubghzReceiver* subghz_receiver;
    SubghzTransmitter* subghz_transmitter;
    SubghzStatic* subghz_static;

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
void subghz_rx(uint32_t frequency);
void subghz_tx(uint32_t frequency);
void subghz_idle(void);
void subghz_end(void);
bool subghz_key_load(SubGhz* subghz, const char* file_path);