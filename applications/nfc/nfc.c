#include "nfc_i.h"
#include "api-hal-nfc.h"

osMessageQueueId_t message_queue = NULL;

uint32_t nfc_view_stop(void* context) {
    furi_assert(message_queue);
    NfcMessage message;
    message.type = NfcMessageTypeStop;
    furi_check(osMessageQueuePut(message_queue, &message, 0, osWaitForever) == osOK);
    return NfcViewMenu;
}

uint32_t nfc_view_exit(void* context) {
    furi_assert(message_queue);
    NfcMessage message;
    message.type = NfcMessageTypeExit;
    furi_check(osMessageQueuePut(message_queue, &message, 0, osWaitForever) == osOK);
    return VIEW_NONE;
}

void nfc_menu_callback(void* context, uint32_t index) {
    furi_assert(message_queue);
    NfcMessage message;
    if(index == 0) {
        message.type = NfcMessageTypeDetect;
    } else if(index == 1) {
        message.type = NfcMessageTypeReadEMV;
    } else if(index == 2) {
        message.type = NfcMessageTypeEmulateEMV;
    } else if(index == 3) {
        message.type = NfcMessageTypeEmulate;
    } else if(index == 4) {
        message.type = NfcMessageTypeField;
    }
    furi_check(osMessageQueuePut(message_queue, &message, 0, osWaitForever) == osOK);
}

Nfc* nfc_alloc() {
    Nfc* nfc = furi_alloc(sizeof(Nfc));

    message_queue = osMessageQueueNew(8, sizeof(NfcMessage), NULL);

    nfc->worker = nfc_worker_alloc(message_queue);

    // Open GUI record
    nfc->gui = furi_record_open("gui");

    // View Dispatcher
    nfc->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(nfc->view_dispatcher, nfc->gui, ViewDispatcherTypeFullscreen);

    // Menu
    nfc->submenu = submenu_alloc();
    submenu_add_item(nfc->submenu, "Detect", 0, nfc_menu_callback, nfc);
    submenu_add_item(nfc->submenu, "Read EMV", 1, nfc_menu_callback, nfc);
    submenu_add_item(nfc->submenu, "Emulate EMV", 2, nfc_menu_callback, nfc);
    submenu_add_item(nfc->submenu, "Emulate", 3, nfc_menu_callback, nfc);
    submenu_add_item(nfc->submenu, "Field", 4, nfc_menu_callback, nfc);
    View* submenu_view = submenu_get_view(nfc->submenu);
    view_set_previous_callback(submenu_view, nfc_view_exit);
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewMenu, submenu_view);

    // Detect
    nfc->view_detect = view_alloc();
    view_set_context(nfc->view_detect, nfc);
    view_set_draw_callback(nfc->view_detect, nfc_view_read_draw);
    view_set_previous_callback(nfc->view_detect, nfc_view_stop);
    view_allocate_model(nfc->view_detect, ViewModelTypeLocking, sizeof(NfcViewReadModel));
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewRead, nfc->view_detect);

    // Read EMV
    nfc->view_read_emv = view_alloc();
    view_set_context(nfc->view_read_emv, nfc);
    view_set_draw_callback(nfc->view_read_emv, nfc_view_read_emv_draw);
    view_set_previous_callback(nfc->view_read_emv, nfc_view_stop);
    view_allocate_model(nfc->view_read_emv, ViewModelTypeLocking, sizeof(NfcViewReadModel));
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewReadEmv, nfc->view_read_emv);

    // Emulate EMV
    nfc->view_emulate_emv = view_alloc();
    view_set_context(nfc->view_emulate_emv, nfc);
    view_set_draw_callback(nfc->view_emulate_emv, nfc_view_emulate_emv_draw);
    view_set_previous_callback(nfc->view_emulate_emv, nfc_view_stop);
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewEmulateEMV, nfc->view_emulate_emv);

    // Emulate
    nfc->view_emulate = view_alloc();
    view_set_context(nfc->view_emulate, nfc);
    view_set_draw_callback(nfc->view_emulate, nfc_view_emulate_draw);
    view_set_previous_callback(nfc->view_emulate, nfc_view_stop);
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewEmulate, nfc->view_emulate);

    // Field
    nfc->view_field = view_alloc();
    view_set_context(nfc->view_field, nfc);
    view_set_draw_callback(nfc->view_field, nfc_view_field_draw);
    view_set_previous_callback(nfc->view_field, nfc_view_stop);
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewField, nfc->view_field);

    // Error
    nfc->view_error = view_alloc();
    view_set_context(nfc->view_error, nfc);
    view_set_draw_callback(nfc->view_error, nfc_view_error_draw);
    view_set_previous_callback(nfc->view_error, nfc_view_stop);
    view_allocate_model(nfc->view_error, ViewModelTypeLockFree, sizeof(NfcViewErrorModel));
    view_dispatcher_add_view(nfc->view_dispatcher, NfcViewError, nfc->view_error);

    // Switch to menu
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);

    return nfc;
}

void nfc_free(Nfc* nfc) {
    // Free nfc worker
    nfc_worker_free(nfc->worker);
    // Free allocated queue
    osMessageQueueDelete(message_queue);
    message_queue = NULL;

    // Free allocated views
    // Menu
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewMenu);
    submenu_free(nfc->submenu);

    // Detect
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewRead);
    view_free(nfc->view_detect);

    // Read EMV
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewReadEmv);
    view_free(nfc->view_read_emv);

    // Emulate EMV
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewEmulateEMV);
    view_free(nfc->view_emulate_emv);

    // Emulate
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewEmulate);
    view_free(nfc->view_emulate);

    // Field
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewField);
    view_free(nfc->view_field);

    // Error
    view_dispatcher_remove_view(nfc->view_dispatcher, NfcViewError);
    view_free(nfc->view_error);

    // Free View Dispatcher
    view_dispatcher_free(nfc->view_dispatcher);

    // Close all opened records
    furi_record_close("gui");
    nfc->gui = NULL;

    // Free nfc object
    free(nfc);
}

void nfc_cli_detect(Cli* cli, string_t args, void* context) {
    // Check if nfc worker is not busy
    if(api_hal_nfc_is_busy()) {
        printf("Nfc is busy");
        return;
    }
    rfalNfcDevice* dev_list;
    uint8_t dev_cnt = 0;
    bool cmd_exit = false;
    api_hal_nfc_init();
    printf("Detecting nfc...\r\nPress Ctrl+C to abort\r\n");
    while(!cmd_exit) {
        cmd_exit |= cli_cmd_interrupt_received(cli);
        cmd_exit |= api_hal_nfc_detect(&dev_list, &dev_cnt, 100, true);
        if(dev_cnt > 0) {
            printf("Found %d devices\r\n", dev_cnt);
            for(uint8_t i = 0; i < dev_cnt; i++) {
                printf("%d found: %s ", i, nfc_get_dev_type(dev_list[i].type));
                if(dev_list[i].type == RFAL_NFC_LISTEN_TYPE_NFCA) {
                    printf("type: %s, ", nfc_get_nfca_type(dev_list[i].dev.nfca.type));
                }
                printf("UID length: %d, UID:", dev_list[i].nfcidLen);
                for(uint8_t j = 0; j < dev_list[i].nfcidLen; j++) {
                    printf("%02X", dev_list[i].nfcid[j]);
                }
                printf("\r\n");
            }
        }
        osDelay(50);
    }
}

void nfc_cli_init() {
    Cli* cli = furi_record_open("cli");
    cli_add_command(cli, "nfc_detect", nfc_cli_detect, NULL);
    furi_record_close("cli");
}

void nfc_start(Nfc* nfc, NfcView view_id, NfcWorkerState worker_state) {
    NfcWorkerState state = nfc_worker_get_state(nfc->worker);
    if(state == NfcWorkerStateBroken) {
        with_view_model(
            nfc->view_error, (NfcViewErrorModel * model) {
                model->error = nfc_worker_get_error(nfc->worker);
                return true;
            });
        view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewError);
    } else if(state == NfcWorkerStateReady) {
        view_dispatcher_switch_to_view(nfc->view_dispatcher, view_id);
        nfc_worker_start(nfc->worker, worker_state);
    }
}

int32_t nfc_task(void* p) {
    Nfc* nfc = nfc_alloc();

    NfcMessage message;
    while(1) {
        furi_check(osMessageQueueGet(message_queue, &message, NULL, osWaitForever) == osOK);
        if(message.type == NfcMessageTypeDetect) {
            with_view_model(
                nfc->view_detect, (NfcViewReadModel * model) {
                    model->found = false;
                    return true;
                });
            nfc_start(nfc, NfcViewRead, NfcWorkerStatePoll);
        } else if(message.type == NfcMessageTypeReadEMV) {
            with_view_model(
                nfc->view_read_emv, (NfcViewReadModel * model) {
                    model->found = false;
                    return true;
                });
            nfc_start(nfc, NfcViewReadEmv, NfcWorkerStateReadEMV);
        } else if(message.type == NfcMessageTypeEmulateEMV) {
            nfc_start(nfc, NfcViewEmulateEMV, NfcWorkerStateEmulateEMV);
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
                    return true;
                });
        } else if(message.type == NfcMessageTypeDeviceNotFound) {
            with_view_model(
                nfc->view_detect, (NfcViewReadModel * model) {
                    model->found = false;
                    return true;
                });
        } else if(message.type == NfcMessageTypeEMVFound) {
            with_view_model(
                nfc->view_read_emv, (NfcViewReadModel * model) {
                    model->found = true;
                    model->device = message.device;
                    return true;
                });
        } else if(message.type == NfcMessageTypeEMVNotFound) {
            with_view_model(
                nfc->view_read_emv, (NfcViewReadModel * model) {
                    model->found = false;
                    return true;
                });
        } else if(message.type == NfcMessageTypeExit) {
            nfc_free(nfc);
            break;
        }
    }

    return 0;
}
