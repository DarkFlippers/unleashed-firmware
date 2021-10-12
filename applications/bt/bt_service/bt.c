#include "bt_i.h"
#include "battery_service.h"

#define BT_SERVICE_TAG "BT"

static void bt_draw_statusbar_callback(Canvas* canvas, void* context) {
    canvas_draw_icon(canvas, 0, 0, &I_Bluetooth_5x8);
}

static ViewPort* bt_statusbar_view_port_alloc() {
    ViewPort* statusbar_view_port = view_port_alloc();
    view_port_set_width(statusbar_view_port, 5);
    view_port_draw_callback_set(statusbar_view_port, bt_draw_statusbar_callback, NULL);
    view_port_enabled_set(statusbar_view_port, false);
    return statusbar_view_port;
}

static void bt_pin_code_show_event_handler(Bt* bt, uint32_t pin) {
    furi_assert(bt);
    string_t pin_str;
    string_init_printf(pin_str, "%06d", pin);
    dialog_message_set_text(
        bt->dialog_message, string_get_cstr(pin_str), 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(bt->dialog_message, "Back", NULL, NULL);
    dialog_message_show(bt->dialogs, bt->dialog_message);
    string_clear(pin_str);
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
    // Load settings
    if(!bt_settings_load(&bt->bt_settings)) {
        bt_settings_save(&bt->bt_settings);
    }
    // Alloc queue
    bt->message_queue = osMessageQueueNew(8, sizeof(BtMessage), NULL);

    // Setup statusbar view port
    bt->statusbar_view_port = bt_statusbar_view_port_alloc();
    // Gui
    bt->gui = furi_record_open("gui");
    gui_add_view_port(bt->gui, bt->statusbar_view_port, GuiLayerStatusBarLeft);

    // Dialogs
    bt->dialogs = furi_record_open("dialogs");
    bt->dialog_message = dialog_message_alloc();

    // Power
    bt->power = furi_record_open("power");
    PubSub* power_pubsub = power_get_pubsub(bt->power);
    subscribe_pubsub(power_pubsub, bt_battery_level_changed_callback, bt);

    // RPC
    bt->rpc = furi_record_open("rpc");
    bt->rpc_sem = osSemaphoreNew(1, 0, NULL);

    return bt;
}

// Called from GAP thread from Serial service
static void bt_on_data_received_callback(uint8_t* data, uint16_t size, void* context) {
    furi_assert(context);
    Bt* bt = context;

    size_t bytes_processed = rpc_feed_bytes(bt->rpc_session, data, size, 1000);
    if(bytes_processed != size) {
        FURI_LOG_E(BT_SERVICE_TAG, "Only %d of %d bytes processed by RPC", bytes_processed, size);
    }
}

// Called from GAP thread from Serial service
static void bt_on_data_sent_callback(void* context) {
    furi_assert(context);
    Bt* bt = context;

    osSemaphoreRelease(bt->rpc_sem);
}

// Called from RPC thread
static void bt_rpc_send_bytes_callback(void* context, uint8_t* bytes, size_t bytes_len) {
    furi_assert(context);
    Bt* bt = context;

    size_t bytes_sent = 0;
    while(bytes_sent < bytes_len) {
        size_t bytes_remain = bytes_len - bytes_sent;
        if(bytes_remain > FURI_HAL_BT_PACKET_SIZE_MAX) {
            furi_hal_bt_tx(&bytes[bytes_sent], FURI_HAL_BT_PACKET_SIZE_MAX);
            bytes_sent += FURI_HAL_BT_PACKET_SIZE_MAX;
        } else {
            furi_hal_bt_tx(&bytes[bytes_sent], bytes_remain);
            bytes_sent += bytes_remain;
        }
        osSemaphoreAcquire(bt->rpc_sem, osWaitForever);
    }
}

// Called from GAP thread
static void bt_on_gap_event_callback(BleEvent event, void* context) {
    furi_assert(context);
    Bt* bt = context;

    if(event.type == BleEventTypeConnected) {
        FURI_LOG_I(BT_SERVICE_TAG, "Open RPC connection");
        bt->rpc_session = rpc_open_session(bt->rpc);
        rpc_set_send_bytes_callback(bt->rpc_session, bt_rpc_send_bytes_callback, bt);
        furi_hal_bt_set_data_event_callbacks(
            bt_on_data_received_callback, bt_on_data_sent_callback, bt);
        // Update battery level
        PowerInfo info;
        power_get_info(bt->power, &info);
        BtMessage message = {
            .type = BtMessageTypeUpdateBatteryLevel, .data.battery_level = info.charge};
        furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
    } else if(event.type == BleEventTypeDisconnected) {
        FURI_LOG_I(BT_SERVICE_TAG, "Close RPC connection");
        if(bt->rpc_session) {
            rpc_close_session(bt->rpc_session);
            bt->rpc_session = NULL;
        }
    } else if(event.type == BleEventTypeStartAdvertising || event.type == BleEventTypeStopAdvertising) {
        BtMessage message = {.type = BtMessageTypeUpdateStatusbar};
        furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
    } else if(event.type == BleEventTypePinCodeShow) {
        BtMessage message = {
            .type = BtMessageTypePinCodeShow, .data.pin_code = event.data.pin_code};
        furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
    }
}

int32_t bt_srv() {
    Bt* bt = bt_alloc();
    furi_record_create("bt", bt);

    if(!furi_hal_bt_wait_startup()) {
        FURI_LOG_E(BT_SERVICE_TAG, "Core2 startup failed");
    } else {
        view_port_enabled_set(bt->statusbar_view_port, true);
        if(furi_hal_bt_init_app(bt_on_gap_event_callback, bt)) {
            FURI_LOG_I(BT_SERVICE_TAG, "BLE stack started");
            if(bt->bt_settings.enabled) {
                furi_hal_bt_start_advertising();
            }
        } else {
            FURI_LOG_E(BT_SERVICE_TAG, "BT App start failed");
        }
    }
    // Update statusbar
    view_port_enabled_set(bt->statusbar_view_port, furi_hal_bt_is_active());

    BtMessage message;
    while(1) {
        furi_check(osMessageQueueGet(bt->message_queue, &message, NULL, osWaitForever) == osOK);
        if(message.type == BtMessageTypeUpdateStatusbar) {
            // Update statusbar
            view_port_enabled_set(bt->statusbar_view_port, furi_hal_bt_is_active());
        } else if(message.type == BtMessageTypeUpdateBatteryLevel) {
            // Update battery level
            if(furi_hal_bt_is_active()) {
                battery_svc_update_level(message.data.battery_level);
            }
        } else if(message.type == BtMessageTypePinCodeShow) {
            // Display PIN code
            bt_pin_code_show_event_handler(bt, message.data.pin_code);
        }
    }
    return 0;
}
