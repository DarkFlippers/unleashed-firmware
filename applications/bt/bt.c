#include "bt_i.h"

uint32_t bt_view_exit(void* context) {
    (void)context;
    return VIEW_NONE;
}

void bt_update_statusbar(void* arg) {
    furi_assert(arg);
    Bt* bt = arg;
    BtMessage m = {.type = BtMessageTypeUpdateStatusbar};
    furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
}

void bt_switch_freq(void* arg) {
    furi_assert(arg);
    Bt* bt = arg;
    BtMessage m = {.type = BtMessageTypeStartTestToneTx};
    furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
}

Bt* bt_alloc() {
    Bt* bt = furi_alloc(sizeof(Bt));

    bt->message_queue = osMessageQueueNew(8, sizeof(BtMessage), NULL);
    bt->update_status_timer = osTimerNew(bt_update_statusbar, osTimerPeriodic, bt, NULL);
    osTimerStart(bt->update_status_timer, 4000);
    bt->hopping_mode_timer = osTimerNew(bt_switch_freq, osTimerPeriodic, bt, NULL);
    bt->gui = furi_record_open("gui");
    bt->menu = furi_record_open("menu");

    bt->state.type = BtStatusReady;
    bt->state.param.channel = BtChannel2402;
    bt->state.param.power = BtPower0dB;
    bt->state.param.datarate = BtDateRate1M;

    bt->statusbar_view_port = view_port_alloc();
    view_port_set_width(bt->statusbar_view_port, 5);
    view_port_draw_callback_set(bt->statusbar_view_port, bt_draw_statusbar_callback, bt);
    view_port_enabled_set(bt->statusbar_view_port, false);
    gui_add_view_port(bt->gui, bt->statusbar_view_port, GuiLayerStatusBarLeft);

    bt->menu_icon = assets_icons_get(A_Bluetooth_14);
    bt->menu_item = menu_item_alloc_menu("Bluetooth", bt->menu_icon);
    menu_item_subitem_add(
        bt->menu_item, menu_item_alloc_function("Test tone TX", NULL, bt_menu_test_tone_tx, bt));
    menu_item_subitem_add(
        bt->menu_item,
        menu_item_alloc_function("Test packet TX", NULL, bt_menu_test_packet_tx, bt));
    menu_item_subitem_add(
        bt->menu_item, menu_item_alloc_function("Test tone RX", NULL, bt_menu_test_tone_rx, bt));
    menu_item_subitem_add(
        bt->menu_item, menu_item_alloc_function("Start app", NULL, bt_menu_start_app, bt));

    bt->view_test_tone_tx = view_alloc();
    view_set_context(bt->view_test_tone_tx, bt);
    view_set_draw_callback(bt->view_test_tone_tx, bt_view_test_tone_tx_draw);
    view_allocate_model(
        bt->view_test_tone_tx, ViewModelTypeLocking, sizeof(BtViewTestToneTxModel));
    view_set_input_callback(bt->view_test_tone_tx, bt_view_test_tone_tx_input);
    bt->view_test_packet_tx = view_alloc();
    view_set_context(bt->view_test_packet_tx, bt);
    view_set_draw_callback(bt->view_test_packet_tx, bt_view_test_packet_tx_draw);
    view_allocate_model(
        bt->view_test_packet_tx, ViewModelTypeLocking, sizeof(BtViewTestPacketTxModel));
    view_set_input_callback(bt->view_test_packet_tx, bt_view_test_packet_tx_input);
    bt->view_test_tone_rx = view_alloc();
    view_set_context(bt->view_test_tone_rx, bt);
    view_set_draw_callback(bt->view_test_tone_rx, bt_view_test_tone_rx_draw);
    view_allocate_model(bt->view_test_tone_rx, ViewModelTypeLocking, sizeof(BtViewTestRxModel));
    view_set_input_callback(bt->view_test_tone_rx, bt_view_test_tone_rx_input);
    bt->view_start_app = view_alloc();
    view_set_context(bt->view_start_app, bt);
    view_set_draw_callback(bt->view_start_app, bt_view_app_draw);
    view_set_previous_callback(bt->view_start_app, bt_view_exit);
    bt->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_add_view(bt->view_dispatcher, BtViewTestToneTx, bt->view_test_tone_tx);
    view_dispatcher_add_view(bt->view_dispatcher, BtViewTestPacketTx, bt->view_test_packet_tx);
    view_dispatcher_add_view(bt->view_dispatcher, BtViewTestToneRx, bt->view_test_tone_rx);
    view_dispatcher_add_view(bt->view_dispatcher, BtViewStartApp, bt->view_start_app);

    Gui* gui = furi_record_open("gui");
    view_dispatcher_attach_to_gui(bt->view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    with_value_mutex(
        bt->menu, (Menu * menu) { menu_item_add(menu, bt->menu_item); });
    return bt;
}

void bt_draw_statusbar_callback(Canvas* canvas, void* context) {
    canvas_draw_icon_name(canvas, 0, 0, I_Bluetooth_5x8);
}

void bt_menu_test_tone_tx(void* context) {
    furi_assert(context);
    Bt* bt = context;
    bt->state.type = BtStatusToneTx;
    BtMessage message = {
        .type = BtMessageTypeStartTestToneTx,
        .param.channel = bt->state.param.channel,
        .param.power = bt->state.param.power};
    furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
}

void bt_menu_test_packet_tx(void* context) {
    furi_assert(context);
    Bt* bt = context;
    bt->state.type = BtStatusPacketSetup;
    BtMessage message = {
        .type = BtMessageTypeSetupTestPacketTx,
        .param.channel = bt->state.param.channel,
        .param.datarate = bt->state.param.datarate};
    furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
}

void bt_menu_test_tone_rx(void* context) {
    furi_assert(context);
    Bt* bt = context;
    bt->state.type = BtStatusToneRx;
    BtMessage message = {
        .type = BtMessageTypeStartTestRx,
        .param.channel = bt->state.param.channel,
        .param.power = bt->state.param.power};
    furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
}

void bt_menu_start_app(void* context) {
    furi_assert(context);
    Bt* bt = context;
    bt->state.type = BtStatusStartedApp;
    BtMessage message = {.type = BtMessageTypeStartApp};
    furi_check(osMessageQueuePut(bt->message_queue, &message, 0, osWaitForever) == osOK);
}

int32_t bt_task() {
    Bt* bt = bt_alloc();

    furi_record_create("bt", bt);

    api_hal_bt_init();
    BtMessage message;
    while(1) {
        furi_check(osMessageQueueGet(bt->message_queue, &message, NULL, osWaitForever) == osOK);
        if(message.type == BtMessageTypeStartTestToneTx) {
            // Start test tx
            api_hal_bt_stop_tone_tx();
            if(bt->state.type == BtStatusToneTx) {
                api_hal_bt_start_tone_tx(message.param.channel, message.param.power);
            } else {
                bt->state.param.channel =
                    bt_switch_channel(InputKeyRight, bt->state.param.channel);
                bt->state.param.power = BtPower6dB;
                api_hal_bt_start_tone_tx(bt->state.param.channel, bt->state.param.power);
            }
            with_view_model(
                bt->view_test_tone_tx, (BtViewTestToneTxModel * model) {
                    model->type = bt->state.type;
                    model->channel = bt->state.param.channel;
                    model->power = bt->state.param.power;
                    return true;
                });
            view_dispatcher_switch_to_view(bt->view_dispatcher, BtViewTestToneTx);
        } else if(message.type == BtMessageTypeStopTestToneTx) {
            // Stop test tone tx
            api_hal_bt_stop_tone_tx();
            bt->state.type = BtStatusReady;
        } else if(message.type == BtMessageTypeSetupTestPacketTx) {
            // Update packet test setup
            api_hal_bt_stop_packet_test();
            with_view_model(
                bt->view_test_packet_tx, (BtViewTestPacketTxModel * model) {
                    model->type = bt->state.type;
                    model->channel = bt->state.param.channel;
                    model->datarate = bt->state.param.datarate;
                    return true;
                });
            view_dispatcher_switch_to_view(bt->view_dispatcher, BtViewTestPacketTx);
        } else if(message.type == BtMessageTypeStartTestPacketTx) {
            // Start sending packets
            api_hal_bt_start_packet_tx(message.param.channel, 1, message.param.datarate);
            with_view_model(
                bt->view_test_packet_tx, (BtViewTestPacketTxModel * model) {
                    model->type = bt->state.type;
                    model->channel = bt->state.param.channel;
                    model->datarate = bt->state.param.datarate;
                    return true;
                });
            view_dispatcher_switch_to_view(bt->view_dispatcher, BtViewTestPacketTx);
        } else if(message.type == BtMessageTypeStopTestPacketTx) {
            // Stop test packet tx
            api_hal_bt_stop_packet_test();
            bt->state.type = BtStatusReady;
        } else if(message.type == BtMessageTypeStartTestRx) {
            // Start test rx
            api_hal_bt_start_rx(message.param.channel);
            with_view_model(
                bt->view_test_tone_rx, (BtViewTestRxModel * model) {
                    model->channel = bt->state.param.channel;
                    return true;
                });
            view_dispatcher_switch_to_view(bt->view_dispatcher, BtViewTestToneRx);
        } else if(message.type == BtMessageTypeStopTestRx) {
            // Stop test rx
            api_hal_bt_stop_rx();
            bt->state.type = BtStatusReady;
        } else if(message.type == BtMessageTypeStartApp) {
            // Start app
            view_dispatcher_switch_to_view(bt->view_dispatcher, BtViewStartApp);
            if(api_hal_bt_start_app()) {
                bt->state.type = BtStatusStartedApp;
            }
        } else if(message.type == BtMessageTypeUpdateStatusbar) {
            // Update statusbar
            view_port_enabled_set(bt->statusbar_view_port, api_hal_bt_is_alive());
        }
    }
    return 0;
}
