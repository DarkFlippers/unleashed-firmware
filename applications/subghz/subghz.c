#include "subghz_i.h"

const uint32_t subghz_frequencies[] = {
    /* 300 - 348 */
    300000000,
    315000000,
    348000000,
    /* 387 - 464 */
    387000000,
    433075000, /* LPD433 first */
    433920000, /* LPD433 mid */
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

const uint32_t subghz_frequencies_count = sizeof(subghz_frequencies) / sizeof(uint32_t);
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

    // Dialog
    subghz->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewDialogEx, dialog_ex_get_view(subghz->dialog_ex));

    // Popup
    subghz->popup = popup_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewPopup, popup_get_view(subghz->popup));

    // Text Input
    subghz->text_input = text_input_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher, SubGhzViewTextInput, text_input_get_view(subghz->text_input));

    // Transmitter
    subghz->subghz_transmitter = subghz_transmitter_alloc();
    view_dispatcher_add_view(
        subghz->view_dispatcher,
        SubGhzViewTransmitter,
        subghz_transmitter_get_view(subghz->subghz_transmitter));

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

    //init Worker & Protocol
    subghz->worker = subghz_worker_alloc();
    subghz->protocol = subghz_protocol_alloc();
    subghz_worker_set_overrun_callback(
        subghz->worker, (SubGhzWorkerOverrunCallback)subghz_protocol_reset);
    subghz_worker_set_pair_callback(
        subghz->worker, (SubGhzWorkerPairCallback)subghz_protocol_parse);
    subghz_worker_set_context(subghz->worker, subghz->protocol);

    subghz_protocol_load_keeloq_file(subghz->protocol, "/ext/assets/subghz/keeloq_mfcodes");
    subghz_protocol_load_nice_flor_s_file(subghz->protocol, "/ext/assets/subghz/nice_floor_s_rx");

    //subghz_protocol_enable_dump_text(subghz->protocol, subghz_text_callback, subghz);

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

    // Receiver
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewTransmitter);
    subghz_transmitter_free(subghz->subghz_transmitter);

    // Submenu
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewMenu);
    submenu_free(subghz->submenu);

    // DialogEx
    view_dispatcher_remove_view(subghz->view_dispatcher, SubGhzViewDialogEx);
    dialog_ex_free(subghz->dialog_ex);

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

    //Worker & Protocol
    subghz_protocol_free(subghz->protocol);
    subghz_worker_free(subghz->worker);

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
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTransmitter);
    } else {
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneStart);
    }

    view_dispatcher_run(subghz->view_dispatcher);

    subghz_free(subghz);

    return 0;
}