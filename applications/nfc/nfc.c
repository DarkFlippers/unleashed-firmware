#include "nfc.h"

#include <flipper_v2.h>

#include <gui/gui.h>
#include <gui/widget.h>
#include <gui/canvas.h>
#include <assets_icons.h>

#include <menu/menu.h>
#include <menu/menu_item.h>

#include <rfal_analogConfig.h>
#include <rfal_rf.h>
#include <rfal_nfc.h>
#include <rfal_nfca.h>

#include <st25r3916_irq.h>

#include "dispatcher.h"

typedef enum {
    MessageTypeBase,
} NfcMessageType;

typedef struct {
    Message base;
    void* data;
} NfcMessage;

struct Nfc {
    Dispatcher* dispatcher;
    Icon* icon;
    Widget* widget;
    ValueMutex* menu_vm;
    MenuItem* menu;
    rfalNfcDiscoverParam* disParams;

    osThreadAttr_t worker_attr;
    osThreadId_t worker;

    uint8_t screen;
    uint8_t ret;
    uint8_t devCnt;
    uint8_t ticker;

    char* current;
};

#define EXAMPLE_NFCA_DEVICES 5

// TODO replace with pubsub
static bool isr_enabled = false;

void nfc_isr() {
    if(isr_enabled) {
        st25r3916Isr();
    }
}

void nfc_worker_task(void* context) {
    Nfc* nfc = context;
    ReturnCode err;
    rfalNfcaSensRes sensRes;
    rfalNfcaSelRes selRes;
    rfalNfcaListenDevice nfcaDevList[EXAMPLE_NFCA_DEVICES];
    uint8_t devCnt;
    uint8_t devIt;

    rfalLowPowerModeStop();

    nfc->ticker = 0;

    isr_enabled = true;

    while(widget_is_enabled(nfc->widget)) {
        rfalFieldOff();
        platformDelay(1000);
        nfc->ticker += 1;
        nfc->current = "Not detected";
        nfc->devCnt = 0;

        rfalNfcaPollerInitialize();
        rfalFieldOnAndStartGT();
        nfc->ret = err = rfalNfcaPollerTechnologyDetection(RFAL_COMPLIANCE_MODE_NFC, &sensRes);
        if(err == ERR_NONE) {
            err = rfalNfcaPollerFullCollisionResolution(
                RFAL_COMPLIANCE_MODE_NFC, EXAMPLE_NFCA_DEVICES, nfcaDevList, &devCnt);
            nfc->devCnt = devCnt;
            if((err == ERR_NONE) && (devCnt > 0)) {
                platformLog("NFC-A device(s) found %d\r\n", devCnt);
                devIt = 0;
                if(nfcaDevList[devIt].isSleep) {
                    err = rfalNfcaPollerCheckPresence(
                        RFAL_14443A_SHORTFRAME_CMD_WUPA, &sensRes); /* Wake up all cards  */
                    if(err != ERR_NONE) {
                        continue;
                    }
                    err = rfalNfcaPollerSelect(
                        nfcaDevList[devIt].nfcId1,
                        nfcaDevList[devIt].nfcId1Len,
                        &selRes); /* Select specific device  */
                    if(err != ERR_NONE) {
                        continue;
                    }
                }

                switch(nfcaDevList[devIt].type) {
                case RFAL_NFCA_T1T:
                    /* No further activation needed for a T1T (RID already performed)*/
                    platformLog(
                        "NFC-A T1T device found \r\n"); /* NFC-A T1T device found, NFCID/UID is contained in: t1tRidRes.uid */
                    nfc->current = "NFC-A T1T";
                    /* Following communications shall be performed using:
                         *   - Non blocking: rfalStartTransceive() + rfalGetTransceiveState()
                         *   -     Blocking: rfalTransceiveBlockingTx() + rfalTransceiveBlockingRx() or rfalTransceiveBlockingTxRx()    */
                    break;
                case RFAL_NFCA_T2T:
                    /* No specific activation needed for a T2T */
                    platformLog(
                        "NFC-A T2T device found \r\n"); /* NFC-A T2T device found, NFCID/UID is contained in: nfcaDev.nfcid */
                    nfc->current = "NFC-A T2T";
                    /* Following communications shall be performed using:
                         *   - Non blocking: rfalStartTransceive() + rfalGetTransceiveState()
                         *   -     Blocking: rfalTransceiveBlockingTx() + rfalTransceiveBlockingRx() or rfalTransceiveBlockingTxRx()    */
                    break;
                case RFAL_NFCA_T4T:
                    platformLog(
                        "NFC-A T4T (ISO-DEP) device found \r\n"); /* NFC-A T4T device found, NFCID/UID is contained in: nfcaDev.nfcid */
                    nfc->current = "NFC-A T4T";
                    /* Activation should continue using rfalIsoDepPollAHandleActivation(), see exampleRfalPoller.c */
                    break;
                case RFAL_NFCA_T4T_NFCDEP: /* Device supports T4T and NFC-DEP */
                case RFAL_NFCA_NFCDEP: /* Device supports NFC-DEP */
                    platformLog(
                        "NFC-A P2P (NFC-DEP) device found \r\n"); /* NFC-A P2P device found, NFCID/UID is contained in: nfcaDev.nfcid */
                    nfc->current = "NFC-A P2P";
                    /* Activation should continue using rfalNfcDepInitiatorHandleActivation(), see exampleRfalPoller.c */
                    break;
                }
                rfalNfcaPollerSleep(); /* Put device to sleep / HLTA (useless as the field will be turned off anyhow) */
            }
        }
        widget_update(nfc->widget);
    }

    isr_enabled = false;
    rfalFieldOff();
    rfalLowPowerModeStart();
    nfc->ret = ERR_NONE;
    nfc->worker = NULL;
    osThreadExit();
}

void nfc_draw_callback(CanvasApi* canvas, void* context) {
    furi_assert(canvas);
    furi_assert(context);

    Nfc* nfc = context;

    dispatcher_lock(nfc->dispatcher);
    canvas->clear(canvas);
    canvas->set_color(canvas, ColorBlack);
    canvas->set_font(canvas, FontPrimary);

    if(nfc->screen == 0) {
        char status[128 / 8];
        if(nfc->ret == ERR_WRONG_STATE)
            canvas->draw_str(canvas, 2, 16, "NFC Wrong State");
        else if(nfc->ret == ERR_PARAM)
            canvas->draw_str(canvas, 2, 16, "NFC Wrong Param");
        else if(nfc->ret == ERR_IO)
            canvas->draw_str(canvas, 2, 16, "NFC IO Error");
        else if(nfc->ret == ERR_NONE)
            canvas->draw_str(canvas, 2, 16, "NFC Device Found");
        else if(nfc->ret == ERR_TIMEOUT)
            canvas->draw_str(canvas, 2, 16, "NFC Timeout");
        else
            canvas->draw_str(canvas, 2, 16, "NFC error");

        snprintf(status, sizeof(status), "Tck:%d Cnt:%d", nfc->ticker, nfc->devCnt);

        canvas->draw_str(canvas, 2, 32, status);
        canvas->draw_str(canvas, 2, 46, nfc->current);
    } else {
        canvas->draw_str(canvas, 2, 16, "Not implemented");
    }

    dispatcher_unlock(nfc->dispatcher);
}

void nfc_input_callback(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Nfc* nfc = context;

    if(!event->state) return;

    widget_enabled_set(nfc->widget, false);
}

void nfc_test_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;

    dispatcher_lock(nfc->dispatcher);

    nfc->screen = 0;
    widget_enabled_set(nfc->widget, true);

    if(nfc->ret == ERR_NONE && !nfc->worker) {
        // TODO change to fuirac_start
        nfc->worker = osThreadNew(nfc_worker_task, nfc, &nfc->worker_attr);
    }

    dispatcher_unlock(nfc->dispatcher);
}

void nfc_read_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    nfc->screen = 1;
    widget_enabled_set(nfc->widget, true);
}

void nfc_write_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    nfc->screen = 1;
    widget_enabled_set(nfc->widget, true);
}

void nfc_bridge_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    nfc->screen = 1;
    widget_enabled_set(nfc->widget, true);
}

Nfc* nfc_alloc() {
    Nfc* nfc = furi_alloc(sizeof(Nfc));

    nfc->dispatcher = dispatcher_alloc(32, sizeof(NfcMessage));

    nfc->icon = assets_icons_get(A_NFC_14);
    nfc->widget = widget_alloc();
    widget_draw_callback_set(nfc->widget, nfc_draw_callback, nfc);
    widget_input_callback_set(nfc->widget, nfc_input_callback, nfc);

    nfc->menu_vm = furi_open("menu");
    furi_check(nfc->menu_vm);

    nfc->menu = menu_item_alloc_menu("NFC", nfc->icon);
    menu_item_subitem_add(
        nfc->menu, menu_item_alloc_function("Test", NULL, nfc_test_callback, nfc));
    menu_item_subitem_add(
        nfc->menu, menu_item_alloc_function("Read", NULL, nfc_read_callback, nfc));
    menu_item_subitem_add(
        nfc->menu, menu_item_alloc_function("Write", NULL, nfc_write_callback, nfc));
    menu_item_subitem_add(
        nfc->menu, menu_item_alloc_function("Brdige", NULL, nfc_bridge_callback, nfc));

    nfc->worker_attr.name = "nfc_worker";
    // nfc->worker_attr.attr_bits = osThreadJoinable;
    nfc->worker_attr.stack_size = 4096;
    return nfc;
}

void nfc_task(void* p) {
    Nfc* nfc = nfc_alloc();

    FuriRecordSubscriber* gui_record = furi_open_deprecated("gui", false, false, NULL, NULL, NULL);
    furi_check(gui_record);
    GuiApi* gui = furi_take(gui_record);
    furi_check(gui);
    widget_enabled_set(nfc->widget, false);
    gui->add_widget(gui, nfc->widget, GuiLayerFullscreen);
    furi_commit(gui_record);

    with_value_mutex(
        nfc->menu_vm, (Menu * menu) { menu_item_add(menu, nfc->menu); });

    if(!furi_create("nfc", nfc)) {
        printf("[nfc_task] cannot create nfc record\n");
        furiac_exit(NULL);
    }

    nfc->ret = rfalNfcInitialize();
    rfalLowPowerModeStart();

    furiac_ready();

    NfcMessage message;
    while(1) {
        dispatcher_recieve(nfc->dispatcher, (Message*)&message);

        if(message.base.type == MessageTypeExit) {
            break;
        }
    }
}
