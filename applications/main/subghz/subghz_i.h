#pragma once

#include "helpers/subghz_types.h"
#include "helpers/subghz_error_type.h"
#include <lib/subghz/types.h>
#include "subghz.h"
#include "views/receiver.h"
#include "views/transmitter.h"
#include "views/subghz_frequency_analyzer.h"
#include "views/subghz_read_raw.h"

#include <gui/gui.h>
#include <assets_icons.h>
#include <dialogs/dialogs.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/text_input.h>
#include <gui/modules/byte_input.h>
#include <gui/modules/widget.h>

#include <subghz/scenes/subghz_scene.h>
#include <lib/subghz/subghz_worker.h>
#include <lib/subghz/subghz_file_encoder_worker.h>
#include <lib/subghz/subghz_setting.h>
#include <lib/subghz/receiver.h>
#include <lib/subghz/transmitter.h>

#include "subghz_history.h"
#include "subghz_last_settings.h"

#include <gui/modules/variable_item_list.h>
#include <lib/toolbox/path.h>

#include "rpc/rpc_app.h"

#include "helpers/subghz_threshold_rssi.h"

#include "helpers/subghz_txrx.h"

#define SUBGHZ_MAX_LEN_NAME 64
#define SUBGHZ_EXT_PRESET_NAME true
#define SUBGHZ_RAW_THRESHOLD_MIN (-90.0f)
#define SUBGHZ_MEASURE_LOADING false

typedef struct {
    uint8_t fix[4];
    uint8_t cnt[4];
    uint8_t seed[4];
} SecureData;

struct SubGhz {
    Gui* gui;
    NotificationApp* notifications;

    SubGhzTxRx* txrx;

    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;

    Submenu* submenu;
    Popup* popup;
    TextInput* text_input;
    ByteInput* byte_input;
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
    bool raw_send_only;

    bool save_datetime_set;
    FuriHalRtcDateTime save_datetime;

    SubGhzLastSettings* last_settings;

    SubGhzProtocolFlag filter;
    SubGhzProtocolFlag ignore_filter;
    FuriString* error_str;
    SubGhzLock lock;

    SecureData* secure_data;

    SubGhzFileEncoderWorker* decode_raw_file_worker_encoder;

    SubGhzThresholdRssi* threshold_rssi;
    SubGhzRxKeyState rx_key_state;
    SubGhzHistory* history;

    uint16_t idx_menu_chosen;
    SubGhzLoadTypeFile load_type_file;

    void* rpc_ctx;
};

void subghz_blink_start(SubGhz* subghz);
void subghz_blink_stop(SubGhz* subghz);

bool subghz_tx_start(SubGhz* subghz, FlipperFormat* flipper_format);
void subghz_dialog_message_freq_error(SubGhz* subghz, bool only_rx);

bool subghz_key_load(SubGhz* subghz, const char* file_path, bool show_dialog);
bool subghz_get_next_name_file(SubGhz* subghz, uint8_t max_len);
bool subghz_save_protocol_to_file(
    SubGhz* subghz,
    FlipperFormat* flipper_format,
    const char* dev_file_name);
void subghz_save_to_file(void* context);
bool subghz_load_protocol_from_file(SubGhz* subghz);
bool subghz_rename_file(SubGhz* subghz);
bool subghz_file_available(SubGhz* subghz);
bool subghz_delete_file(SubGhz* subghz);
void subghz_file_name_clear(SubGhz* subghz);
bool subghz_path_is_file(FuriString* path);
SubGhzLoadTypeFile subghz_get_load_type_file(SubGhz* subghz);

void subghz_lock(SubGhz* subghz);
void subghz_unlock(SubGhz* subghz);
bool subghz_is_locked(SubGhz* subghz);

void subghz_rx_key_state_set(SubGhz* subghz, SubGhzRxKeyState state);
SubGhzRxKeyState subghz_rx_key_state_get(SubGhz* subghz);

extern const NotificationSequence subghz_sequence_rx;
extern const NotificationSequence subghz_sequence_rx_locked;
