#pragma once

typedef enum {
    BtMessageTypeStartTestToneTx,
    BtMessageTypeHoppingTx,
    BtMessageTypeStopTestToneTx,
    BtMessageTypeSetupTestPacketTx,
    BtMessageTypeStartTestPacketTx,
    BtMessageTypeStopTestPacketTx,
    BtMessageTypeStartTestRx,
    BtMessageTypeStopTestRx,
    BtMessageTypeStartApp,
    BtMessageTypeUpdateStatusbar,
} BtMessageType;

typedef enum {
    BtStatusReady,
    BtStatusToneTx,
    BtStatusHoppingTx,
    BtStatusToneRx,
    BtStatusPacketSetup,
    BtStatusPacketTx,
    BtStatusStartedApp,
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
    BtDateRate1M = 1,
    BtDateRate2M = 2,
} BtTestDataRate;

typedef struct {
    BtTestChannel channel;
    BtTestPower power;
    BtTestDataRate datarate;
} BtTestParam;

typedef struct {
    BtMessageType type;
    BtTestParam param;
} BtMessage;

typedef struct {
    BtStateType type;
    BtTestParam param;
} BtState;
