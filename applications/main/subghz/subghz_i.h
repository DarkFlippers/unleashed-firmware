#pragma once

#include "helpers/subghz_types.h"
#include "helpers/subghz_error_type.h"
#include <lib/subghz/types.h>
#include "subghz.h"
#include "views/receiver.h"
#include "views/transmitter.h"
#include "views/subghz_frequency_analyzer.h"
#include "views/subghz_read_raw.h"

#include "views/subghz_test_static.h"
#include "views/subghz_test_carrier.h"
#include "views/subghz_test_packet.h"

#include <gui/gui.h>
#include <assets_icons.h>
#include <dialogs/dialogs.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>

#include <subghz/scenes/subghz_scene.h>
#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/subghz_setting.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>

#include "subghz_history.h"

#include <gui/modules/variable_item_list.h>
#include <lib/toolbox/path.h>

#include "rpc/rpc_app.h"

#define SUBGHZ_MAX_LEN_NAME 64

struct SubGhzTxRx {
    SubGhzWorker* worker;

    SubGhzEnvironment* environment;
    SubGhzReceiver* receiver;
    SubGhzTransmitter* transmitter;
    SubGhzProtocolFlag filter;
    SubGhzProtocolDecoderBase* decoder_result;
    FlipperFormat* fff_data;

    SubGhzRadioPreset* preset;
    SubGhzHistory* history;
    uint16_t idx_menu_chosen;
    SubGhzTxRxState txrx_state;
    SubGhzHopperState hopper_state;
    SubGhzSpeakerState speaker_state;
    uint8_t hopper_timeout;
    uint8_t hopper_idx_frequency;
    SubGhzRxKeyState rx_key_state;

    float raw_threshold_rssi;
    uint8_t raw_threshold_rssi_low_count;
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
    FuriString* file_path;
    FuriString* file_path_tmp;
    char file_name_tmp[SUBGHZ_MAX_LEN_NAME];
    SubGhzNotificationState state_notifications;

    SubGhzViewReceiver* subghz_receiver;
    SubGhzViewTransmitter* subghz_transmitter;
    VariableItemList* variable_item_list;

    SubGhzFrequencyAnalyzer* subghz_frequency_analyzer;
    SubGhzReadRAW* subghz_read_raw;
    SubGhzTestStatic* subghz_test_static;
    SubGhzTestCarrier* subghz_test_carrier;
    SubGhzTestPacket* subghz_test_packet;
    FuriString* error_str;
    SubGhzSetting* setting;
    SubGhzLock lock;

    void* rpc_ctx;
};

void subghz_preset_init(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint8_t* preset_data,
    size_t preset_data_size);
bool subghz_set_preset(SubGhz* subghz, const char* preset);
void subghz_get_frequency_modulation(SubGhz* subghz, FuriString* frequency, FuriString* modulation);
void subghz_begin(SubGhz* subghz, uint8_t* preset_data);
uint32_t subghz_rx(SubGhz* subghz, uint32_t frequency);
void subghz_rx_end(SubGhz* subghz);
void subghz_sleep(SubGhz* subghz);

void subghz_blink_start(SubGhz* instance);
void subghz_blink_stop(SubGhz* instance);

bool subghz_tx_start(SubGhz* subghz, FlipperFormat* flipper_format);
void subghz_tx_stop(SubGhz* subghz);
void subghz_dialog_message_show_only_rx(SubGhz* subghz);
bool subghz_key_load(SubGhz* subghz, const char* file_path, bool show_dialog);
bool subghz_get_next_name_file(SubGhz* subghz, uint8_t max_len);
bool subghz_save_protocol_to_file(
    SubGhz* subghz,
    FlipperFormat* flipper_format,
    const char* dev_file_name);
bool subghz_load_protocol_from_file(SubGhz* subghz);
bool subghz_rename_file(SubGhz* subghz);
bool subghz_file_available(SubGhz* subghz);
bool subghz_delete_file(SubGhz* subghz);
void subghz_file_name_clear(SubGhz* subghz);
bool subghz_path_is_file(FuriString* path);
uint32_t subghz_random_serial(void);
void subghz_hopper_update(SubGhz* subghz);
void subghz_speaker_on(SubGhz* subghz);
void subghz_speaker_off(SubGhz* subghz);
void subghz_speaker_mute(SubGhz* subghz);
void subghz_speaker_unmute(SubGhz* subghz);
