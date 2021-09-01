#pragma once
#include <gui/view.h>

typedef enum {
    BtTestStateStarted,
    BtTestStateStopped,
} BtTestState;

typedef struct BtTest BtTest;
typedef void (*BtTestChangeStateCallback)(BtTestState state, void* context);
typedef void (*BtTestBackCallback)(void* context);
typedef struct BtTestParam BtTestParam;
typedef void (*BtTestParamChangeCallback)(BtTestParam* param);

BtTest* bt_test_alloc();

void bt_test_free(BtTest* bt_test);

View* bt_test_get_view(BtTest* bt_test);

BtTestParam* bt_test_param_add(
    BtTest* bt_test,
    const char* label,
    uint8_t values_count,
    BtTestParamChangeCallback change_callback,
    void* context);

void bt_test_set_change_state_callback(BtTest* bt_test, BtTestChangeStateCallback callback);

void bt_test_set_back_callback(BtTest* bt_test, BtTestBackCallback callback);

void bt_test_set_context(BtTest* bt_test, void* context);

void bt_test_set_rssi(BtTest* bt_test, float rssi);

void bt_test_set_packets_tx(BtTest* bt_test, uint32_t packets_num);

void bt_test_set_packets_rx(BtTest* bt_test, uint32_t packets_num);

void bt_test_set_current_value_index(BtTestParam* param, uint8_t current_value_index);

void bt_test_set_current_value_text(BtTestParam* param, const char* current_value_text);

uint8_t bt_test_get_current_value_index(BtTestParam* param);

void* bt_test_get_context(BtTestParam* param);
