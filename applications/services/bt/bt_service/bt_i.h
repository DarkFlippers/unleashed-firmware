#pragma once

#include "bt.h"

#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <gui/view_port.h>
#include <gui/view.h>

#include <dialogs/dialogs.h>
#include <power/power_service/power.h>
#include <rpc/rpc.h>
#include <notification/notification.h>
#include <storage/storage.h>

#include <bt/bt_settings.h>
#include <bt/bt_service/bt_keys_storage.h>

#include "bt_keys_filename.h"

#define BT_KEYS_STORAGE_PATH INT_PATH(BT_KEYS_STORAGE_FILE_NAME)

#define BT_API_UNLOCK_EVENT (1UL << 0)

typedef enum {
    BtMessageTypeUpdateStatus,
    BtMessageTypeUpdateBatteryLevel,
    BtMessageTypeUpdatePowerState,
    BtMessageTypePinCodeShow,
    BtMessageTypeKeysStorageUpdated,
    BtMessageTypeSetProfile,
    BtMessageTypeDisconnect,
    BtMessageTypeForgetBondedDevices,
} BtMessageType;

typedef struct {
    uint8_t* start_address;
    uint16_t size;
} BtKeyStorageUpdateData;

typedef union {
    uint32_t pin_code;
    uint8_t battery_level;
    BtProfile profile;
    BtKeyStorageUpdateData key_storage_data;
} BtMessageData;

typedef struct {
    BtMessageType type;
    BtMessageData data;
    bool* result;
} BtMessage;

struct Bt {
    uint8_t* bt_keys_addr_start;
    uint16_t bt_keys_size;
    uint16_t max_packet_size;
    BtSettings bt_settings;
    BtKeysStorage* keys_storage;
    BtStatus status;
    BtProfile profile;
    FuriMessageQueue* message_queue;
    NotificationApp* notification;
    Gui* gui;
    ViewPort* statusbar_view_port;
    ViewPort* pin_code_view_port;
    uint32_t pin_code;
    DialogsApp* dialogs;
    DialogMessage* dialog_message;
    Power* power;
    Rpc* rpc;
    RpcSession* rpc_session;
    FuriEventFlag* rpc_event;
    FuriEventFlag* api_event;
    BtStatusChangedCallback status_changed_cb;
    void* status_changed_ctx;
};
