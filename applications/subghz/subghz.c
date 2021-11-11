#include "subghz_i.h"
#include <lib/toolbox/path.h>

const char* const subghz_frequencies_text[] = {
    "300.00",
    "315.00",
    "348.00",
    "387.00",
    "433.08",
    "433.92",
    "434.42",
    "434.78",
    "438.90",
    "464.00",
    "779.00",
    "868.35",
    "915.00",
    "925.00",
    "928.00",
};

const uint32_t subghz_frequencies[] = {
    /* 300 - 348 */
    300000000,
    315000000,
    348000000,
    /* 387 - 464 */
    387000000,
    433075000, /* LPD433 first */
    433920000, /* LPD433 mid */
    434420000,
    434775000, /* LPD433 last channels */
    438900000,
    464000000,
    /* 779 - 928 */
    779000000,
    868350000,
    915000000,
    925000000,
    928000000,
};

const uint32_t subghz_hopper_frequencies[] = {
    315000000,
    433920000,
    868350000,
};

const uint32_t subghz_frequencies_count = sizeof(subghz_frequencies) / sizeof(uint32_t);
const uint32_t subghz_hopper_frequencies_count =
    sizeof(subghz_hopper_frequencies) / sizeof(uint32_t);
const uint32_t subghz_frequencies_433_92 = 5;

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
    SubGhz* subghz = furi_alloc(sizeof(SubGhz));

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
        subghz->view_dispatcher, SubGhzViewMenu, submenu_get_view(subghz->submenu));

    // Receiver
    subghz->subghz_receiver = subghz_receiver_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewReceiver,
        subghz_receiver_get_view(subghz->subghz_receiver));

    // Popup
    subghz->popup = popup_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewPopup, popup_get_view(subghz->popup));

    // Text Input
    subghz->text_input = text_input_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewTextInput, text_input_get_view(subghz->text_input));

    // Custom Widget
    subghz->widget = widget_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewWidget, widget_get_view(subghz->widget));

    //Dialog
    subghz->dialogs = furi_record_open("dialogs");

    // Transmitter
    subghz->subghz_transmitter = subghz_transmitter_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewTransmitter,
        subghz_transmitter_get_view(subghz->subghz_transmitter));

    // Variable Item List
    subghz->variable_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewVariableItemList,
        variable_item_list_get_view(subghz->variable_item_list));

    // Frequency Analyzer
    subghz->subghz_frequency_analyzer = subghz_frequency_analyzer_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewFrequencyAnalyzer,
        subghz_frequency_analyzer_get_view(subghz->subghz_frequency_analyzer));

    // Read RAW
    subghz->subghz_read_raw = subghz_read_raw_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewReadRAW,
        subghz_read_raw_get_view(subghz->subghz_read_raw));

    // Carrier Test Module
    subghz->subghz_test_carrier = subghz_test_carrier_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewTestCarrier,
        subghz_test_carrier_get_view(subghz->subghz_test_carrier));

    // Packet Test
    subghz->subghz_test_packet = subghz_test_packet_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewTestPacket,
        subghz_test_packet_get_view(subghz->subghz_test_packet));

    // Static send
    subghz->subghz_test_static = subghz_test_static_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewStatic,
        subghz_test_static_get_view(subghz->subghz_test_static));

    //init Worker & Protocol & History
    subghz->txrx = furi_alloc(sizeof(SubGhzTxRx));
    subghz->txrx->frequency = subghz_frequencies[subghz_frequencies_433_92];
    subghz->txrx->preset = FuriHalSubGhzPresetOok650Async;
    subghz->txrx->txrx_state = SubGhzTxRxStateSleep;
    subghz->txrx->hopper_state = SubGhzHopperStateOFF;
    subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
    subghz->txrx->history = subghz_history_alloc();
    subghz->txrx->worker = subghz_worker_alloc();
    subghz->txrx->parser = subghz_parser_alloc();
    subghz_worker_set_overrun_callback(
        subghz->txrx->worker, (SubGhzWorkerOverrunCallback)subghz_parser_reset);
    subghz_worker_set_pair_callback(
        subghz->txrx->worker, (SubGhzWorkerPairCallback)subghz_parser_parse);
    subghz_worker_set_context(subghz->txrx->worker, subghz->txrx->parser);

    //Init Error_str
    string_init(subghz->error_str);

    subghz_parser_load_keeloq_file(subghz->txrx->parser, "/ext/subghz/keeloq_mfcodes");
    subghz_parser_load_keeloq_file(subghz->txrx->parser, "/ext/subghz/keeloq_mfcodes_user");
    subghz_parser_load_nice_flor_s_file(subghz->txrx->parser, "/ext/subghz/nice_flor_s_rx");
    subghz_parser_load_came_atomo_file(subghz->txrx->parser, "/ext/subghz/came_atomo");

    //subghz_parser_enable_dump_text(subghz->protocol, subghz_text_callback, subghz);

    return subghz;
}

void subghz_free(SubGhz* subghz) {
    furi_assert(subghz);

    // Packet Test
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewTestPacket);
    subghz_test_packet_free(subghz->subghz_test_packet);

    // Carrier Test
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewTestCarrier);
    subghz_test_carrier_free(subghz->subghz_test_carrier);

    // Static
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewStatic);
    subghz_test_static_free(subghz->subghz_test_static);

    // Receiver
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewReceiver);
    subghz_receiver_free(subghz->subghz_receiver);

    // TextInput
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewTextInput);
    text_input_free(subghz->text_input);

    // Custom Widget
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewWidget);
    widget_free(subghz->widget);

    //Dialog
    furi_record_close("dialogs");

    // Transmitter
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewTransmitter);
    subghz_transmitter_free(subghz->subghz_transmitter);

    // Variable Item List
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewVariableItemList);
    variable_item_list_free(subghz->variable_item_list);

    // Frequency Analyzer
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewFrequencyAnalyzer);
    subghz_frequency_analyzer_free(subghz->subghz_frequency_analyzer);

    // Read RAW
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewReadRAW);
    subghz_read_raw_free(subghz->subghz_read_raw);

    // Submenu
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewMenu);
    submenu_free(subghz->submenu);

    // Popup
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewPopup);
    popup_free(subghz->popup);

    // Scene manager
    scene_manager_free(subghz->scene_manager);

    // View Dispatcher
    view_dispatcher_free(subghz->view_dispatcher);

    // GUI
    furi_record_close("gui");
    subghz->gui = NULL;

    //Worker & Protocol & History
    subghz_parser_free(subghz->txrx->parser);
    subghz_worker_free(subghz->txrx->worker);
    subghz_history_free(subghz->txrx->history);
    free(subghz->txrx);

    //Error string
    string_clear(subghz->error_str);

    // Notifications
    furi_record_close("notification");
    subghz->notifications = NULL;

    // The rest
    free(subghz);
}

int32_t subghz_app(void* p) {
    SubGhz* subghz = subghz_alloc();

    // Check argument and run corresponding scene
    if(p && subghz_key_load(subghz, p)) {
        string_t filename;
        path_extract_filename_no_ext(p, filename);
        strlcpy(
            subghz->file_name, string_get_cstr(filename), strlen(string_get_cstr(filename)) + 1);
        string_clear(filename);

        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTransmitter);
    } else {
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneStart);
    }

    furi_hal_power_suppress_charge_enter();

    view_dispatcher_run(subghz->view_dispatcher);

    furi_hal_power_suppress_charge_exit();

    subghz_free(subghz);

    return 0;
}