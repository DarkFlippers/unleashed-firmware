#pragma once

#include <stdint.h>

typedef enum {
    BtMessageTypeStartTestCarrier,
    BtMessageTypeHoppingTx,
    BtMessageTypeStopTestCarrier,
    BtMessageTypeSetupTestPacketTx,
    BtMessageTypeSetupTestPacketRx,
    BtMessageTypeStartTestPacketTx,
    BtMessageTypeStartTestPacketRx,
    BtMessageTypeStopTestPacket,
    BtMessageTypeStartApp,
    BtMessageTypeUpdateStatusbar,
} BtMessageType;

typedef enum {
    BtStateReady,
    BtStateCarrierTx,
    BtStateHoppingTx,
    BtStateCarrierRxStart,
    BtStateCarrierRxRunning,
    BtStatePacketSetup,
    BtStatePacketStart,
    BtStatePacketRunning,
    BtStateStartedApp,
} BtStateType;

typedef enum {
    BtChannel2402 = 0,
    BtChannel2440 = 19,
    BtChannel2480 = 39,
} BtTestChannel;

typedef enum {
    BtPower0dB = 0x19,
    BtPower2dB = 0x1B,
    BtPower4dB = 0x1D,
    BtPower6dB = 0x1F,
} BtTestPower;

typedef enum {
    BtDataRate1M = 1,
    BtDataRate2M = 2,
} BtTestDataRate;

typedef struct {
    BtTestChannel channel;
    BtTestPower power;
    BtTestDataRate datarate;
    float rssi;
    uint16_t packets_sent;
    uint16_t packets_received;
} BtTestParam;

typedef struct {
    BtMessageType type;
    BtTestParam param;
} BtMessage;

typedef struct {
    BtStateType type;
    BtTestParam param;
} BtState;
