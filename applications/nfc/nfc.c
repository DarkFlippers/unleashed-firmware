#include "nfc_i.h"

uint32_t nfc_view_exit(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    NfcMessage message;
    message.type = NfcMessageTypeStop;
    furi_check(osMessageQueuePut(nfc->message_queue, &message, 0, osWaitForever) == osOK);
    return VIEW_NONE;
}

Nfc* nfc_alloc() {
    Nfc* nfc = furi_alloc(sizeof(Nfc));

    nfc->message_queue = osMessageQueueNew(8, sizeof(NfcMessage), NULL);
    nfc->worker = nfc_worker_alloc(nfc->message_queue);

    nfc->icon = assets_icons_get(A_NFC_14);
    nfc->menu_vm = furi_open("menu");
    furi_check(nfc->menu_vm);

    nfc->menu = menu_item_alloc_menu("NFC", nfc->icon);
    menu_item_subitem_add(
        nfc->menu, menu_item_alloc_function("Detect", NULL, nfc_menu_detect_callback, nfc));
    menu_item_subitem_add(
        nfc->menu, menu_item_alloc_function("Emulate", NULL, nfc_menu_emulate_callback, nfc));
    menu_item_subitem_add(
        nfc->menu, menu_item_alloc_function("Field", NULL, nfc_menu_field_callback, nfc));

    nfc->view_detect = view_alloc();
    view_set_context(nfc->view_detect, nfc);
    view_set_draw_callback(nfc->view_detect, nfc_view_read_draw);
    view_set_previous_callback(nfc->view_detect, nfc_view_exit);
    view_allocate_model(nfc->view_detect, ViewModelTypeLocking, sizeof(NfcViewReadModel));
    nfc->view_emulate = view_alloc();
    view_set_context(nfc->view_emulate, nfc);
    view_set_draw_callback(nfc->view_emulate, nfc_view_emulate_draw);
    view_set_previous_callback(nfc->view_emulate, nfc_view_exit);
    nfc->view_field = view_alloc();
    view_set_context(nfc->view_field, nfc);
    view_set_draw_callback(nfc->view_field, nfc_view_field_draw);
    view_set_previous_callback(nfc->view_field, nfc_view_exit);
    nfc->view_error = view_alloc();
    view_set_context(nfc->view_error, nfc);
    view_set_draw_callback(nfc->view_error, nfc_view_error_draw);
    view_set_previous_callback(nfc->view_error, nfc_view_exit);
    view_allocate_model(nfc->view_error, ViewModelTypeLockFree, sizeof(NfcViewErrorModel));
    nfc->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewRead, nfc->view_detect);
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewEmulate, nfc->view_emulate);
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewField, nfc->view_field);
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewError, nfc->view_error);

    return nfc;
}

void nfc_menu_detect_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    NfcMessage message;
    message.type = NfcMessageTypeDetect;
    furi_check(osMessageQueuePut(nfc->message_queue, &message, 0, osWaitForever) == osOK);
}

void nfc_menu_emulate_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    NfcMessage message;
    message.type = NfcMessageTypeEmulate;
    furi_check(osMessageQueuePut(nfc->message_queue, &message, 0, osWaitForever) == osOK);
}

void nfc_menu_field_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    NfcMessage message;
    message.type = NfcMessageTypeField;
    furi_check(osMessageQueuePut(nfc->message_queue, &message, 0, osWaitForever) == osOK);
}

void nfc_menu_field_off_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    NfcMessage message;
    message.type = NfcMessageTypeField;
    furi_check(osMessageQueuePut(nfc->message_queue, &message, 0, osWaitForever) == osOK);
}

void nfc_start(Nfc* nfc, NfcView view_id, NfcWorkerState worker_state) {
    NfcWorkerState state = nfc_worker_get_state(nfc->worker);
    if(state == NfcWorkerStateBroken) {
        with_view_model(
            nfc->view_error,
            (NfcViewErrorModel * model) { model->error = nfc_worker_get_error(nfc->worker); });
        view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewError);
    } else if(state == NfcWorkerStateReady) {
        view_dispatcher_switch_to_view(nfc->view_dispatcher, view_id);
        nfc_worker_start(nfc->worker, worker_state);
    }
}

void nfc_task(void* p) {
    Nfc* nfc = nfc_alloc();

    Gui* gui = furi_open("gui");
    view_dispatcher_attach_to_gui(nfc->view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    with_value_mutex(
        nfc->menu_vm, (Menu * menu) { menu_item_add(menu, nfc->menu); });

    if(!furi_create("nfc", nfc)) {
        printf("[nfc_task] cannot create nfc record\n");
        furiac_exit(NULL);
    }

    furiac_ready();

    NfcMessage message;
    while(1) {
        furi_check(osMessageQueueGet(nfc->message_queue, &message, NULL, osWaitForever) == osOK);
        if(message.type == NfcMessageTypeDetect) {
            with_view_model(
                nfc->view_detect, (NfcViewReadModel * model) { model->found = false; });
            nfc_start(nfc, NfcViewRead, NfcWorkerStatePoll);
        } else if(message.type == NfcMessageTypeEmulate) {
            nfc_start(nfc, NfcViewEmulate, NfcWorkerStateEmulate);
        } else if(message.type == NfcMessageTypeField) {
            nfc_start(nfc, NfcViewField, NfcWorkerStateField);
        } else if(message.type == NfcMessageTypeStop) {
            nfc_worker_stop(nfc->worker);
        } else if(message.type == NfcMessageTypeDeviceFound) {
            with_view_model(
                nfc->view_detect, (NfcViewReadModel * model) {
                    model->found = true;
                    model->device = message.device;
                });
        } else if(message.type == NfcMessageTypeDeviceNotFound) {
            with_view_model(
                nfc->view_detect, (NfcViewReadModel * model) { model->found = false; });
        }
    }
}
