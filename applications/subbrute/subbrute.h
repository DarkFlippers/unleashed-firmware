#pragma once
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/gui.h>
#include "m-string.h"

#include <toolbox/stream/stream.h>
#include <lib/subghz/transmitter.h>
#include <lib/subghz/receiver.h>
#include <flipper_format/flipper_format_i.h>
#include <dialogs/dialogs.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/text_input.h>
#include <gui/modules/popup.h>

#define TAG "SUBBRUTE"

typedef enum {
    NoneScene,
    SceneSelectFile,
    SceneSelectField,
    SceneAttack,
    SceneEntryPoint,
    SceneSaveName
} SubBruteScene;

typedef enum {
    SubBruteAttackLoadFile,
    SubBruteAttackCAME12bit433,
    SubBruteAttackCAME12bit868,
    SubBruteAttackChamberlain9bit315,
    SubBruteAttackChamberlain9bit390,
    SubBruteAttackLinear10bit300,
    SubBruteAttackLinear10bit310,
    SubBruteAttackNICE12bit433,
    SubBruteAttackNICE12bit868,
} SubBruteAttacks;

typedef enum {
    EventTypeTick,
    EventTypeKey,
    EventTypeCustom,
} EventType;

typedef struct {
    EventType evt_type;
    InputKey key;
    InputType input_type;
} SubBruteEvent;

// STRUCTS
typedef struct {
    // Application stuff
    bool is_running;
    bool is_attacking;
    SubBruteScene current_scene;
    SubBruteScene previous_scene;
    NotificationApp* notify;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    TextInput* text_input;
    Popup* popup;

    // SubGhz Stuff
    FlipperFormat* flipper_format;
    SubGhzEnvironment* environment;
    SubGhzTransmitter* transmitter;
    SubGhzReceiver* receiver;
    SubGhzProtocolDecoderBase* decoder_result;
    SubGhzPresetDefinition* preset_def;
    string_t preset;
    Stream* stream;
    string_t protocol;
    uint32_t frequency;
    uint32_t repeat;
    uint32_t bit;
    string_t key;
    uint32_t te;

    // Context Stuff
    DialogsApp* dialogs;
    char file_name_tmp[64];
    string_t file_path;
    string_t file_path_tmp;
    string_t notification_msg;
    uint8_t key_index;
    uint64_t payload;
    string_t candidate;
    uint8_t str_index;
    string_t flipper_format_string;

    SubBruteAttacks attack;

    //Menu stuff
    uint8_t menu_index;

    // RAW stuff
    string_t subbrute_raw_one;
    string_t subbrute_raw_zero;
    string_t subbrute_raw_stop;

} SubBruteState;