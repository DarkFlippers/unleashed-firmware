#include "nfc.h"
#include "nfc_i.h"
#include "nfc_worker.h"

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

        canvas->set_font(canvas, FontSecondary);
        snprintf(status, sizeof(status), "Found: %d", nfc->devCnt);
        if(nfc->devCnt > 0) {
            canvas->draw_str(canvas, 2, 32, status);
            canvas->draw_str(canvas, 2, 42, nfc->current);

            snprintf(
                status,
                sizeof(status),
                "ATQA:%d SAK:%d",
                nfc->first_atqa.anticollisionInfo,
                nfc->first_sak.sak);
            canvas->draw_str(canvas, 2, 52, status);
        }
    } else {
        canvas->draw_str(canvas, 2, 16, "Not implemented");
    }

    dispatcher_unlock(nfc->dispatcher);
}

void nfc_input_callback(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Nfc* nfc = context;

    if(!event->state || event->input != InputBack) return;

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

void nfc_field_on_callback(void* context) {
    st25r3916OscOn();
    st25r3916TxRxOn();
}

void nfc_field_off_callback(void* context) {
    st25r3916TxRxOff();
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
        nfc->menu, menu_item_alloc_function("Field On", NULL, nfc_field_on_callback, nfc));
    menu_item_subitem_add(
        nfc->menu, menu_item_alloc_function("Field Off", NULL, nfc_field_off_callback, nfc));
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
