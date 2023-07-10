#pragma once
#include "mifare_nested.h"
#include "mifare_nested_worker.h"
#include "lib/nested/nested.h"
#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <notification/notification_messages.h>
#include <gui/modules/submenu.h>
#include <gui/modules/popup.h>
#include <gui/modules/loading.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <input/input.h>
#include "scenes/mifare_nested_scene.h"
#include <storage/storage.h>
#include <lib/toolbox/path.h>
#include <lib/nfc/nfc_device.h>
#include <lib/toolbox/value_index.h>
#include <gui/modules/variable_item_list.h>
#include "mifare_nested_icons.h"

#define NESTED_VERSION_APP "1.5.1"
#define NESTED_GITHUB_LINK "https://github.com/AloneLiberty/FlipperNested"
#define NESTED_RECOVER_KEYS_GITHUB_LINK "https://github.com/AloneLiberty/FlipperNestedRecovery"
#define NESTED_NONCE_FORMAT_VERSION "3"
#define NESTED_AUTHOR "@AloneLiberty (t.me/libertydev)"

enum MifareNestedCustomEvent {
    // Reserve first 100 events for button types and indexes, starting from 0
    MifareNestedCustomEventReserved = 100,

    MifareNestedCustomEventViewExit,
    MifareNestedCustomEventWorkerExit,
    MifareNestedCustomEventByteInputDone,
    MifareNestedCustomEventTextInputDone,
    MifareNestedCustomEventSceneSettingLock
};

typedef void (*NestedCallback)(void* context);

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* event_queue;
    ViewPort* view_port;
    View* view;
    NestedCallback callback;
    void* context;
} NestedState;

typedef void (*CheckKeysCallback)(void* context);

typedef struct {
    FuriMutex* mutex;
    FuriMessageQueue* event_queue;
    ViewPort* view_port;
    View* view;
    CheckKeysCallback callback;
    void* context;
} CheckKeysState;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    bool only_hardnested;
} MifareNestedSettings;

typedef enum { NestedRunIdle, NestedRunCheckKeys, NestedRunAttack } NestedRunNext;

struct MifareNested {
    MifareNestedWorker* worker;
    ViewDispatcher* view_dispatcher;
    Gui* gui;
    NotificationApp* notifications;
    SceneManager* scene_manager;
    NfcDevice* nfc_dev;
    VariableItemList* variable_item_list;
    MifareNestedSettings* settings;
    FuriString* text_box_store;

    // Common Views
    Submenu* submenu;
    Popup* popup;
    Loading* loading;
    TextInput* text_input;
    Widget* widget;

    NonceList_t* nonces;
    KeyInfo_t* keys;

    NestedState* nested_state;
    CheckKeysState* keys_state;
    SaveNoncesResult_t* save_state;

    MifareNestedWorkerState collecting_type;

    NestedRunNext run;
};

typedef enum {
    MifareNestedViewMenu,
    MifareNestedViewPopup,
    MifareNestedViewLoading,
    MifareNestedViewTextInput,
    MifareNestedViewWidget,
    MifareNestedViewVariableList,
    MifareNestedViewCollecting,
    MifareNestedViewCheckKeys,
} MifareNestedView;

typedef struct {
    FuriString* header;
    uint32_t keys_count;
    uint32_t nonces_collected;
    uint32_t hardnested_states;
    bool lost_tag;
    bool calibrating;
    bool need_prediction;
    bool hardnested;
} NestedAttackViewModel;

typedef struct {
    FuriString* header;
    uint32_t keys_count;
    uint32_t keys_checked;
    uint32_t keys_found;
    uint32_t keys_total;
    bool lost_tag;
    bool processing_keys;
} CheckKeysViewModel;

static const NotificationSequence mifare_nested_sequence_blink_start_blue = {
    &message_blink_start_10,
    &message_blink_set_color_blue,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence mifare_nested_sequence_blink_start_magenta = {
    &message_blink_start_10,
    &message_blink_set_color_magenta,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence mifare_nested_sequence_blink_start_yellow = {
    &message_blink_start_10,
    &message_blink_set_color_yellow,
    &message_do_not_reset,
    NULL,
};

static const NotificationSequence mifare_nested_sequence_blink_stop = {
    &message_blink_stop,
    NULL,
};

MifareNested* mifare_nested_alloc();

void mifare_nested_text_store_set(MifareNested* mifare_nested, const char* text, ...);

void mifare_nested_text_store_clear(MifareNested* mifare_nested);

void mifare_nested_blink_start(MifareNested* mifare_nested);

void mifare_nested_blink_calibration_start(MifareNested* mifare_nested);

void mifare_nested_blink_nonce_collection_start(MifareNested* mifare_nested);

void mifare_nested_blink_stop(MifareNested* mifare_nested);

void mifare_nested_show_loading_popup(void* context, bool show);
