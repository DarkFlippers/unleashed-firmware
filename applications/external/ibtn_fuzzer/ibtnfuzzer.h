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

#include <ibtn_fuzzer_icons.h>

#include <lib/ibutton/ibutton_worker.h>
#include <lib/ibutton/ibutton_key.h>

#define TAG "iBtnFuzzer"

typedef enum {
    iBtnFuzzerAttackDefaultValues,
    iBtnFuzzerAttackLoadFile,
    iBtnFuzzerAttackLoadFileCustomUids,
} iBtnFuzzerAttacks;

typedef enum {
    DS1990,
    Metakom,
    Cyfral,
} iBtnFuzzerProtos;

typedef enum {
    NoneScene,
    SceneEntryPoint,
    SceneSelectFile,
    SceneSelectField,
    SceneAttack,
    SceneLoadCustomUids,
} iBtnFuzzerScene;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType evt_type;
    InputKey key;
    InputType input_type;
} iBtnFuzzerEvent;

// STRUCTS
typedef struct {
    FuriMutex* mutex;
    bool is_running;
    bool is_attacking;
    iBtnFuzzerScene current_scene;
    iBtnFuzzerScene previous_scene;
    NotificationApp* notify;
    u_int8_t menu_index;
    u_int8_t menu_proto_index;

    FuriString* data_str;
    uint8_t data[8];
    uint8_t payload[8];
    uint8_t attack_step;
    iBtnFuzzerAttacks attack;
    iBtnFuzzerProtos proto;
    FuriString* attack_name;
    FuriString* proto_name;
    FuriString* main_menu_items[3];
    FuriString* main_menu_proto_items[3];

    DialogsApp* dialogs;
    FuriString* notification_msg;
    uint8_t key_index;
    iButtonWorker* worker;
    iButtonKey* key;
    iButtonProtocolId keytype;
    iButtonProtocols* protocols;
    bool workr_rund;
    bool enter_rerun;
    bool attack_stop_called;

    uint8_t time_between_cards;

    // Used for custom dictionnary
    Stream* uids_stream;
} iBtnFuzzerState;