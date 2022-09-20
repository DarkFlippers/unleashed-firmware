#pragma once
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/gui.h>
#include <gui/modules/submenu.h>
#include <m-string.h>
#include <dialogs/dialogs.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <toolbox/stream/stream.h>
#include <flipper_format/flipper_format_i.h>

#include <toolbox/stream/stream.h>
#include <toolbox/stream/string_stream.h>
#include <toolbox/stream/file_stream.h>
#include <toolbox/stream/buffered_file_stream.h>

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

    string_t data_str;
    uint8_t data[6];
    uint8_t payload[6];
    uint8_t attack_step;
    FlipFridAttacks attack;
    FlipFridProtos proto;
    string_t attack_name;
    string_t proto_name;

    DialogsApp* dialogs;
    string_t notification_msg;
    uint8_t key_index;
    LFRFIDWorker* worker;
    ProtocolDict* dict;
    ProtocolId protocol;

    // Used for custom dictionnary
    Stream* uids_stream;
} FlipFridState;