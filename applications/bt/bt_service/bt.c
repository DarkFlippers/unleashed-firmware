#include "bt_i.h"
#include "battery_service.h"
#include "bt_keys_storage.h"

#define TAG "BtSrv"

#define BT_RPC_EVENT_BUFF_SENT (1UL << 0)
#define BT_RPC_EVENT_DISCONNECTED (1UL << 1)
#define BT_RPC_EVENT_ALL (BT_RPC_EVENT_BUFF_SENT | BT_RPC_EVENT_DISCONNECTED)

static void bt_draw_statusbar_callback(Canvas* canvas, void* context) {
    furi_assert(context);

    Bt* bt = context;
    if(bt->status == BtStatusAdvertising) {
        canvas_draw_icon(canvas, 0, 0, &I_Bluetooth_5x8);
    } else if(bt->status == BtStatusConnected) {
        canvas_draw_icon(canvas, 0, 0, &I_BT_Pair_9x8);
    }
}

static ViewPort* bt_statusbar_view_port_alloc(Bt* bt) {
    ViewPort* statusbar_view_port = view_port_alloc();
    view_port_set_width(statusbar_view_port, 5);
    view_port_draw_callback_set(statusbar_view_port, bt_draw_statusbar_callback, bt);
    view_port_enabled_set(statusbar_view_port, false);
    return statusbar_view_port;
}

static void bt_pin_code_show_event_handler(Bt* bt, uint32_t pin) {
    furi_assert(bt);
    string_t pin_str;
    dialog_message_set_icon(bt->dialog_message, &I_BLE_Pairing_128x64, 0, 0);
    string_init_printf(pin_str, "Pairing code\n%06d", pin);
    dialog_message_set_text(
        bt->dialog_message, string_get_cstr(pin_str), 64, 4, AlignCenter, AlignTop);
    dialog_message_set_buttons(bt->dialog_message, "Quit", NULL, NULL);
    dialog_message_show(bt->dialogs, bt->dialog_message);
    string_clear(pin_str);
}

static bool bt_pin_code_verify_event_handler(Bt* bt, uint32_t pin) {
    furi_assert(bt);
    string_t pin_str;
    dialog_message_set_icon(bt->dialog_message, &I_BLE_Pairing_128x64, 0, 0);
    string_init_printf(pin_str, "Verify code\n%06d", pin);
    dialog_message_set_text(
        bt->dialog_message, string_get_cstr(pin_str), 64, 4, AlignCenter, AlignTop);
    dialog_message_set_buttons(bt->dialog_message, "Cancel", "Ok", NULL);
    DialogMessageButton button = dialog_message_show(bt->dialogs, bt->dialog_message);
    string_clear(pin_str);
    return button == DialogMessageButtonCenter;
}

static void bt_battery_level_changed_callback(const void* _event, void* context) {
    furi_assert(_event);
    furi_assert(context);

    Bt* bt = context;
    const PowerEvent* event = _event;
    if(event->type == PowerEventTypeBatteryLevelChanged) {
        BtMessage message = {
            .type = BtMessageTypeUpdateBatteryLevel,
            .data.battery_level = event->data.battery_level};
        furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
    }
}

Bt* bt_alloc() {
    Bt* bt = furi_alloc(sizeof(Bt));
    // Init default maximum packet size
    bt->max_packet_size = FURI_HAL_BT_SERIAL_PACKET_SIZE_MAX;
    bt->profile = BtProfileSerial;
    // Load settings
    if(!bt_settings_load(&bt->bt_settings)) {
        bt_settings_save(&bt->bt_settings);
    }
    // Alloc queue
    bt->message_queue = osMessageQueueNew(8, sizeof(BtMessage), NULL);

    // Setup statusbar view port
    bt->statusbar_view_port = bt_statusbar_view_port_alloc(bt);
    // Gui
    bt->gui = furi_record_open("gui");
    gui_add_view_port(bt->gui, bt->statusbar_view_port, GuiLayerStatusBarLeft);

    // Dialogs
    bt->dialogs = furi_record_open("dialogs");
    bt->dialog_message = dialog_message_alloc();

    // Power
    bt->power = furi_record_open("power");
    FuriPubSub* power_pubsub = power_get_pubsub(bt->power);
    furi_pubsub_subscribe(power_pubsub, bt_battery_level_changed_callback, bt);

    // RPC
    bt->rpc = furi_record_open("rpc");
    bt->rpc_event = osEventFlagsNew(NULL);

    // API evnent
    bt->api_event = osEventFlagsNew(NULL);

    return bt;
}

// Called from GAP thread from Serial service
static uint16_t bt_serial_event_callback(SerialServiceEvent event, void* context) {
    furi_assert(context);
    Bt* bt = context;
    uint16_t ret = 0;

    if(event.event == SerialServiceEventTypeDataReceived) {
        size_t bytes_processed =
            rpc_session_feed(bt->rpc_session, event.data.buffer, event.data.size, 1000);
        if(bytes_processed != event.data.size) {
            FURI_LOG_E(
                TAG, "Only %d of %d bytes processed by RPC", bytes_processed, event.data.size);
        }
        ret = rpc_session_get_available_size(bt->rpc_session);
    } else if(event.event == SerialServiceEventTypeDataSent) {
        osEventFlagsSet(bt->rpc_event, BT_RPC_EVENT_BUFF_SENT);
    }
    return ret;
}

// Called from RPC thread
static void bt_rpc_send_bytes_callback(void* context, uint8_t* bytes, size_t bytes_len) {
    furi_assert(context);
    Bt* bt = context;

    osEventFlagsClear(bt->rpc_event, BT_RPC_EVENT_ALL);
    size_t bytes_sent = 0;
    while(bytes_sent < bytes_len) {
        size_t bytes_remain = bytes_len - bytes_sent;
        if(bytes_remain > bt->max_packet_size) {
            furi_hal_bt_serial_tx(&bytes[bytes_sent], bt->max_packet_size);
            bytes_sent += bt->max_packet_size;
        } else {
            furi_hal_bt_serial_tx(&bytes[bytes_sent], bytes_remain);
            bytes_sent += bytes_remain;
        }
        uint32_t event_flag =
            osEventFlagsWait(bt->rpc_event, BT_RPC_EVENT_ALL, osFlagsWaitAny, osWaitForever);
        if(event_flag & BT_RPC_EVENT_DISCONNECTED) {
            break;
        }
    }
}

// Called from GAP thread
static bool bt_on_gap_event_callback(BleEvent event, void* context) {
    furi_assert(context);
    Bt* bt = context;
    bool ret = false;

    if(event.type == BleEventTypeConnected) {
        // Update status bar
        bt->status = BtStatusConnected;
        BtMessage message = {.type = BtMessageTypeUpdateStatusbar};
        furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
        if(bt->profile == BtProfileSerial) {
            // Open RPC session
            FURI_LOG_I(TAG, "Open RPC connection");
            bt->rpc_session = rpc_session_open(bt->rpc);
            rpc_session_set_send_bytes_callback(bt->rpc_session, bt_rpc_send_bytes_callback);
            rpc_session_set_buffer_is_empty_callback(
                bt->rpc_session, furi_hal_bt_serial_notify_buffer_is_empty);
            rpc_session_set_context(bt->rpc_session, bt);
            furi_hal_bt_serial_set_event_callback(RPC_BUFFER_SIZE, bt_serial_event_callback, bt);
        }
        // Update battery level
        PowerInfo info;
        power_get_info(bt->power, &info);
        message.type = BtMessageTypeUpdateBatteryLevel;
        message.data.battery_level = info.charge;
        furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
        ret = true;
    } else if(event.type == BleEventTypeDisconnected) {
        if(bt->profile == BtProfileSerial && bt->rpc_session) {
            FURI_LOG_I(TAG, "Close RPC connection");
            osEventFlagsSet(bt->rpc_event, BT_RPC_EVENT_DISCONNECTED);
            rpc_session_close(bt->rpc_session);
            furi_hal_bt_serial_set_event_callback(0, NULL, NULL);
            bt->rpc_session = NULL;
        }
        ret = true;
    } else if(event.type == BleEventTypeStartAdvertising) {
        bt->status = BtStatusAdvertising;
        BtMessage message = {.type = BtMessageTypeUpdateStatusbar};
        furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
        ret = true;
    } else if(event.type == BleEventTypeStopAdvertising) {
        bt->status = BtStatusOff;
        BtMessage message = {.type = BtMessageTypeUpdateStatusbar};
        furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
        ret = true;
    } else if(event.type == BleEventTypePinCodeShow) {
        BtMessage message = {
            .type = BtMessageTypePinCodeShow, .data.pin_code = event.data.pin_code};
        furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
        ret = true;
    } else if(event.type == BleEventTypePinCodeVerify) {
        ret = bt_pin_code_verify_event_handler(bt, event.data.pin_code);
    } else if(event.type == BleEventTypeUpdateMTU) {
        bt->max_packet_size = event.data.max_packet_size;
        ret = true;
    }
    return ret;
}

static void bt_on_key_storage_change_callback(uint8_t* addr, uint16_t size, void* context) {
    furi_assert(context);
    Bt* bt = context;
    FURI_LOG_I(TAG, "Changed addr start: %08lX, size changed: %d", addr, size);
    BtMessage message = {.type = BtMessageTypeKeysStorageUpdated};
    furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
}

static void bt_statusbar_update(Bt* bt) {
    if(bt->status == BtStatusAdvertising) {
        view_port_set_width(bt->statusbar_view_port, icon_get_width(&I_Bluetooth_5x8));
        view_port_enabled_set(bt->statusbar_view_port, true);
    } else if(bt->status == BtStatusConnected) {
        view_port_set_width(bt->statusbar_view_port, icon_get_width(&I_BT_Pair_9x8));
        view_port_enabled_set(bt->statusbar_view_port, true);
    } else {
        view_port_enabled_set(bt->statusbar_view_port, false);
    }
}

static void bt_change_profile(Bt* bt, BtMessage* message) {
    if(bt->profile == BtProfileSerial && bt->rpc_session) {
        FURI_LOG_I(TAG, "Close RPC connection");
        osEventFlagsSet(bt->rpc_event, BT_RPC_EVENT_DISCONNECTED);
        rpc_session_close(bt->rpc_session);
        furi_hal_bt_serial_set_event_callback(0, NULL, NULL);
        bt->rpc_session = NULL;
    }

    FuriHalBtProfile furi_profile;
    if(message->data.profile == BtProfileHidKeyboard) {
        furi_profile = FuriHalBtProfileHidKeyboard;
    } else {
        furi_profile = FuriHalBtProfileSerial;
    }

    if(furi_hal_bt_change_app(furi_profile, bt_on_gap_event_callback, bt)) {
        FURI_LOG_I(TAG, "Bt App started");
        if(bt->bt_settings.enabled) {
            furi_hal_bt_start_advertising();
        }
        furi_hal_bt_set_key_storage_change_callback(bt_on_key_storage_change_callback, bt);
        bt->profile = message->data.profile;
        *message->result = true;
    } else {
        FURI_LOG_E(TAG, "Failed to start Bt App");
        *message->result = false;
    }
    osEventFlagsSet(bt->api_event, BT_API_UNLOCK_EVENT);
}

int32_t bt_srv() {
    Bt* bt = bt_alloc();
    furi_record_create("bt", bt);

    // Read keys
    if(!bt_load_key_storage(bt)) {
        FURI_LOG_W(TAG, "Failed to load bonding keys");
    }

    // Start BLE stack
    if(furi_hal_bt_start_app(FuriHalBtProfileSerial, bt_on_gap_event_callback, bt)) {
        FURI_LOG_I(TAG, "BLE stack started");
        if(bt->bt_settings.enabled) {
            furi_hal_bt_start_advertising();
        }
        furi_hal_bt_set_key_storage_change_callback(bt_on_key_storage_change_callback, bt);
    } else {
        FURI_LOG_E(TAG, "BT App start failed");
    }

    // Update statusbar
    bt_statusbar_update(bt);

    BtMessage message;
    while(1) {
        furi_check(osMessageQueueGet(bt->message_queue, &message, NULL, osWaitForever) == osOK);
        if(message.type == BtMessageTypeUpdateStatusbar) {
            // Update statusbar
            bt_statusbar_update(bt);
        } else if(message.type == BtMessageTypeUpdateBatteryLevel) {
            // Update battery level
            furi_hal_bt_update_battery_level(message.data.battery_level);
        } else if(message.type == BtMessageTypePinCodeShow) {
            // Display PIN code
            bt_pin_code_show_event_handler(bt, message.data.pin_code);
        } else if(message.type == BtMessageTypeKeysStorageUpdated) {
            bt_save_key_storage(bt);
        } else if(message.type == BtMessageTypeSetProfile) {
            bt_change_profile(bt, &message);
        }
    }
    return 0;
}
