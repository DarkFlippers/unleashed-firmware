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
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
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
    CcidTestSubmenuIndexInsertSmartcard,
    CcidTestSubmenuIndexRemoveSmartcard,
    CcidTestSubmenuIndexInsertSmartcardReader
} SubmenuIndex;

static void ccid_test_app_render_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "CCID Test App");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 63, "Hold [back] to exit");
}

static void ccid_test_app_input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;

    CcidTestAppEvent event;
    event.type = EventTypeInput;
    event.input = *input_event;
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
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

    //viewport
    app->view_port = view_port_alloc();
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    view_port_draw_callback_set(app->view_port, ccid_test_app_render_callback, NULL);

    //message queue
    app->event_queue = furi_message_queue_alloc(8, sizeof(CcidTestAppEvent));
    view_port_input_callback_set(app->view_port, ccid_test_app_input_callback, app->event_queue);

    return app;
}

void ccid_test_app_free(CcidTestApp* app) {
    furi_assert(app);

    //message queue
    furi_message_queue_free(app->event_queue);

    //view port
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);

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

    //handle button events
    CcidTestAppEvent event;
    while(1) {
        FuriStatus event_status =
            furi_message_queue_get(app->event_queue, &event, FuriWaitForever);

        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeInput) {
                if(event.input.type == InputTypeLong && event.input.key == InputKeyBack) {
                    break;
                }
            }
        }
        view_port_update(app->view_port);
    }

    //tear down USB
    iso7816_handler_reset_usb_ccid_callbacks();
    furi_hal_usb_set_config(usb_mode_prev, NULL);

    //teardown view
    ccid_test_app_free(app);
    return 0;
}
