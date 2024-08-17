#pragma once

#if defined(BITS_BIG_ENDIAN) && BITS_BIG_ENDIAN == 1
#error Bit structures defined in this file are not portable to BE
#endif

#define LP5562_ADDRESS     0x60
#define LP5562_I2C_TIMEOUT 50

#define LP5562_CHANNEL_RED_CURRENT_REGISTER   0x07
#define LP5562_CHANNEL_GREEN_CURRENT_REGISTER 0x06
#define LP5562_CHANNEL_BLUE_CURRENT_REGISTER  0x05
#define LP5562_CHANNEL_WHITE_CURRENT_REGISTER 0x0F

#define LP5562_CHANNEL_RED_VALUE_REGISTER   0x04
#define LP5562_CHANNEL_GREEN_VALUE_REGISTER 0x03
#define LP5562_CHANNEL_BLUE_VALUE_REGISTER  0x02
#define LP5562_CHANNEL_WHITE_VALUE_REGISTER 0x0E

typedef enum {
    EngExecHold = 0b00,
    EngExecStep = 0b01,
    EngExecRun = 0b10,
    EngExecPC = 0b11,
} EngExec;

typedef struct {
    EngExec ENG3_EXEC : 2;
    EngExec ENG2_EXEC : 2;
    EngExec ENG1_EXEC : 2;
    bool CHIP_EN      : 1;
    bool LOG_EN       : 1;
} Reg00_Enable;

typedef enum {
    EngModeDisable = 0b00,
    EngModeLoad = 0b01,
    EngModeRun = 0b10,
    EngModeDirect = 0b11,
} EngMode;

typedef struct {
    EngMode ENG3_MODE : 2;
    EngMode ENG2_MODE : 2;
    EngMode ENG1_MODE : 2;
    uint8_t reserved  : 2;
} Reg01_OpMode;

typedef struct {
    bool INT_CLK_EN   : 1;
    bool CLK_DET_EN   : 1;
    uint8_t reserved0 : 3;
    bool PS_EN        : 1;
    bool PWM_HF       : 1;
    uint8_t reserved1 : 1;
} Reg08_Config;

typedef struct {
    uint8_t pc       : 3;
    uint8_t reserved : 5;
} Reg09_Engine1PC;

typedef struct {
    uint8_t pc       : 3;
    uint8_t reserved : 5;
} Reg0A_Engine2PC;

typedef struct {
    uint8_t pc       : 3;
    uint8_t reserved : 5;
} Reg0B_Engine3PC;

typedef struct {
    bool ENG3_INT     : 1;
    bool ENG2_INT     : 1;
    bool ENG1_INT     : 1;
    bool EXT_CLK_USED : 1;
    uint8_t reserved  : 5;
} Reg0C_Status;

typedef struct {
    uint8_t value;
} Reg0D_Reset;

typedef enum {
    EngSelectI2C = 0b00,
    EngSelectEngine1 = 0b01,
    EngSelectEngine2 = 0b10,
    EngSelectEngine3 = 0b11,
} EngSelect;

typedef struct {
    EngSelect blue  : 2;
    EngSelect green : 2;
    EngSelect red   : 2;
    EngSelect white : 2;
} Reg70_LedMap;
