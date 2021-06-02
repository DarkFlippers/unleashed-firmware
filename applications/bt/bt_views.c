#include "bt_views.h"

void bt_view_test_carrier_draw(Canvas* canvas, void* model) {
    BtViewTestCarrierModel* m = model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 12, "Performing Cattier test");
    if(m->type == BtStateCarrierTx) {
        canvas_draw_str(canvas, 0, 24, "Manual Carrier TX");
    } else if(m->type == BtStateHoppingTx) {
        canvas_draw_str(canvas, 0, 24, "Carrier TX Hopping mode");
    } else if(m->type == BtStateCarrierRxRunning) {
        canvas_draw_str(canvas, 0, 24, "Manual Carrier RX");
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Channel:%d MHz", m->channel * 2 + 2402);
    canvas_draw_str(canvas, 0, 36, buffer);
    if(m->type == BtStateCarrierTx || m->type == BtStateHoppingTx) {
        snprintf(buffer, sizeof(buffer), "Power:%d dB", m->power - BtPower0dB);
    } else if(m->type == BtStateCarrierRxRunning) {
        snprintf(buffer, sizeof(buffer), "RSSI: %3.1f dB", m->rssi);
    }
    canvas_draw_str(canvas, 0, 48, buffer);
}

void bt_view_test_packet_rx_draw(Canvas* canvas, void* model) {
    BtViewTestPacketRxModel* m = model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 12, "Performing packets RX test");
    if(m->type == BtStatePacketSetup) {
        canvas_draw_str(canvas, 0, 24, "Setup parameters. Ok to start");
    } else {
        canvas_draw_str(canvas, 0, 24, "Receiving packets ...");
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Channel:%d MHz", m->channel * 2 + 2402);
    canvas_draw_str(canvas, 0, 36, buffer);
    snprintf(buffer, sizeof(buffer), "Datarate:%d Mbps", m->datarate);
    canvas_draw_str(canvas, 0, 48, buffer);
    if(m->type == BtStatePacketSetup) {
        snprintf(buffer, sizeof(buffer), "%d packets received", m->packets_received);
    } else {
        snprintf(buffer, sizeof(buffer), "RSSI: %3.1f dB", m->rssi);
    }
    canvas_draw_str(canvas, 0, 60, buffer);
}

void bt_view_test_packet_tx_draw(Canvas* canvas, void* model) {
    BtViewTestPacketTxModel* m = model;
    canvas_clear(canvas);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 12, "Packets send TX test");
    if(m->type == BtStatePacketSetup) {
        canvas_draw_str(canvas, 0, 24, "Setup parameters. Ok to start");
    } else {
        canvas_draw_str(canvas, 0, 24, "Sending packets ...");
    }
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Channel:%d MHz", m->channel * 2 + 2402);
    canvas_draw_str(canvas, 0, 36, buffer);
    snprintf(buffer, sizeof(buffer), "Datarate:%d Mbps", m->datarate);
    canvas_draw_str(canvas, 0, 48, buffer);
    if(m->packets_sent && m->type == BtStatePacketSetup) {
        snprintf(buffer, sizeof(buffer), "%d packets sent", m->packets_sent);
        canvas_draw_str(canvas, 0, 60, buffer);
    }
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

bool bt_view_test_carrier_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Bt* bt = context;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            if(osTimerIsRunning(bt->update_param_timer)) {
                osTimerStop(bt->update_param_timer);
            }
            BtMessage m = {.type = BtMessageTypeStopTestCarrier};
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
                if(bt->state.type == BtStateCarrierTx) {
                    bt->state.type = BtStateHoppingTx;
                    osTimerStart(bt->update_param_timer, 2000);
                } else if(bt->state.type == BtStateHoppingTx) {
                    osTimerStop(bt->update_param_timer);
                    bt->state.type = BtStateCarrierRxStart;
                    osTimerStart(bt->update_param_timer, 200);
                } else {
                    osTimerStop(bt->update_param_timer);
                    bt->state.type = BtStateCarrierTx;
                }
            }
            BtMessage m = {
                .type = BtMessageTypeStartTestCarrier,
                .param.channel = bt->state.param.channel,
                .param.power = bt->state.param.power};
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            return true;
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
                if(bt->state.param.datarate < BtDataRate2M) {
                    bt->state.param.datarate += 1;
                }
            } else if(event->key == InputKeyDown) {
                if(bt->state.param.datarate > BtDataRate1M) {
                    bt->state.param.datarate -= 1;
                }
            }
            bt->state.type = BtStatePacketSetup;
            BtMessage m = {
                .type = BtMessageTypeSetupTestPacketTx,
                .param.channel = bt->state.param.channel,
                .param.datarate = bt->state.param.datarate,
            };
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            return true;
        } else if(event->key == InputKeyOk) {
            if(bt->state.type == BtStatePacketSetup) {
                bt->state.type = BtStatePacketStart;
            } else if(bt->state.type == BtStatePacketStart) {
                bt->state.type = BtStatePacketSetup;
            }
            BtMessage m = {
                .type = BtMessageTypeStartTestPacketTx,
                .param.channel = bt->state.param.channel,
                .param.datarate = bt->state.param.datarate,
            };
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            return true;
        } else if(event->key == InputKeyBack) {
            BtMessage m = {
                .type = BtMessageTypeStopTestPacket,
            };
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            view_dispatcher_switch_to_view(bt->view_dispatcher, VIEW_NONE);
            return true;
        }
    }
    return false;
}

bool bt_view_test_packet_rx_input(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    Bt* bt = context;
    if(event->type == InputTypeShort) {
        if(event->key < InputKeyOk) {
            // Process InputKeyUp, InputKeyDown, InputKeyLeft, InputKeyRight
            if(event->key == InputKeyRight || event->key == InputKeyLeft) {
                bt->state.param.channel = bt_switch_channel(event->key, bt->state.param.channel);
            } else if(event->key == InputKeyUp) {
                if(bt->state.param.datarate < BtDataRate2M) {
                    bt->state.param.datarate += 1;
                }
            } else if(event->key == InputKeyDown) {
                if(bt->state.param.datarate > BtDataRate1M) {
                    bt->state.param.datarate -= 1;
                }
            }
            bt->state.type = BtStatePacketSetup;
            BtMessage m = {
                .type = BtMessageTypeSetupTestPacketRx,
                .param.channel = bt->state.param.channel,
                .param.datarate = bt->state.param.datarate,
            };
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            return true;
        } else if(event->key == InputKeyOk) {
            if(bt->state.type == BtStatePacketSetup) {
                bt->state.type = BtStatePacketStart;
                osTimerStart(bt->update_param_timer, 200);
            } else if(bt->state.type == BtStatePacketRunning) {
                bt->state.type = BtStatePacketSetup;
                osTimerStop(bt->update_param_timer);
            }
            BtMessage m = {
                .type = BtMessageTypeStartTestPacketRx,
                .param.channel = bt->state.param.channel,
                .param.datarate = bt->state.param.datarate,
            };
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            return true;
        } else if(event->key == InputKeyBack) {
            if(osTimerIsRunning(bt->update_param_timer)) {
                osTimerStop(bt->update_param_timer);
            }
            BtMessage m = {
                .type = BtMessageTypeStopTestPacket,
            };
            furi_check(osMessageQueuePut(bt->message_queue, &m, 0, osWaitForever) == osOK);
            view_dispatcher_switch_to_view(bt->view_dispatcher, VIEW_NONE);
            return true;
        }
    }
    return false;
}
