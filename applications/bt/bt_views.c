#include "bt_views.h"

void bt_view_test_tone_tx_draw(Canvas* canvas, void* model) {
    BtViewTestToneTxModel* m = model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 12, "Performing continous TX test");
    if(m->type == BtStatusToneTx) {
        canvas_draw_str(canvas, 0, 24, "Manual control mode");
    } else {
        canvas_draw_str(canvas, 0, 24, "Hopping mode");
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Channel:%d MHz", m->channel * 2 + 2402);
    canvas_draw_str(canvas, 0, 36, buffer);
    snprintf(buffer, sizeof(buffer), "Power:%d dB", m->power - BtPower0dB);
    canvas_draw_str(canvas, 0, 48, buffer);
}

void bt_view_test_tone_rx_draw(Canvas* canvas, void* model) {
    BtViewTestRxModel* m = model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 12, "Performing continous RX test");
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Channel:%d MHz", m->channel * 2 + 2402);
    canvas_draw_str(canvas, 0, 24, buffer);
}

void bt_view_test_packet_tx_draw(Canvas* canvas, void* model) {
    BtViewTestPacketTxModel* m = model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 12, "Packets send TX test");
    if(m->type == BtStatusPacketSetup) {
        canvas_draw_str(canvas, 0, 24, "Setup parameters");
        canvas_draw_str(canvas, 0, 36, "Press OK to send packets");
    } else {
        canvas_draw_str(canvas, 0, 24, "Sending packets");
        canvas_draw_str(canvas, 0, 36, "Packets parameters:");
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Channel:%d MHz", m->channel * 2 + 2402);
    canvas_draw_str(canvas, 0, 48, buffer);
    snprintf(buffer, sizeof(buffer), "Daterate:%d Mbps", m->datarate);
    canvas_draw_str(canvas, 0, 60, buffer);
}

void bt_view_app_draw(Canvas* canvas, void* model) {
    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 12, "Start BLE app");
}

BtTestChannel bt_switch_channel(InputKey key, BtTestChannel inst_chan) {
    uint8_t pos = 0;
    BtTestChannel arr[] = {BtChannel2402, BtChannel2440, BtChannel2480};
    for(pos = 0; pos < sizeof(arr); pos++) {
        if(arr[pos] == inst_chan) {
            break;
        }
    }
    if(key == InputKeyRight) {
        pos = (pos + 1) % sizeof(arr);
        return arr[pos];
    } else if(key == InputKeyLeft) {
        if(pos) {
            return arr[pos - 1];
        } else {
            return arr[sizeof(arr) - 1];
        }
    }
    return arr[0];
}

bool bt_view_test_tone_tx_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Bt* bt = context;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            if(osTimerIsRunning(bt->hopping_mode_timer)) {
                osTimerStop(bt->hopping_mode_timer);
            }
            BtMessage m = {.type = BtMessageTypeStopTestToneTx};
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            view_dispatcher_switch_to_view(bt->view_dispatcher, VIEW_NONE);
            return true;
        } else {
            if(event->key == InputKeyRight || event->key == InputKeyLeft) {
                bt->state.param.channel = bt_switch_channel(event->key, bt->state.param.channel);
            } else if(event->key == InputKeyUp) {
                if(bt->state.param.power < BtPower6dB) {
                    bt->state.param.power += 2;
                }
            } else if(event->key == InputKeyDown) {
                if(bt->state.param.power > BtPower0dB) {
                    bt->state.param.power -= 2;
                }
            } else if(event->key == InputKeyOk) {
                if(bt->state.type == BtStatusToneTx) {
                    bt->state.type = BtStatusHoppingTx;
                    osTimerStart(bt->hopping_mode_timer, 2000);
                } else {
                    bt->state.type = BtStatusToneTx;
                    osTimerStop(bt->hopping_mode_timer);
                }
            }
            BtMessage m = {
                .type = BtMessageTypeStartTestToneTx,
                .param.channel = bt->state.param.channel,
                .param.power = bt->state.param.power};
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            return true;
        }
    }
    return false;
}

bool bt_view_test_tone_rx_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Bt* bt = context;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyRight || event->key == InputKeyLeft) {
            bt->state.param.channel = bt_switch_channel(event->key, bt->state.param.channel);
            BtMessage m = {
                .type = BtMessageTypeStartTestRx, .param.channel = bt->state.param.channel};
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            return true;
        } else if(event->key == InputKeyBack) {
            BtMessage m = {.type = BtMessageTypeStopTestRx};
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            view_dispatcher_switch_to_view(bt->view_dispatcher, VIEW_NONE);
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool bt_view_test_packet_tx_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Bt* bt = context;
    if(event->type == InputTypeShort) {
        if(event->key < InputKeyOk) {
            // Process InputKeyUp, InputKeyDown, InputKeyLeft, InputKeyRight
            if(event->key == InputKeyRight || event->key == InputKeyLeft) {
                bt->state.param.channel = bt_switch_channel(event->key, bt->state.param.channel);
            } else if(event->key == InputKeyUp) {
                if(bt->state.param.datarate < BtDateRate2M) {
                    bt->state.param.datarate += 1;
                }
            } else if(event->key == InputKeyDown) {
                if(bt->state.param.datarate > BtDateRate1M) {
                    bt->state.param.datarate -= 1;
                }
            }
            bt->state.type = BtStatusPacketSetup;
            BtMessage m = {
                .type = BtMessageTypeSetupTestPacketTx,
                .param.channel = bt->state.param.channel,
                .param.datarate = bt->state.param.datarate,
            };
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            return true;
        } else if(event->key == InputKeyOk) {
            bt->state.type = BtStatusPacketTx;
            BtMessage m = {
                .type = BtMessageTypeStartTestPacketTx,
                .param.channel = bt->state.param.channel,
                .param.datarate = bt->state.param.datarate,
            };
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            return true;
        } else if(event->key == InputKeyBack) {
            BtMessage m = {
                .type = BtMessageTypeStopTestPacketTx,
            };
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            view_dispatcher_switch_to_view(bt->view_dispatcher, VIEW_NONE);
            return true;
        }
    }
    return false;
}
