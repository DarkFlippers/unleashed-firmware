#include "bt_i.h"
#include "battery_service.h"

#define BT_SERVICE_TAG "BT"

// static void bt_update_statusbar(void* arg) {
//     furi_assert(arg);
//     Bt* bt = arg;
//     BtMessage m = {.type = BtMessageTypeUpdateStatusbar};
//     furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
// }

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

Bt* bt_alloc() {
    Bt* bt = furi_alloc(sizeof(Bt));
    // Load settings
    if(!bt_settings_load(&bt->bt_settings)) {
        bt_settings_save(&bt->bt_settings);
    }
    // Alloc queue
    bt->message_queue = osMessageQueueNew(8, sizeof(BtMessage), NULL);

    // doesn't make sense if we waiting for transition on service start
    // bt->update_status_timer = osTimerNew(bt_update_statusbar, osTimerPeriodic, bt, NULL);
    // osTimerStart(bt->update_status_timer, 4000);

    // Setup statusbar view port
    bt->statusbar_view_port = bt_statusbar_view_port_alloc();
    // Gui
    bt->gui = furi_record_open("gui");
    gui_add_view_port(bt->gui, bt->statusbar_view_port, GuiLayerStatusBarLeft);

    // Dialogs
    bt->dialogs = furi_record_open("dialogs");
    bt->dialog_message = dialog_message_alloc();

    return bt;
}

int32_t bt_srv() {
    Bt* bt = bt_alloc();
    furi_record_create("bt", bt);

    if(!furi_hal_bt_wait_startup()) {
        FURI_LOG_E(BT_SERVICE_TAG, "Core2 startup failed");
    } else {
        view_port_enabled_set(bt->statusbar_view_port, true);
        if(bt->bt_settings.enabled) {
            bool bt_app_started = furi_hal_bt_start_app();
            if(!bt_app_started) {
                FURI_LOG_E(BT_SERVICE_TAG, "BT App start failed");
            } else {
                FURI_LOG_I(BT_SERVICE_TAG, "BT App started");
            }
        }
    }

    BtMessage message;
    while(1) {
        furi_check(osMessageQueueGet(bt->message_queue, &message, NULL, osWaitForever) == osOK);
        if(message.type == BtMessageTypeUpdateStatusbar) {
            // Update statusbar
            view_port_enabled_set(bt->statusbar_view_port, furi_hal_bt_is_alive());
        } else if(message.type == BtMessageTypeUpdateBatteryLevel) {
            // Update battery level
            if(furi_hal_bt_is_alive()) {
                battery_svc_update_level(message.data.battery_level);
            }
        } else if(message.type == BtMessageTypePinCodeShow) {
            // Display PIN code
            bt_pin_code_show_event_handler(bt, message.data.pin_code);
        }
    }
    return 0;
}
