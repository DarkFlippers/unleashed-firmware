/* Abandon hope, all ye who enter here. */

#include "subghz_i.h"

bool subghz_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SubGhz* subghz = context;
    return scene_manager_handle_custom_event(subghz->scene_manager, event);
}

bool subghz_back_event_callback(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    return scene_manager_handle_back_event(subghz->scene_manager);
}

void subghz_tick_event_callback(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    scene_manager_handle_tick_event(subghz->scene_manager);
}

static void subghz_rpc_command_callback(const RpcAppSystemEvent* event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;

    furi_assert(subghz->rpc_ctx);

    if(event->type == RpcAppEventTypeSessionClose) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneRpcSessionClose);
        rpc_system_app_set_callback(subghz->rpc_ctx, NULL, NULL);
        subghz->rpc_ctx = NULL;
    } else if(event->type == RpcAppEventTypeAppExit) {
        view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneExit);
    } else if(event->type == RpcAppEventTypeLoadFile) {
        furi_assert(event->data.type == RpcAppSystemEventDataTypeString);
        furi_string_set(subghz->file_path, event->data.string);
        view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneRpcLoad);
    } else if(event->type == RpcAppEventTypeButtonPress) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneRpcButtonPress);
    } else if(event->type == RpcAppEventTypeButtonRelease) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneRpcButtonRelease);
    } else {
        rpc_system_app_confirm(subghz->rpc_ctx, false);
    }
}

SubGhz* subghz_alloc() {
    SubGhz* subghz = malloc(sizeof(SubGhz));

    subghz->file_path = furi_string_alloc();
    subghz->file_path_tmp = furi_string_alloc();

    // GUI
    subghz->gui = furi_record_open(RECORD_GUI);

    // View Dispatcher
    subghz->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(subghz->view_dispatcher);

    subghz->scene_manager = scene_manager_alloc(&subghz_scene_handlers, subghz);
    view_dispatcher_set_event_callback_context(subghz->view_dispatcher, subghz);
    view_dispatcher_set_custom_event_callback(
        subghz->view_dispatcher, subghz_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        subghz->view_dispatcher, subghz_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        subghz->view_dispatcher, subghz_tick_event_callback, 100);

    // Open Notification record
    subghz->notifications = furi_record_open(RECORD_NOTIFICATION);

    // SubMenu
    subghz->submenu = submenu_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewIdMenu, submenu_get_view(subghz->submenu));

    // Receiver
    subghz->subghz_receiver = subghz_view_receiver_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdReceiver,
        subghz_view_receiver_get_view(subghz->subghz_receiver));

    // Popup
    subghz->popup = popup_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewIdPopup, popup_get_view(subghz->popup));

    // Text Input
    subghz->text_input = text_input_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewIdTextInput, text_input_get_view(subghz->text_input));

    // Custom Widget
    subghz->widget = widget_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewIdWidget, widget_get_view(subghz->widget));

    //Dialog
    subghz->dialogs = furi_record_open(RECORD_DIALOGS);

    // Transmitter
    subghz->subghz_transmitter = subghz_view_transmitter_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdTransmitter,
        subghz_view_transmitter_get_view(subghz->subghz_transmitter));

    // Variable Item List
    subghz->variable_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdVariableItemList,
        variable_item_list_get_view(subghz->variable_item_list));

    // Frequency Analyzer
    subghz->subghz_frequency_analyzer = subghz_frequency_analyzer_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdFrequencyAnalyzer,
        subghz_frequency_analyzer_get_view(subghz->subghz_frequency_analyzer));

    // Read RAW
    subghz->subghz_read_raw = subghz_read_raw_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdReadRAW,
        subghz_read_raw_get_view(subghz->subghz_read_raw));

    //init threshold rssi
    subghz->threshold_rssi = subghz_threshold_rssi_alloc();

    subghz_unlock(subghz);
    subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);
    subghz->history = subghz_history_alloc();
    subghz->filter = SubGhzProtocolFlag_Decodable;

    //init TxRx & History & KeyBoard
    subghz->txrx = subghz_txrx_alloc();
    subghz_txrx_receiver_set_filter(subghz->txrx, subghz->filter);
    subghz_txrx_set_need_save_callback(subghz->txrx, subghz_save_to_file, subghz);

    //Init Error_str
    subghz->error_str = furi_string_alloc();

    return subghz;
}

void subghz_free(SubGhz* subghz) {
    furi_assert(subghz);

    if(subghz->rpc_ctx) {
        rpc_system_app_set_callback(subghz->rpc_ctx, NULL, NULL);
        rpc_system_app_send_exited(subghz->rpc_ctx);
        subghz_blink_stop(subghz);
        subghz->rpc_ctx = NULL;
    }

    subghz_txrx_speaker_off(subghz->txrx);
    subghz_txrx_stop(subghz->txrx);
    subghz_txrx_sleep(subghz->txrx);

    // Receiver
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdReceiver);
    subghz_view_receiver_free(subghz->subghz_receiver);

    // TextInput
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdTextInput);
    text_input_free(subghz->text_input);

    // Custom Widget
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdWidget);
    widget_free(subghz->widget);

    //Dialog
    furi_record_close(RECORD_DIALOGS);

    // Transmitter
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdTransmitter);
    subghz_view_transmitter_free(subghz->subghz_transmitter);

    // Variable Item List
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdVariableItemList);
    variable_item_list_free(subghz->variable_item_list);

    // Frequency Analyzer
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdFrequencyAnalyzer);
    subghz_frequency_analyzer_free(subghz->subghz_frequency_analyzer);

    // Read RAW
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdReadRAW);
    subghz_read_raw_free(subghz->subghz_read_raw);

    // Submenu
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdMenu);
    submenu_free(subghz->submenu);

    // Popup
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdPopup);
    popup_free(subghz->popup);

    // Scene manager
    scene_manager_free(subghz->scene_manager);

    // View Dispatcher
    view_dispatcher_free(subghz->view_dispatcher);

    // GUI
    furi_record_close(RECORD_GUI);
    subghz->gui = NULL;

    // threshold rssi
    subghz_threshold_rssi_free(subghz->threshold_rssi);

    //Worker & Protocol & History
    subghz_history_free(subghz->history);

    //TxRx
    subghz_txrx_free(subghz->txrx);

    //Error string
    furi_string_free(subghz->error_str);

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    subghz->notifications = NULL;

    // Path strings
    furi_string_free(subghz->file_path);
    furi_string_free(subghz->file_path_tmp);

    // The rest
    free(subghz);
}

int32_t subghz_app(void* p) {
    SubGhz* subghz = subghz_alloc();

    if(!furi_hal_region_is_provisioned()) {
        subghz_dialog_message_show_only_rx(subghz);
        subghz_free(subghz);
        return 1;
    }

    // Check argument and run corresponding scene
    if(p && strlen(p)) {
        uint32_t rpc_ctx = 0;
        if(sscanf(p, "RPC %lX", &rpc_ctx) == 1) {
            subghz->rpc_ctx = (void*)rpc_ctx;
            rpc_system_app_set_callback(subghz->rpc_ctx, subghz_rpc_command_callback, subghz);
            rpc_system_app_send_started(subghz->rpc_ctx);
            view_dispatcher_attach_to_gui(
                subghz->view_dispatcher, subghz->gui, ViewDispatcherTypeDesktop);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneRpc);
        } else {
            view_dispatcher_attach_to_gui(
                subghz->view_dispatcher, subghz->gui, ViewDispatcherTypeFullscreen);
            if(subghz_key_load(subghz, p, true)) {
                furi_string_set(subghz->file_path, (const char*)p);

                if(subghz_get_load_type_file(subghz) == SubGhzLoadTypeFileRaw) {
                    //Load Raw TX
                    subghz_rx_key_state_set(subghz, SubGhzRxKeyStateRAWLoad);
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReadRAW);
                } else {
                    //Load transmitter TX
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTransmitter);
                }
            } else {
                //exit app
                scene_manager_stop(subghz->scene_manager);
                view_dispatcher_stop(subghz->view_dispatcher);
            }
        }
    } else {
        view_dispatcher_attach_to_gui(
            subghz->view_dispatcher, subghz->gui, ViewDispatcherTypeFullscreen);
        furi_string_set(subghz->file_path, SUBGHZ_APP_FOLDER);
        if(subghz_txrx_is_database_loaded(subghz->txrx)) {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneStart);
        } else {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneShowError, SubGhzCustomEventManagerSet);
            furi_string_set(
                subghz->error_str,
                "No SD card or\ndatabase found.\nSome app function\nmay be reduced.");
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
        }
    }

    furi_hal_power_suppress_charge_enter();

    view_dispatcher_run(subghz->view_dispatcher);

    furi_hal_power_suppress_charge_exit();

    subghz_free(subghz);

    return 0;
}
