#pragma once

typedef enum {
    BtTestModeRx,
    BtTestModeTx,
    BtTestModeTxHopping,
} BtTestMode;

typedef enum {
    BtTestChannel2402 = 0,
    BtTestChannel2440 = 19,
    BtTestChannel2480 = 39,
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
    uint32_t value;
    const char* str;
} BtTestParamValue;
