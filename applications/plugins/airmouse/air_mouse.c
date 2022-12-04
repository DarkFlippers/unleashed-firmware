#include "air_mouse.h"

#include <furi.h>
#include <dolphin/dolphin.h>

#include "tracking/imu/imu.h"

#define TAG "AirMouseApp"

enum AirMouseSubmenuIndex {
    AirMouseSubmenuIndexBtMouse,
    AirMouseSubmenuIndexUsbMouse,
    AirMouseSubmenuIndexCalibration,
};

void air_mouse_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    AirMouse* app = context;
    if(index == AirMouseSubmenuIndexBtMouse) {
        app->view_id = AirMouseViewBtMouse;
        view_dispatcher_switch_to_view(app->view_dispatcher, AirMouseViewBtMouse);
    } else if(index == AirMouseSubmenuIndexUsbMouse) {
        app->view_id = AirMouseViewUsbMouse;
        view_dispatcher_switch_to_view(app->view_dispatcher, AirMouseViewUsbMouse);
    } else if(index == AirMouseSubmenuIndexCalibration) {
        app->view_id = AirMouseViewCalibration;
        view_dispatcher_switch_to_view(app->view_dispatcher, AirMouseViewCalibration);
    }
}

void air_mouse_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);
    AirMouse* app = context;
    if(result == DialogExResultLeft) {
        view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_NONE); // Exit
    } else if(result == DialogExResultRight) {
        view_dispatcher_switch_to_view(app->view_dispatcher, app->view_id); // Show last view
    } else if(result == DialogExResultCenter) {
        view_dispatcher_switch_to_view(app->view_dispatcher, AirMouseViewSubmenu); // Menu
    }
}

uint32_t air_mouse_exit_confirm_view(void* context) {
    UNUSED(context);
    return AirMouseViewExitConfirm;
}

uint32_t air_mouse_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

AirMouse* air_mouse_app_alloc() {
    AirMouse* app = malloc(sizeof(AirMouse));

    // Gui
    app->gui = furi_record_open(RECORD_GUI);

    // View dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Submenu view
    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu, "Bluetooth", AirMouseSubmenuIndexBtMouse, air_mouse_submenu_callback, app);
    submenu_add_item(
        app->submenu, "USB", AirMouseSubmenuIndexUsbMouse, air_mouse_submenu_callback, app);
    submenu_add_item(
        app->submenu,
        "Calibration",
        AirMouseSubmenuIndexCalibration,
        air_mouse_submenu_callback,
        app);
    view_set_previous_callback(submenu_get_view(app->submenu), air_mouse_exit);
    view_dispatcher_add_view(
        app->view_dispatcher, AirMouseViewSubmenu, submenu_get_view(app->submenu));

    // Dialog view
    app->dialog = dialog_ex_alloc();
    dialog_ex_set_result_callback(app->dialog, air_mouse_dialog_callback);
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_left_button_text(app->dialog, "Exit");
    dialog_ex_set_right_button_text(app->dialog, "Stay");
    dialog_ex_set_center_button_text(app->dialog, "Menu");
    dialog_ex_set_header(app->dialog, "Close Current App?", 16, 12, AlignLeft, AlignTop);
    view_dispatcher_add_view(
        app->view_dispatcher, AirMouseViewExitConfirm, dialog_ex_get_view(app->dialog));

    // Bluetooth view
    app->bt_mouse = bt_mouse_alloc(app->view_dispatcher);
    view_set_previous_callback(bt_mouse_get_view(app->bt_mouse), air_mouse_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher, AirMouseViewBtMouse, bt_mouse_get_view(app->bt_mouse));

    // USB view
    app->usb_mouse = usb_mouse_alloc(app->view_dispatcher);
    view_set_previous_callback(usb_mouse_get_view(app->usb_mouse), air_mouse_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher, AirMouseViewUsbMouse, usb_mouse_get_view(app->usb_mouse));

    // Calibration view
    app->calibration = calibration_alloc(app->view_dispatcher);
    view_set_previous_callback(
        calibration_get_view(app->calibration), air_mouse_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher, AirMouseViewCalibration, calibration_get_view(app->calibration));

    app->view_id = AirMouseViewSubmenu;
    view_dispatcher_switch_to_view(app->view_dispatcher, app->view_id);

    return app;
}

void air_mouse_app_free(AirMouse* app) {
    furi_assert(app);

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, AirMouseViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, AirMouseViewExitConfirm);
    dialog_ex_free(app->dialog);
    view_dispatcher_remove_view(app->view_dispatcher, AirMouseViewBtMouse);
    bt_mouse_free(app->bt_mouse);
    view_dispatcher_remove_view(app->view_dispatcher, AirMouseViewUsbMouse);
    usb_mouse_free(app->usb_mouse);
    view_dispatcher_remove_view(app->view_dispatcher, AirMouseViewCalibration);
    calibration_free(app->calibration);
    view_dispatcher_free(app->view_dispatcher);

    // Close records
    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    // Free rest
    free(app);
}

int32_t air_mouse_app(void* p) {
    UNUSED(p);

    AirMouse* app = air_mouse_app_alloc();
    if(!imu_begin()) {
        air_mouse_app_free(app);
        return -1;
    }

    DOLPHIN_DEED(DolphinDeedPluginStart);
    view_dispatcher_run(app->view_dispatcher);

    imu_end();
    air_mouse_app_free(app);

    return 0;
}
