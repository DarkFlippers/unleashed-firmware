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
    BtViewTestToneTx,
    BtViewTestPacketTx,
    BtViewTestToneRx,
    BtViewStartApp,
} BtView;

typedef struct {
    BtStateType type;
    BtTestChannel channel;
    BtTestPower power;
} BtViewTestToneTxModel;

typedef struct {
    BtStateType type;
    BtTestChannel channel;
    BtTestDataRate datarate;
} BtViewTestPacketTxModel;

typedef struct {
    BtTestChannel channel;
} BtViewTestRxModel;

void bt_view_test_tone_tx_draw(Canvas* canvas, void* model);

bool bt_view_test_tone_tx_input(InputEvent* event, void* context);

void bt_view_test_packet_tx_draw(Canvas* canvas, void* model);

bool bt_view_test_packet_tx_input(InputEvent* event, void* context);

void bt_view_test_tone_rx_draw(Canvas* canvas, void* model);

bool bt_view_test_tone_rx_input(InputEvent* event, void* context);

void bt_view_app_draw(Canvas* canvas, void* model);
