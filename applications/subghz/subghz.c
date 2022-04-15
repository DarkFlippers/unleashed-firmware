/* Abandon hope, all ye who enter here. */

#include "subghz_i.h"
#include <lib/toolbox/path.h>

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

SubGhz* subghz_alloc() {
    SubGhz* subghz = malloc(sizeof(SubGhz));

    // GUI
    subghz->gui = furi_record_open("gui");

    // View Dispatcher
    subghz->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(subghz->view_dispatcher);
    view_dispatcher_attach_to_gui(
        subghz->view_dispatcher, subghz->gui, ViewDispatcherTypeFullscreen);

    subghz->scene_manager = scene_manager_alloc(&subghz_scene_handlers, subghz);
    view_dispatcher_set_event_callback_context(subghz->view_dispatcher, subghz);
    view_dispatcher_set_custom_event_callback(
        subghz->view_dispatcher, subghz_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        subghz->view_dispatcher, subghz_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        subghz->view_dispatcher, subghz_tick_event_callback, 100);

    // Open Notification record
    subghz->notifications = furi_record_open("notification");

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
    subghz->dialogs = furi_record_open("dialogs");

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

    // Carrier Test Module
    subghz->subghz_test_carrier = subghz_test_carrier_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdTestCarrier,
        subghz_test_carrier_get_view(subghz->subghz_test_carrier));

    // Packet Test
    subghz->subghz_test_packet = subghz_test_packet_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdTestPacket,
        subghz_test_packet_get_view(subghz->subghz_test_packet));

    // Static send
    subghz->subghz_test_static = subghz_test_static_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewIdStatic,
        subghz_test_static_get_view(subghz->subghz_test_static));

    //init setting
    subghz->setting = subghz_setting_alloc();
    subghz_setting_load(subghz->setting, "/ext/subghz/assets/setting_user");

    //init Worker & Protocol & History
    subghz->txrx = malloc(sizeof(SubGhzTxRx));
    subghz->txrx->frequency = subghz_setting_get_default_frequency(subghz->setting);
    subghz->txrx->preset = FuriHalSubGhzPresetOok650Async;
    subghz->txrx->txrx_state = SubGhzTxRxStateSleep;
    subghz->txrx->hopper_state = SubGhzHopperStateOFF;
    subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
    subghz->txrx->history = subghz_history_alloc();
    subghz->txrx->worker = subghz_worker_alloc();
    subghz->txrx->fff_data = flipper_format_string_alloc();

    subghz->txrx->environment = subghz_environment_alloc();
    subghz_environment_set_came_atomo_rainbow_table_file_name(
        subghz->txrx->environment, "/ext/subghz/assets/came_atomo");
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        subghz->txrx->environment, "/ext/subghz/assets/nice_flor_s");
    subghz->txrx->receiver = subghz_receiver_alloc_init(subghz->txrx->environment);
    subghz_receiver_set_filter(subghz->txrx->receiver, SubGhzProtocolFlag_Decodable);

    subghz_worker_set_overrun_callback(
        subghz->txrx->worker, (SubGhzWorkerOverrunCallback)subghz_receiver_reset);
    subghz_worker_set_pair_callback(
        subghz->txrx->worker, (SubGhzWorkerPairCallback)subghz_receiver_decode);
    subghz_worker_set_context(subghz->txrx->worker, subghz->txrx->receiver);

    //Init Error_str
    string_init(subghz->error_str);

    return subghz;
}

void subghz_free(SubGhz* subghz) {
    furi_assert(subghz);

    // Packet Test
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdTestPacket);
    subghz_test_packet_free(subghz->subghz_test_packet);

    // Carrier Test
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdTestCarrier);
    subghz_test_carrier_free(subghz->subghz_test_carrier);

    // Static
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewIdStatic);
    subghz_test_static_free(subghz->subghz_test_static);

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
    furi_record_close("dialogs");

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
    furi_record_close("gui");
    subghz->gui = NULL;

    //setting
    subghz_setting_free(subghz->setting);

    //Worker & Protocol & History
    subghz_receiver_free(subghz->txrx->receiver);
    subghz_environment_free(subghz->txrx->environment);
    subghz_worker_free(subghz->txrx->worker);
    flipper_format_free(subghz->txrx->fff_data);
    subghz_history_free(subghz->txrx->history);
    free(subghz->txrx);

    //Error string
    string_clear(subghz->error_str);

    // Notifications
    furi_record_close("notification");
    subghz->notifications = NULL;

    // About birds
    furi_assert(subghz->file_name[SUBGHZ_MAX_LEN_NAME] == 0);
    furi_assert(subghz->file_name_tmp[SUBGHZ_MAX_LEN_NAME] == 0);

    // The rest
    free(subghz);
}

int32_t subghz_app(void* p) {
    SubGhz* subghz = subghz_alloc();

    //Load database
    bool load_database = subghz_environment_load_keystore(
        subghz->txrx->environment, "/ext/subghz/assets/keeloq_mfcodes");
    subghz_environment_load_keystore(
        subghz->txrx->environment, "/ext/subghz/assets/keeloq_mfcodes_user");
    // Check argument and run corresponding scene
    if(p) {
        if(subghz_key_load(subghz, p)) {
            string_t filename;
            string_init(filename);

            path_extract_filename_no_ext(p, filename);
            strncpy(subghz->file_name, string_get_cstr(filename), SUBGHZ_MAX_LEN_NAME);
            string_clear(filename);
            if((!strcmp(subghz->txrx->decoder_result->protocol->name, "RAW"))) {
                //Load Raw TX
                subghz->txrx->rx_key_state = SubGhzRxKeyStateRAWLoad;
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
    } else {
        if(load_database) {
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneStart);
        } else {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneShowError, SubGhzCustomEventManagerSet);
            string_set(
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
