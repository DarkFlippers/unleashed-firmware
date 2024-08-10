#include <furi.h>
#include <furi_hal.h>

#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/gui.h>

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    FuriHalUsbHidConfig hid_cfg;
} UsbTestApp;

typedef enum {
    UsbTestSubmenuIndexEnable,
    UsbTestSubmenuIndexDisable,
    UsbTestSubmenuIndexRestart,
    UsbTestSubmenuIndexVcpSingle,
    UsbTestSubmenuIndexVcpDual,
    UsbTestSubmenuIndexHid,
    UsbTestSubmenuIndexHidWithParams,
    UsbTestSubmenuIndexHidU2F,
} SubmenuIndex;

void usb_test_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    UsbTestApp* app = context;
    if(index == UsbTestSubmenuIndexEnable) {
        furi_hal_usb_enable();
    } else if(index == UsbTestSubmenuIndexDisable) {
        furi_hal_usb_disable();
    } else if(index == UsbTestSubmenuIndexRestart) {
        furi_hal_usb_reinit();
    } else if(index == UsbTestSubmenuIndexVcpSingle) {
        furi_hal_usb_set_config(&usb_cdc_single, NULL);
    } else if(index == UsbTestSubmenuIndexVcpDual) {
        furi_hal_usb_set_config(&usb_cdc_dual, NULL);
    } else if(index == UsbTestSubmenuIndexHid) {
        furi_hal_usb_set_config(&usb_hid, NULL);
    } else if(index == UsbTestSubmenuIndexHidWithParams) {
        app->hid_cfg.vid = 0x1234;
        app->hid_cfg.pid = 0xabcd;
        strlcpy(app->hid_cfg.manuf, "WEN", sizeof(app->hid_cfg.manuf));
        strlcpy(app->hid_cfg.product, "FLIP", sizeof(app->hid_cfg.product));
        furi_hal_usb_set_config(&usb_hid, &app->hid_cfg);
    } else if(index == UsbTestSubmenuIndexHidU2F) {
        furi_hal_usb_set_config(&usb_hid_u2f, NULL);
    }
}

uint32_t usb_test_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

UsbTestApp* usb_test_app_alloc(void) {
    UsbTestApp* app = malloc(sizeof(UsbTestApp));

    // Gui
    app->gui = furi_record_open(RECORD_GUI);

    // View dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Views
    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu, "Enable", UsbTestSubmenuIndexEnable, usb_test_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Disable", UsbTestSubmenuIndexDisable, usb_test_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Restart", UsbTestSubmenuIndexRestart, usb_test_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Single VCP", UsbTestSubmenuIndexVcpSingle, usb_test_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Dual VCP", UsbTestSubmenuIndexVcpDual, usb_test_submenu_callback, app);
    submenu_add_item(
        app->submenu, "HID KB+Mouse", UsbTestSubmenuIndexHid, usb_test_submenu_callback, app);
    submenu_add_item(
        app->submenu,
        "HID KB+Mouse custom ID",
        UsbTestSubmenuIndexHidWithParams,
        usb_test_submenu_callback,
        app);
    submenu_add_item(
        app->submenu, "HID U2F", UsbTestSubmenuIndexHidU2F, usb_test_submenu_callback, app);
    view_set_previous_callback(submenu_get_view(app->submenu), usb_test_exit);
    view_dispatcher_add_view(app->view_dispatcher, 0, submenu_get_view(app->submenu));

    // Switch to menu
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    return app;
}

void usb_test_app_free(UsbTestApp* app) {
    furi_assert(app);

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    submenu_free(app->submenu);
    view_dispatcher_free(app->view_dispatcher);

    // Close gui record
    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    // Free rest
    free(app);
}

int32_t usb_test_app(void* p) {
    UNUSED(p);
    UsbTestApp* app = usb_test_app_alloc();

    view_dispatcher_run(app->view_dispatcher);

    usb_test_app_free(app);
    return 0;
}
