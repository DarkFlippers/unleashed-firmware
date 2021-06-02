#pragma once

#include "bt_i.h"
#include "bt_types.h"

#include <stdint.h>
#include <stdbool.h>
#include <gui/canvas.h>
#include <furi.h>
#include <gui/view_dispatcher.h>
#include <gui/view.h>

typedef enum {
    BtViewTestCarrier,
    BtViewTestPacketTx,
    BtViewTestPacketRx,
    BtViewStartApp,
} BtView;

typedef struct {
    BtStateType type;
    BtTestChannel channel;
    BtTestPower power;
    float rssi;
} BtViewTestCarrierModel;

typedef struct {
    BtStateType type;
    BtTestChannel channel;
    BtTestDataRate datarate;
    uint16_t packets_sent;
} BtViewTestPacketTxModel;

typedef struct {
    BtStateType type;
    BtTestChannel channel;
    BtTestDataRate datarate;
    float rssi;
    uint16_t packets_received;
} BtViewTestPacketRxModel;

void bt_view_test_carrier_draw(Canvas* canvas, void* model);

bool bt_view_test_carrier_input(InputEvent* event, void* context);

void bt_view_test_packet_tx_draw(Canvas* canvas, void* model);

bool bt_view_test_packet_tx_input(InputEvent* event, void* context);

void bt_view_test_packet_rx_draw(Canvas* canvas, void* model);

bool bt_view_test_packet_rx_input(InputEvent* event, void* context);

void bt_view_app_draw(Canvas* canvas, void* model);
