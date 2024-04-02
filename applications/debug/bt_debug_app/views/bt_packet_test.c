#include "bt_packet_test.h"
#include "bt_test.h"
#include "bt_test_types.h"
#include <furi_hal_bt.h>

struct BtPacketTest {
    BtTest* bt_test;
    BtTestMode mode;
    BtTestChannel channel;
    BtTestDataRate data_rate;
    FuriTimer* timer;
};

static BtTestParamValue bt_param_mode[] = {
    {.value = BtTestModeRx, .str = "Rx"},
    {.value = BtTestModeTx, .str = "Tx"},
};

static BtTestParamValue bt_param_channel[] = {
    {.value = BtTestChannel2402, .str = "2402 MHz"},
    {.value = BtTestChannel2440, .str = "2440 MHz"},
    {.value = BtTestChannel2480, .str = "2480 MHz"},
};

static BtTestParamValue bt_param_data_rate[] = {
    {.value = BtDataRate1M, .str = "1 Mbps"},
    {.value = BtDataRate2M, .str = "2 Mbps"},
};

static void bt_packet_test_start(BtPacketTest* bt_packet_test) {
    furi_assert(bt_packet_test);
    if(bt_packet_test->mode == BtTestModeRx) {
        furi_hal_bt_start_packet_rx(bt_packet_test->channel, bt_packet_test->data_rate);
        furi_timer_start(bt_packet_test->timer, furi_kernel_get_tick_frequency() / 4);
    } else if(bt_packet_test->mode == BtTestModeTx) {
        furi_hal_bt_start_packet_tx(bt_packet_test->channel, 1, bt_packet_test->data_rate);
    }
}

static void bt_packet_test_stop(BtPacketTest* bt_packet_test) {
    furi_assert(bt_packet_test);
    if(bt_packet_test->mode == BtTestModeTx) {
        furi_hal_bt_stop_packet_test();
        bt_test_set_packets_tx(bt_packet_test->bt_test, furi_hal_bt_get_transmitted_packets());
    } else if(bt_packet_test->mode == BtTestModeRx) {
        bt_test_set_packets_rx(bt_packet_test->bt_test, furi_hal_bt_stop_packet_test());
        furi_timer_stop(bt_packet_test->timer);
    }
}

static uint32_t bt_packet_test_param_changed(BtTestParam* param, BtTestParamValue* param_val) {
    furi_assert(param);
    uint8_t index = bt_test_get_current_value_index(param);
    bt_test_set_current_value_text(param, param_val[index].str);
    return param_val[index].value;
}

static void bt_packet_test_mode_changed(BtTestParam* param) {
    BtPacketTest* bt_packet_test = bt_test_get_context(param);
    bt_packet_test_stop(bt_packet_test);
    bt_packet_test->mode = bt_packet_test_param_changed(param, bt_param_mode);
}

static void bt_packet_test_channel_changed(BtTestParam* param) {
    BtPacketTest* bt_packet_test = bt_test_get_context(param);
    bt_packet_test_stop(bt_packet_test);
    bt_packet_test->channel = bt_packet_test_param_changed(param, bt_param_channel);
}

static void bt_packet_test_param_channel(BtTestParam* param) {
    BtPacketTest* bt_packet_test = bt_test_get_context(param);
    bt_packet_test_stop(bt_packet_test);
    bt_packet_test->data_rate = bt_packet_test_param_changed(param, bt_param_data_rate);
}

static void bt_packet_test_change_state_callback(BtTestState state, void* context) {
    furi_assert(context);
    BtPacketTest* bt_packet_test = context;
    if(state == BtTestStateStarted) {
        bt_packet_test_start(bt_packet_test);
    } else if(state == BtTestStateStopped) {
        bt_packet_test_stop(bt_packet_test);
    }
}

static void bt_packet_test_exit_callback(void* context) {
    furi_assert(context);
    BtPacketTest* bt_packet_test = context;
    bt_packet_test_stop(bt_packet_test);
}

static void bt_test_packet_timer_callback(void* context) {
    furi_assert(context);
    BtPacketTest* bt_packet_test = context;
    if(bt_packet_test->mode == BtTestModeRx) {
        bt_test_set_rssi(bt_packet_test->bt_test, furi_hal_bt_get_rssi());
    }
}

BtPacketTest* bt_packet_test_alloc(void) {
    BtPacketTest* bt_packet_test = malloc(sizeof(BtPacketTest));
    bt_packet_test->bt_test = bt_test_alloc();
    bt_test_set_context(bt_packet_test->bt_test, bt_packet_test);
    bt_test_set_change_state_callback(
        bt_packet_test->bt_test, bt_packet_test_change_state_callback);
    bt_test_set_back_callback(bt_packet_test->bt_test, bt_packet_test_exit_callback);

    BtTestParam* param;
    param = bt_test_param_add(
        bt_packet_test->bt_test,
        "Mode",
        COUNT_OF(bt_param_mode),
        bt_packet_test_mode_changed,
        bt_packet_test);
    bt_test_set_current_value_index(param, 0);
    bt_test_set_current_value_text(param, bt_param_mode[0].str);
    bt_packet_test->mode = BtTestModeRx;

    param = bt_test_param_add(
        bt_packet_test->bt_test,
        "Channel",
        COUNT_OF(bt_param_channel),
        bt_packet_test_channel_changed,
        bt_packet_test);
    bt_test_set_current_value_index(param, 0);
    bt_test_set_current_value_text(param, bt_param_channel[0].str);
    bt_packet_test->channel = BtTestChannel2402;

    param = bt_test_param_add(
        bt_packet_test->bt_test,
        "Data rate",
        COUNT_OF(bt_param_data_rate),
        bt_packet_test_param_channel,
        bt_packet_test);
    bt_test_set_current_value_index(param, 0);
    bt_test_set_current_value_text(param, bt_param_data_rate[0].str);
    bt_packet_test->data_rate = BtDataRate1M;

    bt_packet_test->timer =
        furi_timer_alloc(bt_test_packet_timer_callback, FuriTimerTypePeriodic, bt_packet_test);

    return bt_packet_test;
}

void bt_packet_test_free(BtPacketTest* bt_packet_test) {
    furi_assert(bt_packet_test);
    bt_test_free(bt_packet_test->bt_test);
    furi_timer_free(bt_packet_test->timer);
    free(bt_packet_test);
}

View* bt_packet_test_get_view(BtPacketTest* bt_packet_test) {
    furi_assert(bt_packet_test);
    return bt_test_get_view(bt_packet_test->bt_test);
}
