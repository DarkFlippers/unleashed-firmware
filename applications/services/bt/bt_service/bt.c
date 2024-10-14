#include "bt_i.h"
#include "bt_keys_storage.h"

#include <core/check.h>
#include <furi_hal_bt.h>
#include <services/battery_service.h>
#include <notification/notification_messages.h>
#include <gui/elements.h>
#include <assets_icons.h>
#include <profiles/serial_profile.h>

#define TAG "BtSrv"

#define BT_RPC_EVENT_BUFF_SENT    (1UL << 0)
#define BT_RPC_EVENT_DISCONNECTED (1UL << 1)
#define BT_RPC_EVENT_ALL          (BT_RPC_EVENT_BUFF_SENT | BT_RPC_EVENT_DISCONNECTED)

#define ICON_SPACER 2

static void bt_draw_statusbar_callback(Canvas* canvas, void* context) {
    furi_assert(context);

    Bt* bt = context;
    uint8_t draw_offset = 0;
    if(bt->beacon_active) {
        canvas_draw_icon(canvas, 0, 0, &I_BLE_beacon_7x8);
        draw_offset += icon_get_width(&I_BLE_beacon_7x8) + ICON_SPACER;
    }
    if(bt->status == BtStatusAdvertising) {
        canvas_draw_icon(canvas, draw_offset, 0, &I_Bluetooth_Idle_5x8);
    } else if(bt->status == BtStatusConnected) {
        canvas_draw_icon(canvas, draw_offset, 0, &I_Bluetooth_Connected_16x8);
    }
}

static ViewPort* bt_statusbar_view_port_alloc(Bt* bt) {
    ViewPort* statusbar_view_port = view_port_alloc();
    view_port_set_width(statusbar_view_port, 5);
    view_port_draw_callback_set(statusbar_view_port, bt_draw_statusbar_callback, bt);
    view_port_enabled_set(statusbar_view_port, false);
    return statusbar_view_port;
}

static void bt_pin_code_view_port_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    Bt* bt = context;
    char pin_code_info[24];
    canvas_draw_icon(canvas, 0, 0, &I_BLE_Pairing_128x64);
    snprintf(pin_code_info, sizeof(pin_code_info), "Pairing code\n%06lu", bt->pin_code);
    elements_multiline_text_aligned(canvas, 64, 4, AlignCenter, AlignTop, pin_code_info);
    elements_button_left(canvas, "Quit");
}

static void bt_pin_code_view_port_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    Bt* bt = context;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyLeft || event->key == InputKeyBack) {
            view_port_enabled_set(bt->pin_code_view_port, false);
        }
    }
}

static void bt_storage_callback(const void* message, void* context) {
    furi_assert(context);
    Bt* bt = context;
    const StorageEvent* event = message;

    if(event->type == StorageEventTypeCardMount) {
        const BtMessage msg = {
            .type = BtMessageTypeReloadKeysSettings,
        };

        furi_check(
            furi_message_queue_put(bt->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    }
}

static ViewPort* bt_pin_code_view_port_alloc(Bt* bt) {
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, bt_pin_code_view_port_draw_callback, bt);
    view_port_input_callback_set(view_port, bt_pin_code_view_port_input_callback, bt);
    view_port_enabled_set(view_port, false);
    return view_port;
}

static void bt_pin_code_show(Bt* bt, uint32_t pin_code) {
    bt->pin_code = pin_code;
    if(!bt->pin_code_view_port) {
        // Pin code view port
        bt->pin_code_view_port = bt_pin_code_view_port_alloc(bt);
        gui_add_view_port(bt->gui, bt->pin_code_view_port, GuiLayerFullscreen);
    }
    notification_message(bt->notification, &sequence_display_backlight_on);
    gui_view_port_send_to_front(bt->gui, bt->pin_code_view_port);
    view_port_enabled_set(bt->pin_code_view_port, true);
}

static void bt_pin_code_hide(Bt* bt) {
    bt->pin_code = 0;
    if(bt->pin_code_view_port && view_port_is_enabled(bt->pin_code_view_port)) {
        view_port_enabled_set(bt->pin_code_view_port, false);
    }
}

static bool bt_pin_code_verify_event_handler(Bt* bt, uint32_t pin) {
    furi_assert(bt);
    notification_message(bt->notification, &sequence_display_backlight_on);
    FuriString* pin_str;
    if(!bt->dialog_message) {
        bt->dialog_message = dialog_message_alloc();
    }
    dialog_message_set_icon(bt->dialog_message, &I_BLE_Pairing_128x64, 0, 0);
    pin_str = furi_string_alloc_printf("Verify code\n%06lu", pin);
    dialog_message_set_text(
        bt->dialog_message, furi_string_get_cstr(pin_str), 64, 4, AlignCenter, AlignTop);
    dialog_message_set_buttons(bt->dialog_message, "Cancel", "OK", NULL);
    DialogMessageButton button = dialog_message_show(bt->dialogs, bt->dialog_message);
    furi_string_free(pin_str);
    return button == DialogMessageButtonCenter;
}

static void bt_battery_level_changed_callback(const void* _event, void* context) {
    furi_assert(_event);
    furi_assert(context);

    Bt* bt = context;
    BtMessage message = {};
    const PowerEvent* event = _event;
    bool is_charging = false;
    switch(event->type) {
    case PowerEventTypeBatteryLevelChanged:
        message.type = BtMessageTypeUpdateBatteryLevel;
        message.data.battery_level = event->data.battery_level;
        furi_check(
            furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
        break;
    case PowerEventTypeStartCharging:
        is_charging = true;
        /* fallthrough */
    case PowerEventTypeFullyCharged:
    case PowerEventTypeStopCharging:
        message.type = BtMessageTypeUpdatePowerState;
        message.data.power_state_charging = is_charging;
        furi_check(
            furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
        break;
    }
}

Bt* bt_alloc(void) {
    Bt* bt = malloc(sizeof(Bt));
    // Init default maximum packet size
    bt->max_packet_size = BLE_PROFILE_SERIAL_PACKET_SIZE_MAX;
    bt->current_profile = NULL;
    // Keys storage
    bt->keys_storage = bt_keys_storage_alloc(BT_KEYS_STORAGE_PATH);
    // Alloc queue
    bt->message_queue = furi_message_queue_alloc(8, sizeof(BtMessage));

    // Setup statusbar view port
    bt->statusbar_view_port = bt_statusbar_view_port_alloc(bt);
    // Notification
    bt->notification = furi_record_open(RECORD_NOTIFICATION);
    // Gui
    bt->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(bt->gui, bt->statusbar_view_port, GuiLayerStatusBarLeft);

    // Dialogs
    bt->dialogs = furi_record_open(RECORD_DIALOGS);

    // Power
    bt->power = furi_record_open(RECORD_POWER);
    FuriPubSub* power_pubsub = power_get_pubsub(bt->power);
    furi_pubsub_subscribe(power_pubsub, bt_battery_level_changed_callback, bt);

    // RPC
    bt->rpc = furi_record_open(RECORD_RPC);
    bt->rpc_event = furi_event_flag_alloc();

    // API evnent
    bt->api_event = furi_event_flag_alloc();

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
                TAG, "Only %zu of %u bytes processed by RPC", bytes_processed, event.data.size);
        }
        ret = rpc_session_get_available_size(bt->rpc_session);
    } else if(event.event == SerialServiceEventTypeDataSent) {
        furi_event_flag_set(bt->rpc_event, BT_RPC_EVENT_BUFF_SENT);
    } else if(event.event == SerialServiceEventTypesBleResetRequest) {
        FURI_LOG_I(TAG, "BLE restart request received");
        BtMessage message = {
            .type = BtMessageTypeSetProfile,
            .data.profile.params = NULL,
            .data.profile.template = ble_profile_serial,
        };
        furi_check(
            furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
    }
    return ret;
}

// Called from RPC thread
static void bt_rpc_send_bytes_callback(void* context, uint8_t* bytes, size_t bytes_len) {
    furi_assert(context);
    Bt* bt = context;

    if(furi_event_flag_get(bt->rpc_event) & BT_RPC_EVENT_DISCONNECTED) {
        // Early stop from sending if we're already disconnected
        return;
    }
    furi_event_flag_clear(bt->rpc_event, BT_RPC_EVENT_ALL & (~BT_RPC_EVENT_DISCONNECTED));
    size_t bytes_sent = 0;
    while(bytes_sent < bytes_len) {
        size_t bytes_remain = bytes_len - bytes_sent;
        if(bytes_remain > bt->max_packet_size) {
            ble_profile_serial_tx(bt->current_profile, &bytes[bytes_sent], bt->max_packet_size);
            bytes_sent += bt->max_packet_size;
        } else {
            ble_profile_serial_tx(bt->current_profile, &bytes[bytes_sent], bytes_remain);
            bytes_sent += bytes_remain;
        }
        // We want BT_RPC_EVENT_DISCONNECTED to stick, so don't clear
        uint32_t event_flag = furi_event_flag_wait(
            bt->rpc_event, BT_RPC_EVENT_ALL, FuriFlagWaitAny | FuriFlagNoClear, FuriWaitForever);
        if(event_flag & BT_RPC_EVENT_DISCONNECTED) {
            break;
        } else {
            // If we didn't get BT_RPC_EVENT_DISCONNECTED, then clear everything else
            furi_event_flag_clear(bt->rpc_event, BT_RPC_EVENT_ALL & (~BT_RPC_EVENT_DISCONNECTED));
        }
    }
}

static void bt_serial_buffer_is_empty_callback(void* context) {
    furi_assert(context);
    Bt* bt = context;
    furi_check(furi_hal_bt_check_profile_type(bt->current_profile, ble_profile_serial));
    ble_profile_serial_notify_buffer_is_empty(bt->current_profile);
}

// Called from GAP thread
static bool bt_on_gap_event_callback(GapEvent event, void* context) {
    furi_assert(context);
    Bt* bt = context;
    bool ret = false;
    bool do_update_status = false;
    bool current_profile_is_serial =
        furi_hal_bt_check_profile_type(bt->current_profile, ble_profile_serial);

    if(event.type == GapEventTypeConnected) {
        // Update status bar
        bt->status = BtStatusConnected;
        do_update_status = true;
        // Clear BT_RPC_EVENT_DISCONNECTED because it might be set from previous session
        furi_event_flag_clear(bt->rpc_event, BT_RPC_EVENT_DISCONNECTED);

        if(current_profile_is_serial) {
            // Open RPC session
            bt->rpc_session = rpc_session_open(bt->rpc, RpcOwnerBle);
            if(bt->rpc_session) {
                FURI_LOG_I(TAG, "Open RPC connection");
                rpc_session_set_send_bytes_callback(bt->rpc_session, bt_rpc_send_bytes_callback);
                rpc_session_set_buffer_is_empty_callback(
                    bt->rpc_session, bt_serial_buffer_is_empty_callback);
                rpc_session_set_context(bt->rpc_session, bt);
                ble_profile_serial_set_event_callback(
                    bt->current_profile, RPC_BUFFER_SIZE, bt_serial_event_callback, bt);
                ble_profile_serial_set_rpc_active(
                    bt->current_profile, FuriHalBtSerialRpcStatusActive);
            } else {
                FURI_LOG_W(TAG, "RPC is busy, failed to open new session");
            }
        }
        // Update battery level
        PowerInfo info;
        power_get_info(bt->power, &info);
        BtMessage message = {.type = BtMessageTypeUpdateStatus};
        message.type = BtMessageTypeUpdateBatteryLevel;
        message.data.battery_level = info.charge;
        furi_check(
            furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
        ret = true;
    } else if(event.type == GapEventTypeDisconnected) {
        if(current_profile_is_serial && bt->rpc_session) {
            FURI_LOG_I(TAG, "Close RPC connection");
            ble_profile_serial_set_rpc_active(
                bt->current_profile, FuriHalBtSerialRpcStatusNotActive);
            furi_event_flag_set(bt->rpc_event, BT_RPC_EVENT_DISCONNECTED);
            rpc_session_close(bt->rpc_session);
            ble_profile_serial_set_event_callback(bt->current_profile, 0, NULL, NULL);
            bt->rpc_session = NULL;
        }
        ret = true;
    } else if(event.type == GapEventTypeStartAdvertising) {
        bt->status = BtStatusAdvertising;
        do_update_status = true;
        ret = true;
    } else if(event.type == GapEventTypeStopAdvertising) {
        bt->status = BtStatusOff;
        do_update_status = true;
        ret = true;
    } else if(event.type == GapEventTypePinCodeShow) {
        BtMessage message = {
            .type = BtMessageTypePinCodeShow, .data.pin_code = event.data.pin_code};
        furi_check(
            furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
        ret = true;
    } else if(event.type == GapEventTypePinCodeVerify) {
        ret = bt_pin_code_verify_event_handler(bt, event.data.pin_code);
    } else if(event.type == GapEventTypeUpdateMTU) {
        bt->max_packet_size = event.data.max_packet_size;
        ret = true;
    } else if(event.type == GapEventTypeBeaconStart) {
        bt->beacon_active = true;
        do_update_status = true;
        ret = true;
    } else if(event.type == GapEventTypeBeaconStop) {
        bt->beacon_active = false;
        do_update_status = true;
        ret = true;
    }

    if(do_update_status) {
        BtMessage message = {.type = BtMessageTypeUpdateStatus};
        furi_check(
            furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
    }
    return ret;
}

static void bt_on_key_storage_change_callback(uint8_t* addr, uint16_t size, void* context) {
    furi_assert(context);
    Bt* bt = context;
    BtMessage message = {
        .type = BtMessageTypeKeysStorageUpdated,
        .data.key_storage_data.start_address = addr,
        .data.key_storage_data.size = size};
    furi_check(
        furi_message_queue_put(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
}

static void bt_statusbar_update(Bt* bt) {
    uint8_t active_icon_width = 0;
    if(bt->beacon_active) {
        active_icon_width = icon_get_width(&I_BLE_beacon_7x8) + ICON_SPACER;
    }
    if(bt->status == BtStatusAdvertising) {
        active_icon_width += icon_get_width(&I_Bluetooth_Idle_5x8);
    } else if(bt->status == BtStatusConnected) {
        active_icon_width += icon_get_width(&I_Bluetooth_Connected_16x8);
    }

    if(active_icon_width > 0) {
        view_port_set_width(bt->statusbar_view_port, active_icon_width);
        view_port_enabled_set(bt->statusbar_view_port, true);
    } else {
        view_port_enabled_set(bt->statusbar_view_port, false);
    }
}

static void bt_show_warning(Bt* bt, const char* text) {
    if(!bt->dialog_message) {
        bt->dialog_message = dialog_message_alloc();
    }
    dialog_message_set_text(bt->dialog_message, text, 64, 28, AlignCenter, AlignCenter);
    dialog_message_set_buttons(bt->dialog_message, "Quit", NULL, NULL);
    dialog_message_show(bt->dialogs, bt->dialog_message);
}

static void bt_close_rpc_connection(Bt* bt) {
    if(furi_hal_bt_check_profile_type(bt->current_profile, ble_profile_serial) &&
       bt->rpc_session) {
        FURI_LOG_I(TAG, "Close RPC connection");
        furi_event_flag_set(bt->rpc_event, BT_RPC_EVENT_DISCONNECTED);
        rpc_session_close(bt->rpc_session);
        ble_profile_serial_set_event_callback(bt->current_profile, 0, NULL, NULL);
        bt->rpc_session = NULL;
    }
}

static void bt_change_profile(Bt* bt, BtMessage* message) {
    if(furi_hal_bt_is_gatt_gap_supported()) {
        bt_settings_load(&bt->bt_settings);

        bt_close_rpc_connection(bt);

        bt_keys_storage_load(bt->keys_storage);

        bt->current_profile = furi_hal_bt_change_app(
            message->data.profile.template,
            message->data.profile.params,
            bt_on_gap_event_callback,
            bt);
        if(bt->current_profile) {
            FURI_LOG_I(TAG, "Bt App started");
            if(bt->bt_settings.enabled) {
                furi_hal_bt_start_advertising();
            }
            furi_hal_bt_set_key_storage_change_callback(bt_on_key_storage_change_callback, bt);
        } else {
            FURI_LOG_E(TAG, "Failed to start Bt App");
        }
        if(message->profile_instance) {
            *message->profile_instance = bt->current_profile;
        }
        if(message->result) {
            *message->result = bt->current_profile != NULL;
        }

    } else {
        bt_show_warning(bt, "Radio stack doesn't support this app");
        if(message->result) {
            *message->result = false;
        }
        if(message->profile_instance) {
            *message->profile_instance = NULL;
        }
    }
}

static void bt_close_connection(Bt* bt) {
    bt_close_rpc_connection(bt);
    furi_hal_bt_stop_advertising();
}

static void bt_apply_settings(Bt* bt) {
    if(bt->bt_settings.enabled) {
        furi_hal_bt_start_advertising();
    } else {
        furi_hal_bt_stop_advertising();
    }
}

static void bt_load_keys(Bt* bt) {
    if(!furi_hal_bt_is_gatt_gap_supported()) {
        bt_show_warning(bt, "Unsupported radio stack");
        bt->status = BtStatusUnavailable;
        return;

    } else if(bt_keys_storage_is_changed(bt->keys_storage)) {
        FURI_LOG_I(TAG, "Loading new keys");

        bt_close_rpc_connection(bt);
        bt_keys_storage_load(bt->keys_storage);

        bt->current_profile = NULL;

    } else {
        FURI_LOG_I(TAG, "Keys unchanged");
    }
}

static void bt_start_application(Bt* bt) {
    if(!bt->current_profile) {
        bt->current_profile =
            furi_hal_bt_change_app(ble_profile_serial, NULL, bt_on_gap_event_callback, bt);

        if(!bt->current_profile) {
            FURI_LOG_E(TAG, "BLE App start failed");
            bt->status = BtStatusUnavailable;
        }
    }
}

static void bt_load_settings(Bt* bt) {
    bt_settings_load(&bt->bt_settings);
    bt_apply_settings(bt);
}

static void bt_handle_get_settings(Bt* bt, BtMessage* message) {
    *message->data.settings = bt->bt_settings;
}

static void bt_handle_set_settings(Bt* bt, BtMessage* message) {
    bt->bt_settings = *message->data.csettings;
    bt_apply_settings(bt);
    bt_settings_save(&bt->bt_settings);
}

static void bt_handle_reload_keys_settings(Bt* bt) {
    bt_load_keys(bt);
    bt_start_application(bt);
    bt_load_settings(bt);
}

static void bt_init_keys_settings(Bt* bt) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_pubsub_subscribe(storage_get_pubsub(storage), bt_storage_callback, bt);

    if(storage_sd_status(storage) != FSE_OK) {
        FURI_LOG_D(TAG, "SD Card not ready, skipping settings");

        // Just start the BLE serial application without loading the keys or settings
        bt_start_application(bt);
        return;
    }

    bt_handle_reload_keys_settings(bt);
}

int32_t bt_srv(void* p) {
    UNUSED(p);
    Bt* bt = bt_alloc();

    if(furi_hal_rtc_get_boot_mode() != FuriHalRtcBootModeNormal) {
        FURI_LOG_W(TAG, "Skipping start in special boot mode");
        ble_glue_wait_for_c2_start(FURI_HAL_BT_C2_START_TIMEOUT);
        furi_record_create(RECORD_BT, bt);

        furi_thread_suspend(furi_thread_get_current_id());
        return 0;
    }

    if(furi_hal_bt_start_radio_stack()) {
        bt_init_keys_settings(bt);
        furi_hal_bt_set_key_storage_change_callback(bt_on_key_storage_change_callback, bt);

    } else {
        FURI_LOG_E(TAG, "Radio stack start failed");
    }

    furi_record_create(RECORD_BT, bt);

    BtMessage message;

    while(1) {
        furi_check(
            furi_message_queue_get(bt->message_queue, &message, FuriWaitForever) == FuriStatusOk);
        FURI_LOG_D(
            TAG,
            "call %d, lock 0x%p, result 0x%p",
            message.type,
            (void*)message.lock,
            (void*)message.result);
        if(message.type == BtMessageTypeUpdateStatus) {
            // Update view ports
            bt_statusbar_update(bt);
            bt_pin_code_hide(bt);
            if(bt->status_changed_cb) {
                bt->status_changed_cb(bt->status, bt->status_changed_ctx);
            }
        } else if(message.type == BtMessageTypeUpdateBatteryLevel) {
            // Update battery level
            furi_hal_bt_update_battery_level(message.data.battery_level);
        } else if(message.type == BtMessageTypeUpdatePowerState) {
            furi_hal_bt_update_power_state(message.data.power_state_charging);
        } else if(message.type == BtMessageTypePinCodeShow) {
            // Display PIN code
            bt_pin_code_show(bt, message.data.pin_code);
        } else if(message.type == BtMessageTypeKeysStorageUpdated) {
            bt_keys_storage_update(
                bt->keys_storage,
                message.data.key_storage_data.start_address,
                message.data.key_storage_data.size);
        } else if(message.type == BtMessageTypeSetProfile) {
            bt_change_profile(bt, &message);
        } else if(message.type == BtMessageTypeDisconnect) {
            bt_close_connection(bt);
        } else if(message.type == BtMessageTypeForgetBondedDevices) {
            bt_keys_storage_delete(bt->keys_storage);
        } else if(message.type == BtMessageTypeGetSettings) {
            bt_handle_get_settings(bt, &message);
        } else if(message.type == BtMessageTypeSetSettings) {
            bt_handle_set_settings(bt, &message);
        } else if(message.type == BtMessageTypeReloadKeysSettings) {
            bt_handle_reload_keys_settings(bt);
        }

        if(message.lock) api_lock_unlock(message.lock);
    }

    return 0;
}
