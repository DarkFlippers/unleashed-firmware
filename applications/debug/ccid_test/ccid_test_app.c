#include <stdint.h>
#include <furi.h>
#include <furi_hal.h>

#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/gui.h>
#include "iso7816_callbacks.h"
#include "iso7816_t0_apdu.h"
#include "iso7816_atr.h"

typedef enum {
    EventTypeInput,
} EventType;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    FuriHalUsbCcidConfig ccid_cfg;
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

    // Free rest
    free(app);
}

void ccid_icc_power_on_callback(uint8_t* atrBuffer, uint32_t* atrlen, void* context) {
    UNUSED(context);

    iso7816_icc_power_on_callback(atrBuffer, atrlen);
}

void ccid_xfr_datablock_callback(
    const uint8_t* pcToReaderDataBlock,
    uint32_t pcToReaderDataBlockLen,
    uint8_t* readerToPcDataBlock,
    uint32_t* readerToPcDataBlockLen,
    void* context) {
    UNUSED(context);

    iso7816_xfr_datablock_callback(
        pcToReaderDataBlock, pcToReaderDataBlockLen, readerToPcDataBlock, readerToPcDataBlockLen);
}

static const CcidCallbacks ccid_cb = {
    ccid_icc_power_on_callback,
    ccid_xfr_datablock_callback,
};

void iso7816_answer_to_reset(Iso7816Atr* atr) {
    //minimum valid ATR: https://smartcard-atr.apdu.fr/parse?ATR=3B+00
    atr->TS = 0x3B;
    atr->T0 = 0x00;
}

void iso7816_process_command(
    const struct ISO7816_Command_APDU* commandAPDU,
    struct ISO7816_Response_APDU* responseAPDU,
    const uint8_t* commandApduDataBuffer,
    uint8_t commandApduDataBufferLen,
    uint8_t* responseApduDataBuffer,
    uint8_t* responseApduDataBufferLen) {
    //example 1: sends a command with no body, receives a response with no body
    //sends APDU 0x01:0x02:0x00:0x00
    //receives SW1=0x90, SW2=0x00
    if(commandAPDU->CLA == 0x01 && commandAPDU->INS == 0x01) {
        responseAPDU->SW1 = 0x90;
        responseAPDU->SW2 = 0x00;
    }
    //example 2: sends a command with no body, receives a response with a body with two bytes
    //sends APDU 0x01:0x02:0x00:0x00
    //receives 'bc' (0x62, 0x63) SW1=0x80, SW2=0x10
    else if(commandAPDU->CLA == 0x01 && commandAPDU->INS == 0x02) {
        responseApduDataBuffer[0] = 0x62;
        responseApduDataBuffer[1] = 0x63;

        *responseApduDataBufferLen = 2;

        responseAPDU->SW1 = 0x90;
        responseAPDU->SW2 = 0x00;
    }
    //example 3: ends a command with a body with two bytes, receives a response with  a body with two bytes
    //sends APDU 0x01:0x03:0x00:0x00:0x02:CA:FE
    //receives (0xCA, 0xFE) SW1=0x90, SW2=0x02
    else if(
        commandAPDU->CLA == 0x01 && commandAPDU->INS == 0x03 && commandApduDataBufferLen == 2 &&
        commandAPDU->Lc == 2) {
        //echo command body to response body
        responseApduDataBuffer[0] = commandApduDataBuffer[0];
        responseApduDataBuffer[1] = commandApduDataBuffer[1];

        *responseApduDataBufferLen = 2;

        responseAPDU->SW1 = 0x90;
        responseAPDU->SW2 = 0x00;
    } else {
        responseAPDU->SW1 = 0x6A;
        responseAPDU->SW2 = 0x00;
    }
}

static const Iso7816Callbacks iso87816_cb = {
    iso7816_answer_to_reset,
    iso7816_process_command,
};

int32_t ccid_test_app(void* p) {
    UNUSED(p);

    //setup view
    CcidTestApp* app = ccid_test_app_alloc();

    //setup CCID USB
    // On linux: set VID PID using: /usr/lib/pcsc/drivers/ifd-ccid.bundle/Contents/Info.plist
    app->ccid_cfg.vid = 0x076B;
    app->ccid_cfg.pid = 0x3A21;

    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_unlock();
    furi_hal_ccid_set_callbacks((CcidCallbacks*)&ccid_cb, NULL);
    furi_check(furi_hal_usb_set_config(&usb_ccid, &app->ccid_cfg) == true);

    iso7816_set_callbacks((Iso7816Callbacks*)&iso87816_cb);

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
    furi_hal_usb_set_config(usb_mode_prev, NULL);
    furi_hal_ccid_set_callbacks(NULL, NULL);

    iso7816_set_callbacks(NULL);

    //teardown view
    ccid_test_app_free(app);
    return 0;
}
