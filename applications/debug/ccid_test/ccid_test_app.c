#include <stdint.h>
#include <furi.h>
#include <furi_hal.h>

#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/gui.h>

#include "iso7816/iso7816_handler.h"
#include "iso7816/iso7816_t0_apdu.h"
#include "iso7816/iso7816_atr.h"
#include "iso7816/iso7816_response.h"

#include "ccid_test_app_commands.h"

typedef enum {
    EventTypeInput,
} EventType;

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    Submenu* submenu;
    FuriHalUsbCcidConfig ccid_cfg;
    Iso7816Handler* iso7816_handler;
} CcidTestApp;

typedef struct {
    union {
        InputEvent input;
    };
    EventType type;
} CcidTestAppEvent;

typedef enum {
    CcidTestAppViewSubmenu,
} CcidTestAppView;

typedef enum {
    CcidTestSubmenuIndexInsertSmartcard,
    CcidTestSubmenuIndexRemoveSmartcard
} SubmenuIndex;

static void ccid_test_submenu_callback(void* context, uint32_t index) {
    furi_assert(context);
    if(index == CcidTestSubmenuIndexInsertSmartcard) {
        furi_hal_usb_ccid_insert_smartcard();
    } else if(index == CcidTestSubmenuIndexRemoveSmartcard) {
        furi_hal_usb_ccid_remove_smartcard();
    }
}

uint32_t ccid_test_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

CcidTestApp* ccid_test_app_alloc(void) {
    CcidTestApp* app = malloc(sizeof(CcidTestApp));

    //setup CCID USB
    // On linux: set VID PID using: /usr/lib/pcsc/drivers/ifd-ccid.bundle/Contents/Info.plist
    app->ccid_cfg.vid = 0x076B;
    app->ccid_cfg.pid = 0x3A21;

    app->iso7816_handler = iso7816_handler_alloc();
    app->iso7816_handler->iso7816_answer_to_reset = iso7816_answer_to_reset;
    app->iso7816_handler->iso7816_process_command = iso7816_process_command;

    // Gui
    app->gui = furi_record_open(RECORD_GUI);

    // View dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Views
    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu,
        "Insert smartcard",
        CcidTestSubmenuIndexInsertSmartcard,
        ccid_test_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Remove smartcard",
        CcidTestSubmenuIndexRemoveSmartcard,
        ccid_test_submenu_callback,
        app);
    view_set_previous_callback(submenu_get_view(app->submenu), ccid_test_exit);
    view_dispatcher_add_view(
        app->view_dispatcher, CcidTestAppViewSubmenu, submenu_get_view(app->submenu));

    // Switch to menu
    view_dispatcher_switch_to_view(app->view_dispatcher, CcidTestAppViewSubmenu);
    return app;
}

void ccid_test_app_free(CcidTestApp* app) {
    furi_assert(app);

    // Free views
    view_dispatcher_remove_view(app->view_dispatcher, CcidTestAppViewSubmenu);
    view_dispatcher_free(app->view_dispatcher);
    submenu_free(app->submenu);

    // Close gui record
    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    iso7816_handler_free(app->iso7816_handler);

    // Free rest
    free(app);
}

int32_t ccid_test_app(void* p) {
    UNUSED(p);

    //setup view
    CcidTestApp* app = ccid_test_app_alloc();

    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();

    furi_check(furi_hal_usb_set_config(&usb_ccid, &app->ccid_cfg) == true);
    iso7816_handler_set_usb_ccid_callbacks();
    furi_hal_usb_ccid_insert_smartcard();

    view_dispatcher_run(app->view_dispatcher);

    //tear down USB
    iso7816_handler_reset_usb_ccid_callbacks();
    furi_hal_usb_set_config(usb_mode_prev, NULL);

    //teardown view
    ccid_test_app_free(app);
    return 0;
}
