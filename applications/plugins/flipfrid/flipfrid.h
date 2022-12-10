#pragma once
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/gui.h>
#include <gui/modules/submenu.h>
#include <dialogs/dialogs.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <toolbox/stream/stream.h>
#include <flipper_format/flipper_format_i.h>

#include <toolbox/stream/stream.h>
#include <toolbox/stream/string_stream.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/stream/buffered_file_stream.h>

#include <RFID_Fuzzer_icons.h>

#include <lib/lfrfid/lfrfid_worker.h>
#include <lfrfid/protocols/lfrfid_protocols.h>

#define TAG "FlipFrid"

typedef enum {
    FlipFridAttackDefaultValues,
    FlipFridAttackBfCustomerId,
    FlipFridAttackLoadFile,
    FlipFridAttackLoadFileCustomUids,
} FlipFridAttacks;

typedef enum {
    EM4100,
    HIDProx,
    PAC,
    H10301,
} FlipFridProtos;

typedef enum {
    NoneScene,
    SceneEntryPoint,
    SceneSelectFile,
    SceneSelectField,
    SceneAttack,
    SceneLoadCustomUids,
} FlipFridScene;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType evt_type;
    InputKey key;
    InputType input_type;
} FlipFridEvent;

// STRUCTS
typedef struct {
    bool is_running;
    bool is_attacking;
    FlipFridScene current_scene;
    FlipFridScene previous_scene;
    NotificationApp* notify;
    u_int8_t menu_index;
    u_int8_t menu_proto_index;

    FuriString* data_str;
    uint8_t data[6];
    uint8_t payload[6];
    uint8_t attack_step;
    FlipFridAttacks attack;
    FlipFridProtos proto;
    FuriString* attack_name;
    FuriString* proto_name;

    DialogsApp* dialogs;
    FuriString* notification_msg;
    uint8_t key_index;
    LFRFIDWorker* worker;
    ProtocolDict* dict;
    ProtocolId protocol;
    bool workr_rund;
    bool attack_stop_called;

    uint8_t time_between_cards;

    // Used for custom dictionnary
    Stream* uids_stream;
} FlipFridState;