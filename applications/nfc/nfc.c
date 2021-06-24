#include "nfc_i.h"
#include "api-hal-nfc.h"

uint32_t nfc_view_exit(void* context) {
    return VIEW_NONE;
}

void nfc_menu_callback(void* context, uint32_t index) {
    furi_assert(context);

    Nfc* nfc = (Nfc*)context;
    if(index == NfcSubmenuDetect) {
        view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewDetect);
    } else if(index == NfcSubmenuEmulate) {
        view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewEmulate);
    } else if(index == NfcSubmenuEMV) {
        view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewEmv);
    } else if(index == NfcSubmenuMifareUl) {
        view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewMifareUl);
    }
}

void nfc_view_dispatcher_callback(uint32_t event, void* context) {
    furi_assert(context);

    Nfc* nfc = (Nfc*)context;
    NfcMessage message;
    osMessageQueueGet(nfc->message_queue, &message, NULL, osWaitForever);
    if(event == NfcEventDetect) {
        nfc_detect_view_dispatcher_callback(nfc->nfc_detect, &message);
    } else if(event == NfcEventEmv) {
        nfc_emv_view_dispatcher_callback(nfc->nfc_emv, &message);
    } else if(event == NfcEventMifareUl) {
        nfc_mifare_ul_view_dispatcher_callback(nfc->nfc_mifare_ul, &message);
    }
}

Nfc* nfc_alloc() {
    Nfc* nfc = furi_alloc(sizeof(Nfc));

    nfc->message_queue = osMessageQueueNew(8, sizeof(NfcMessage), NULL);
    nfc->nfc_common.worker = nfc_worker_alloc(nfc->message_queue);
    nfc->nfc_common.view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(nfc->nfc_common.view_dispatcher);

    // Open GUI record
    nfc->gui = furi_record_open("gui");
    view_dispatcher_attach_to_gui(
        nfc->nfc_common.view_dispatcher, nfc->gui, ViewDispatcherTypeFullscreen);

    // Menu
    nfc->submenu = submenu_alloc();
    submenu_add_item(nfc->submenu, "Detect", NfcSubmenuDetect, nfc_menu_callback, nfc);
    submenu_add_item(nfc->submenu, "Emulate", NfcSubmenuEmulate, nfc_menu_callback, nfc);
    submenu_add_item(nfc->submenu, "Read bank card", NfcSubmenuEMV, nfc_menu_callback, nfc);
    submenu_add_item(
        nfc->submenu, "Read Mifare Ultralight", NfcSubmenuMifareUl, nfc_menu_callback, nfc);

    View* submenu_view = submenu_get_view(nfc->submenu);
    view_set_previous_callback(submenu_view, nfc_view_exit);
    view_dispatcher_add_view(nfc->nfc_common.view_dispatcher, NfcViewMenu, submenu_view);

    // Detect
    nfc->nfc_detect = nfc_detect_alloc(&nfc->nfc_common);
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewDetect, nfc_detect_get_view(nfc->nfc_detect));

    // Emulate
    nfc->nfc_emulate = nfc_emulate_alloc(&nfc->nfc_common);
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewEmulate, nfc_emulate_get_view(nfc->nfc_emulate));

    // EMV
    nfc->nfc_emv = nfc_emv_alloc(&nfc->nfc_common);
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher, NfcViewEmv, nfc_emv_get_view(nfc->nfc_emv));

    // Mifare Ultralight
    nfc->nfc_mifare_ul = nfc_mifare_ul_alloc(&nfc->nfc_common);
    view_dispatcher_add_view(
        nfc->nfc_common.view_dispatcher,
        NfcViewMifareUl,
        nfc_mifare_ul_get_view(nfc->nfc_mifare_ul));

    // Set View Dispatcher custom event callback
    view_dispatcher_set_custom_callback(
        nfc->nfc_common.view_dispatcher, nfc_view_dispatcher_callback, nfc);

    // Switch to menu
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewMenu);

    return nfc;
}

void nfc_free(Nfc* nfc) {
    furi_assert(nfc);

    // Submenu
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewMenu);
    submenu_free(nfc->submenu);

    // Detect
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewDetect);
    nfc_detect_free(nfc->nfc_detect);

    // Emulate
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewEmulate);
    nfc_emulate_free(nfc->nfc_emulate);

    // EMV
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewEmv);
    nfc_emv_free(nfc->nfc_emv);

    // Mifare ultralight
    view_dispatcher_remove_view(nfc->nfc_common.view_dispatcher, NfcViewMifareUl);
    nfc_mifare_ul_free(nfc->nfc_mifare_ul);

    // Worker
    nfc_worker_stop(nfc->nfc_common.worker);
    nfc_worker_free(nfc->nfc_common.worker);

    // View dispatcher
    view_dispatcher_free(nfc->nfc_common.view_dispatcher);

    // GUI
    furi_record_close("gui");
    nfc->gui = NULL;

    // The rest
    osMessageQueueDelete(nfc->message_queue);
    free(nfc);
}

int32_t nfc_task(void* p) {
    Nfc* nfc = nfc_alloc();

    view_dispatcher_run(nfc->nfc_common.view_dispatcher);

    nfc_free(nfc);

    return 0;
}
