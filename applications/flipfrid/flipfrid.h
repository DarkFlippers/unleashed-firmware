#pragma once
#include <furi.h>
#include <furi_hal.h>
#include <input/input.h>
#include <gui/gui.h>
#include <gui/modules/submenu.h>
#include "m-string.h"
#include <dialogs/dialogs.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <toolbox/stream/stream.h>
#include <flipper_format/flipper_format_i.h>

#include <lib/lfrfid/lfrfid_worker.h>
#include <lfrfid/protocols/lfrfid_protocols.h>

#define TAG "FlipFrid"

typedef enum {
    FlipFridAttackDefaultValues,
    FlipFridAttackBfCustomerId,
    FlipFridAttackLoadFile
} FlipFridAttacks;

typedef enum {
    NoneScene,
    SceneEntryPoint,
    SceneSelectFile,
    SceneSelectField,
    SceneAttack
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

    string_t data_str;
    uint8_t data[5];
    uint8_t payload[5];
    uint8_t attack_step;
    FlipFridAttacks attack;
    string_t attack_name;

    DialogsApp* dialogs;
    string_t file_path;
    string_t file_path_tmp;
    string_t notification_msg;
    uint8_t key_index;
    LFRFIDWorker* worker;
    ProtocolDict* dict;
} FlipFridState;