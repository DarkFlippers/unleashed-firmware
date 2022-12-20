#include "usb_hid.h"
#include <furi.h>
#include <furi_hal.h>
#include <notification/notification_messages.h>

#define TAG "UsbHidApp"

enum UsbDebugSubmenuIndex {
    UsbHidSubmenuIndexDirpad,
    UsbHidSubmenuIndexKeyboard,
    UsbHidSubmenuIndexMedia,
    UsbHidSubmenuIndexMouse,
    UsbHidSubmenuIndexMouseJiggler,
};

void usb_hid_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    UsbHid* app = context;
    if(index == UsbHidSubmenuIndexDirpad) {
        app->view_id = UsbHidViewDirpad;
        view_dispatcher_switch_to_view(app->view_dispatcher, UsbHidViewDirpad);
    } else if(index == UsbHidSubmenuIndexKeyboard) {
        app->view_id = UsbHidViewKeyboard;
        view_dispatcher_switch_to_view(app->view_dispatcher, UsbHidViewKeyboard);
    } else if(index == UsbHidSubmenuIndexMedia) {
        app->view_id = UsbHidViewMedia;
        view_dispatcher_switch_to_view(app->view_dispatcher, UsbHidViewMedia);
    } else if(index == UsbHidSubmenuIndexMouse) {
        app->view_id = UsbHidViewMouse;
        view_dispatcher_switch_to_view(app->view_dispatcher, UsbHidViewMouse);
    } else if(index == UsbHidSubmenuIndexMouseJiggler) {
        app->view_id = UsbHidViewMouseJiggler;
        view_dispatcher_switch_to_view(app->view_dispatcher, UsbHidViewMouseJiggler);
    }
}

void usb_hid_dialog_callback(DialogExResult result, void* context) {
    furi_assert(context);
    UsbHid* app = context;
    if(result == DialogExResultLeft) {
        view_dispatcher_stop(app->view_dispatcher);
    } else if(result == DialogExResultRight) {
        view_dispatcher_switch_to_view(app->view_dispatcher, app->view_id); // Show last view
    } else if(result == DialogExResultCenter) {
        view_dispatcher_switch_to_view(app->view_dispatcher, UsbHidViewSubmenu);
    }
}

uint32_t usb_hid_exit_confirm_view(void* context) {
    UNUSED(context);
    return UsbHidViewExitConfirm;
}

uint32_t usb_hid_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

UsbHid* usb_hid_app_alloc() {
    UsbHid* app = malloc(sizeof(UsbHid));

    // Gui
    app->gui = furi_record_open(RECORD_GUI);

    // Notifications
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // View dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Submenu view
    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu, "Dirpad", UsbHidSubmenuIndexDirpad, usb_hid_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Keyboard", UsbHidSubmenuIndexKeyboard, usb_hid_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Media", UsbHidSubmenuIndexMedia, usb_hid_submenu_callback, app);
    submenu_add_item(
        app->submenu, "Mouse", UsbHidSubmenuIndexMouse, usb_hid_submenu_callback, app);
    submenu_add_item(
        app->submenu,
        "Mouse Jiggler",
        UsbHidSubmenuIndexMouseJiggler,
        usb_hid_submenu_callback,
        app);
    view_set_previous_callback(submenu_get_view(app->submenu), usb_hid_exit);
    view_dispatcher_add_view(
        app->view_dispatcher, UsbHidViewSubmenu, submenu_get_view(app->submenu));

    // Dialog view
    app->dialog = dialog_ex_alloc();
    dialog_ex_set_result_callback(app->dialog, usb_hid_dialog_callback);
    dialog_ex_set_context(app->dialog, app);
    dialog_ex_set_left_button_text(app->dialog, "Exit");
    dialog_ex_set_right_button_text(app->dialog, "Stay");
    dialog_ex_set_center_button_text(app->dialog, "Menu");
    dialog_ex_set_header(app->dialog, "Close Current App?", 16, 12, AlignLeft, AlignTop);
    view_dispatcher_add_view(
        app->view_dispatcher, UsbHidViewExitConfirm, dialog_ex_get_view(app->dialog));

    // Dirpad view
    app->usb_hid_dirpad = usb_hid_dirpad_alloc();
    view_set_previous_callback(
        usb_hid_dirpad_get_view(app->usb_hid_dirpad), usb_hid_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher, UsbHidViewDirpad, usb_hid_dirpad_get_view(app->usb_hid_dirpad));

    // Keyboard view
    app->usb_hid_keyboard = usb_hid_keyboard_alloc();
    view_set_previous_callback(
        usb_hid_keyboard_get_view(app->usb_hid_keyboard), usb_hid_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher,
        UsbHidViewKeyboard,
        usb_hid_keyboard_get_view(app->usb_hid_keyboard));

    // Media view
    app->usb_hid_media = usb_hid_media_alloc();
    view_set_previous_callback(
        usb_hid_media_get_view(app->usb_hid_media), usb_hid_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher, UsbHidViewMedia, usb_hid_media_get_view(app->usb_hid_media));

    // Mouse view
    app->usb_hid_mouse = usb_hid_mouse_alloc();
    view_set_previous_callback(
        usb_hid_mouse_get_view(app->usb_hid_mouse), usb_hid_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher, UsbHidViewMouse, usb_hid_mouse_get_view(app->usb_hid_mouse));

    // Mouse jiggler view
    app->hid_mouse_jiggler = hid_mouse_jiggler_alloc(app);
    view_set_previous_callback(
        hid_mouse_jiggler_get_view(app->hid_mouse_jiggler), usb_hid_exit_confirm_view);
    view_dispatcher_add_view(
        app->view_dispatcher,
        UsbHidViewMouseJiggler,
        hid_mouse_jiggler_get_view(app->hid_mouse_jiggler));

    // TODO switch to menu after Media is done
    app->view_id = UsbHidViewSubmenu;
    view_dispatcher_switch_to_view(app->view_dispatcher, app->view_id);

    return app;
}

void usb_hid_app_free(UsbHid* app) {
    furi_assert(app);

    // Reset notification
    notification_internal_message(app->notifications, &sequence_reset_blue);

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, UsbHidViewSubmenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, UsbHidViewExitConfirm);
    dialog_ex_free(app->dialog);
    view_dispatcher_remove_view(app->view_dispatcher, UsbHidViewDirpad);
    usb_hid_dirpad_free(app->usb_hid_dirpad);
    view_dispatcher_remove_view(app->view_dispatcher, UsbHidViewKeyboard);
    usb_hid_keyboard_free(app->usb_hid_keyboard);
    view_dispatcher_remove_view(app->view_dispatcher, UsbHidViewMedia);
    usb_hid_media_free(app->usb_hid_media);
    view_dispatcher_remove_view(app->view_dispatcher, UsbHidViewMouse);
    usb_hid_mouse_free(app->usb_hid_mouse);
    view_dispatcher_remove_view(app->view_dispatcher, UsbHidViewMouseJiggler);
    hid_mouse_jiggler_free(app->hid_mouse_jiggler);
    view_dispatcher_free(app->view_dispatcher);
    // Close records
    furi_record_close(RECORD_GUI);
    app->gui = NULL;
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Free rest
    free(app);
}

int32_t usb_hid_app(void* p) {
    UNUSED(p);
    // Switch profile to Hid
    UsbHid* app = usb_hid_app_alloc();

    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_hid, NULL) == true);

    view_dispatcher_run(app->view_dispatcher);

    // Change back profile
    furi_hal_usb_set_config(usb_mode_prev, NULL);
    usb_hid_app_free(app);

    return 0;
}
