#include "battery_test_app.h"

#include <notification/notification_messages.h>

void battery_test_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);
    BatteryTestApp* app = context;
    if(result == DialogExResultLeft) {
        view_dispatcher_stop(app->view_dispatcher);
    } else if(result == DialogExResultRight) {
        view_dispatcher_switch_to_view(app->view_dispatcher, BatteryTestAppViewBatteryInfo);
    }
}

uint32_t battery_test_exit_confirm_view() {
    return BatteryTestAppViewExitDialog;
}

static void battery_test_battery_info_update_model(void* context) {
    BatteryTestApp* app = context;
    power_get_info(app->power, &app->info);
    BatteryInfoModel battery_info_data = {
        .vbus_voltage = app->info.voltage_vbus,
        .gauge_voltage = app->info.voltage_gauge,
        .gauge_current = app->info.current_gauge,
        .gauge_temperature = app->info.temperature_gauge,
        .charge = app->info.charge,
        .health = app->info.health,
    };
    battery_info_set_data(app->battery_info, &battery_info_data);
    notification_message(app->notifications, &sequence_display_backlight_on);
}

BatteryTestApp* battery_test_alloc() {
    BatteryTestApp* app = malloc(sizeof(BatteryTestApp));

    // Records
    app->gui = furi_record_open(RECORD_GUI);
    app->power = furi_record_open(RECORD_POWER);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // View dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, battery_test_battery_info_update_model, 500);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Views
    app->battery_info = battery_info_alloc();
    view_set_previous_callback(
        battery_info_get_view(app->battery_info), battery_test_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher,
        BatteryTestAppViewBatteryInfo,
        battery_info_get_view(app->battery_info));

    app->dialog = dialog_ex_alloc();
    dialog_ex_set_header(app->dialog, "Close Battery Test?", 64, 12, AlignCenter, AlignTop);
    dialog_ex_set_left_button_text(app->dialog, "Exit");
    dialog_ex_set_right_button_text(app->dialog, "Stay");
    dialog_ex_set_result_callback(app->dialog, battery_test_dialog_callback);
    dialog_ex_set_context(app->dialog, app);

    view_dispatcher_add_view(
        app->view_dispatcher, BatteryTestAppViewExitDialog, dialog_ex_get_view(app->dialog));

    battery_test_battery_info_update_model(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, BatteryTestAppViewBatteryInfo);
    return app;
}

void battery_test_free(BatteryTestApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, BatteryTestAppViewBatteryInfo);
    battery_info_free(app->battery_info);
    view_dispatcher_remove_view(app->view_dispatcher, BatteryTestAppViewExitDialog);
    dialog_ex_free(app->dialog);
    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    // Records
    furi_record_close(RECORD_POWER);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    free(app);
}

int32_t battery_test_app(void* p) {
    UNUSED(p);
    BatteryTestApp* app = battery_test_alloc();
    // Disable battery low level notification
    power_enable_low_battery_level_notification(app->power, false);

    view_dispatcher_run(app->view_dispatcher);
    power_enable_low_battery_level_notification(app->power, true);
    battery_test_free(app);
    return 0;
}
