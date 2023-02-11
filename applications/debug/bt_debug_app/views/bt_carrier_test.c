#include "bt_carrier_test.h"
#include "bt_test.h"
#include "bt_test_types.h"
#include <furi_hal_bt.h>

struct BtCarrierTest {
    BtTest* bt_test;
    BtTestParam* bt_param_channel;
    BtTestMode mode;
    BtTestChannel channel;
    BtTestPower power;
    FuriTimer* timer;
};

static BtTestParamValue bt_param_mode[] = {
    {.value = BtTestModeRx, .str = "Rx"},
    {.value = BtTestModeTx, .str = "Tx"},
    {.value = BtTestModeTxHopping, .str = "Hopping Tx"},
};

static BtTestParamValue bt_param_channel[] = {
    {.value = BtTestChannel2402, .str = "2402 MHz"},
    {.value = BtTestChannel2440, .str = "2440 MHz"},
    {.value = BtTestChannel2480, .str = "2480 MHz"},
};

static BtTestParamValue bt_param_power[] = {
    {.value = BtPower0dB, .str = "0 dB"},
    {.value = BtPower2dB, .str = "2 dB"},
    {.value = BtPower4dB, .str = "4 dB"},
    {.value = BtPower6dB, .str = "6 dB"},
};

static void bt_carrier_test_start(BtCarrierTest* bt_carrier_test) {
    furi_assert(bt_carrier_test);
    if(bt_carrier_test->mode == BtTestModeRx) {
        furi_hal_bt_start_packet_rx(bt_carrier_test->channel, 1);
        furi_timer_start(bt_carrier_test->timer, furi_kernel_get_tick_frequency() / 4);
    } else if(bt_carrier_test->mode == BtTestModeTxHopping) {
        furi_hal_bt_start_tone_tx(bt_carrier_test->channel, bt_carrier_test->power);
        furi_timer_start(bt_carrier_test->timer, furi_kernel_get_tick_frequency() * 2);
    } else if(bt_carrier_test->mode == BtTestModeTx) {
        furi_hal_bt_start_tone_tx(bt_carrier_test->channel, bt_carrier_test->power);
    }
}

static void bt_carrier_test_switch_channel(BtCarrierTest* bt_carrier_test) {
    furi_assert(bt_carrier_test);
    furi_hal_bt_stop_tone_tx();
    uint8_t channel_i = 0;
    if(bt_carrier_test->channel == BtTestChannel2402) {
        bt_carrier_test->channel = BtTestChannel2440;
        channel_i = 1;
    } else if(bt_carrier_test->channel == BtTestChannel2440) {
        bt_carrier_test->channel = BtTestChannel2480;
        channel_i = 2;
    } else if(bt_carrier_test->channel == BtTestChannel2480) {
        bt_carrier_test->channel = BtTestChannel2402;
        channel_i = 0;
    }
    furi_hal_bt_start_tone_tx(bt_carrier_test->channel, bt_carrier_test->power);
    bt_test_set_current_value_index(bt_carrier_test->bt_param_channel, channel_i);
    bt_test_set_current_value_text(
        bt_carrier_test->bt_param_channel, bt_param_channel[channel_i].str);
}

static void bt_carrier_test_stop(BtCarrierTest* bt_carrier_test) {
    furi_assert(bt_carrier_test);
    if(bt_carrier_test->mode == BtTestModeTxHopping) {
        furi_hal_bt_stop_tone_tx();
        furi_timer_stop(bt_carrier_test->timer);
    } else if(bt_carrier_test->mode == BtTestModeTx) {
        furi_hal_bt_stop_tone_tx();
    } else if(bt_carrier_test->mode == BtTestModeRx) {
        furi_hal_bt_stop_packet_test();
        furi_timer_stop(bt_carrier_test->timer);
    }
}

static uint32_t bt_carrier_test_param_changed(BtTestParam* param, BtTestParamValue* param_val) {
    furi_assert(param);
    uint8_t index = bt_test_get_current_value_index(param);
    bt_test_set_current_value_text(param, param_val[index].str);
    return param_val[index].value;
}

static void bt_carrier_test_mode_changed(BtTestParam* param) {
    BtCarrierTest* bt_carrier_test = bt_test_get_context(param);
    bt_carrier_test_stop(bt_carrier_test);
    bt_carrier_test->mode = bt_carrier_test_param_changed(param, bt_param_mode);
}

static void bt_carrier_test_channel_changed(BtTestParam* param) {
    BtCarrierTest* bt_carrier_test = bt_test_get_context(param);
    bt_carrier_test_stop(bt_carrier_test);
    bt_carrier_test->channel = bt_carrier_test_param_changed(param, bt_param_channel);
}

static void bt_carrier_test_param_channel(BtTestParam* param) {
    BtCarrierTest* bt_carrier_test = bt_test_get_context(param);
    bt_carrier_test_stop(bt_carrier_test);
    bt_carrier_test->power = bt_carrier_test_param_changed(param, bt_param_power);
}

static void bt_carrier_test_change_state_callback(BtTestState state, void* context) {
    furi_assert(context);
    BtCarrierTest* bt_carrier_test = context;
    furi_hal_bt_stop_tone_tx();
    if(state == BtTestStateStarted) {
        bt_carrier_test_start(bt_carrier_test);
    } else if(state == BtTestStateStopped) {
        bt_carrier_test_stop(bt_carrier_test);
    }
}

static void bt_carrier_test_exit_callback(void* context) {
    furi_assert(context);
    BtCarrierTest* bt_carrier_test = context;
    bt_carrier_test_stop(bt_carrier_test);
}

static void bt_test_carrier_timer_callback(void* context) {
    furi_assert(context);
    BtCarrierTest* bt_carrier_test = context;
    if(bt_carrier_test->mode == BtTestModeRx) {
        bt_test_set_rssi(bt_carrier_test->bt_test, furi_hal_bt_get_rssi());
    } else if(bt_carrier_test->mode == BtTestModeTxHopping) {
        bt_carrier_test_switch_channel(bt_carrier_test);
    }
}

BtCarrierTest* bt_carrier_test_alloc() {
    BtCarrierTest* bt_carrier_test = malloc(sizeof(BtCarrierTest));
    bt_carrier_test->bt_test = bt_test_alloc();
    bt_test_set_context(bt_carrier_test->bt_test, bt_carrier_test);
    bt_test_set_change_state_callback(
        bt_carrier_test->bt_test, bt_carrier_test_change_state_callback);
    bt_test_set_back_callback(bt_carrier_test->bt_test, bt_carrier_test_exit_callback);

    BtTestParam* param;
    param = bt_test_param_add(
        bt_carrier_test->bt_test,
        "Mode",
        COUNT_OF(bt_param_mode),
        bt_carrier_test_mode_changed,
        bt_carrier_test);
    bt_test_set_current_value_index(param, 0);
    bt_test_set_current_value_text(param, bt_param_mode[0].str);
    bt_carrier_test->mode = BtTestModeRx;

    param = bt_test_param_add(
        bt_carrier_test->bt_test,
        "Channel",
        COUNT_OF(bt_param_channel),
        bt_carrier_test_channel_changed,
        bt_carrier_test);
    bt_test_set_current_value_index(param, 0);
    bt_test_set_current_value_text(param, bt_param_channel[0].str);
    bt_carrier_test->channel = BtTestChannel2402;
    bt_carrier_test->bt_param_channel = param;

    param = bt_test_param_add(
        bt_carrier_test->bt_test,
        "Power",
        COUNT_OF(bt_param_power),
        bt_carrier_test_param_channel,
        bt_carrier_test);
    bt_test_set_current_value_index(param, 0);
    bt_test_set_current_value_text(param, bt_param_power[0].str);
    bt_carrier_test->power = BtPower0dB;

    bt_carrier_test->timer =
        furi_timer_alloc(bt_test_carrier_timer_callback, FuriTimerTypePeriodic, bt_carrier_test);

    return bt_carrier_test;
}

void bt_carrier_test_free(BtCarrierTest* bt_carrier_test) {
    furi_assert(bt_carrier_test);
    bt_test_free(bt_carrier_test->bt_test);
    furi_timer_free(bt_carrier_test->timer);
    free(bt_carrier_test);
}

View* bt_carrier_test_get_view(BtCarrierTest* bt_carrier_test) {
    furi_assert(bt_carrier_test);
    return bt_test_get_view(bt_carrier_test->bt_test);
}
