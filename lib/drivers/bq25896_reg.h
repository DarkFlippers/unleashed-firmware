#pragma once

#include <stdbool.h>
#include <stdint.h>

#if BITS_BIG_ENDIAN == 1
#error Bit structures defined in this file is not portable to BE
#endif

#define BQ25896_ADDRESS 0xD6
#define BQ25896_I2C_TIMEOUT 50

#define IILIM_1600 (1 << 5)
#define IILIM_800 (1 << 4)
#define IILIM_400 (1 << 3)
#define IILIM_200 (1 << 2)
#define IILIM_100 (1 << 1)
#define IILIM_50 (1 << 0)

typedef struct {
    uint8_t IINLIM : 6; // Input Current Limit, mA, offset: +100mA
    bool EN_ILIM : 1; // Enable ILIM Pin
    bool EN_HIZ : 1; // Enable HIZ Mode
} REG00;

#define VINDPM_OS_1600 (1 << 4)
#define VINDPM_OS_800 (1 << 3)
#define VINDPM_OS_400 (1 << 2)
#define VINDPM_OS_200 (1 << 1)
#define VINDPM_OS_100 (1 << 0)

typedef enum {
    Bhot34 = 0b00, // – VBHOT1 Threshold (34.75%) (default)
    Bhot37 = 0b01, // – VBHOT0 Threshold (Typ. 37.75%)
    Bhot31 = 0b10, // – VBHOT2 Threshold (Typ. 31.25%)
    BhotDisable = 0b11, // – Disable boost mode thermal protection
} Bhot;

typedef struct {
    uint8_t VINDPM_OS : 5; // Input Voltage Limit Offset, mV
    bool BCOLD : 1; // Boost Mode Cold Temperature Monitor Threshold
    Bhot BHOT : 2; // Boost Mode Hot Temperature Monitor Threshold
} REG01;

typedef struct {
    bool AUTO_DPDM_EN : 1; // Automatic Input Detection Enable
    bool FORCE_DPDM : 1; // Force Input Detection
    uint8_t RES : 2; // Reserved
    bool ICO_EN : 1; // Input Current Optimizer (ICO) Enable
    bool BOOST_FREQ : 1; // Boost Mode Frequency Selection
    bool CONV_RATE : 1; // ADC Conversion Rate Selection
    bool CONV_START : 1; // ADC Conversion Start Control
} REG02;

#define SYS_MIN_400 (1 << 2)
#define SYS_MIN_200 (1 << 1)
#define SYS_MIN_100 (1 << 0)

typedef struct {
    bool MIN_VBAT_SEL : 1; // Minimum Battery Voltage (falling) to exit boost mode
    uint8_t SYS_MIN : 3; // Minimum System Voltage Limit, mV, offset: +3000mV
    bool CHG_CONFIG : 1; // Charge Enable Configuration
    bool OTG_CONFIG : 1; // Boost (OTG) Mode Configuration
    bool WD_RST : 1; // I2C Watchdog Timer Reset
    bool BAT_LOADEN : 1; // Battery Load (IBATLOAD) Enable
} REG03;

#define ICHG_4096 (1 << 6)
#define ICHG_2048 (1 << 5)
#define ICHG_1024 (1 << 4)
#define ICHG_512 (1 << 3)
#define ICHG_256 (1 << 2)
#define ICHG_128 (1 << 1)
#define ICHG_64 (1 << 0)

typedef struct {
    uint8_t ICHG : 7; // Fast Charge Current Limit, mA
    bool EN_PUMPX : 1; // Current pulse control Enable
} REG04;

#define IPRETERM_512 (1 << 3)
#define IPRETERM_256 (1 << 2)
#define IPRETERM_128 (1 << 1)
#define IPRETERM_64 (1 << 0)

typedef struct {
    uint8_t ITERM : 4; // Termination Current Limit, offset: +64mA
    uint8_t IPRECHG : 4; // Precharge Current Limit, offset: +64mA
} REG05;

#define VREG_512 (1 << 5)
#define VREG_256 (1 << 4)
#define VREG_128 (1 << 3)
#define VREG_64 (1 << 2)
#define VREG_32 (1 << 1)
#define VREG_16 (1 << 0)

typedef struct {
    bool VRECHG : 1; // Battery Recharge Threshold Offset
    bool BATLOWV : 1; // Battery Precharge to Fast Charge Threshold
    uint8_t VREG : 6; // Charge Voltage Limit, offset: +3840mV
} REG06;

typedef enum {
    WatchdogDisable = 0b00,
    Watchdog40 = 0b01,
    Watchdog80 = 0b10,
    Watchdog160 = 0b11,
} Watchdog;

typedef enum {
    ChgTimer5 = 0b00,
    ChgTimer8 = 0b01,
    ChgTimer12 = 0b10,
    ChgTimer20 = 0b11,
} ChgTimer;

typedef struct {
    bool JEITA_ISET : 1; // JEITA Low Temperature Current Setting
    ChgTimer CHG_TIMER : 2; // Fast Charge Timer Setting
    bool EN_TIMER : 1; // Charging Safety Timer Enable
    Watchdog WATCHDOG : 2; // I2C Watchdog Timer Setting
    bool STAT_DIS : 1; // STAT Pin Disable
    bool EN_TERM : 1; // Charging Termination Enable
} REG07;

#define BAT_COMP_80 (1 << 2)
#define BAT_COMP_40 (1 << 1)
#define BAT_COMP_20 (1 << 0)

#define VCLAMP_128 (1 << 2)
#define VCLAMP_64 (1 << 1)
#define VCLAMP_32 (1 << 0)

#define TREG_60 (0b00)
#define TREG_80 (0b01)
#define TREG_100 (0b10)
#define TREG_120 (0b11)

typedef struct {
    uint8_t TREG : 2; // Thermal Regulation Threshold
    uint8_t VCLAMP : 3; // IR Compensation Voltage Clamp
    uint8_t BAT_COMP : 3; // IR Compensation Resistor Setting
} REG08;

typedef struct {
    bool PUMPX_DN : 1; // Current pulse control voltage down enable
    bool PUMPX_UP : 1; // Current pulse control voltage up enable
    bool BATFET_RST_EN : 1; // BATFET full system reset enable
    bool BATFET_DLY : 1; // BATFET turn off delay control
    bool JEITA_VSET : 1; // JEITA High Temperature Voltage Setting
    bool BATFET_DIS : 1; // Force BATFET off to enable ship mode
    bool TMR2X_EN : 1; // Safety Timer Setting during DPM or Thermal Regulation
    bool FORCE_ICO : 1; // Force Start Input Current Optimizer
} REG09;

#define BOOSTV_512 (1 << 3)
#define BOOSTV_256 (1 << 2)
#define BOOSTV_128 (1 << 1)
#define BOOSTV_64 (1 << 0)

typedef enum {
    BoostLim_500 = 0b000,
    BoostLim_750 = 0b001,
    BoostLim_1200 = 0b010,
    BoostLim_1400 = 0b011,
    BoostLim_1650 = 0b100,
    BoostLim_1875 = 0b101,
    BoostLim_2150 = 0b110,
    BoostLim_Rsvd = 0b111,
} BoostLim;

typedef struct {
    uint8_t BOOST_LIM : 3; // Boost Mode Current Limit
    bool PFM_OTG_DIS : 1; // PFM mode allowed in boost mode
    uint8_t BOOSTV : 4; // Boost Mode Voltage Regulation, offset: +4550mV
} REG0A;

typedef enum {
    VBusStatNo = 0b000,
    VBusStatUSB = 0b001,
    VBusStatExternal = 0b010,
    VBusStatOTG = 0b111,
} VBusStat;

typedef enum {
    ChrgStatNo = 0b00,
    ChrgStatPre = 0b01,
    ChrgStatFast = 0b10,
    ChrgStatDone = 0b11,
} ChrgStat;

typedef struct {
    bool VSYS_STAT : 1; // VSYS Regulation Status
    bool RES : 1; // Reserved: Always reads 1
    bool PG_STAT : 1; // Power Good Status
    ChrgStat CHRG_STAT : 2; // Charging Status
    VBusStat VBUS_STAT : 3; // VBUS Status register
} REG0B;

typedef enum {
    ChrgFaultNO = 0b00,
    ChrgFaultIN = 0b01,
    ChrgFaultTH = 0b10,
    ChrgFaultTIM = 0b11,
} ChrgFault;

typedef enum {
    NtcFaultNo = 0b000,
    NtcFaultWarm = 0b010,
    NtcFaultCool = 0b011,
    NtcFaultCold = 0b101,
    NtcFaultHot = 0b110,
} NtcFault;

typedef struct {
    NtcFault NTC_FAULT : 3; // NTC Fault Status
    bool BAT_FAULT : 1; // Battery Fault Status
    ChrgFault CHRG_FAULT : 2; // Charge Fault Status
    bool BOOST_FAULT : 1; // Boost Mode Fault Status
    bool WATCHDOG_FAULT : 1; // Watchdog Fault Status
} REG0C;

#define VINDPM_6400 (1 << 6)
#define VINDPM_3200 (1 << 5)
#define VINDPM_1600 (1 << 4)
#define VINDPM_800 (1 << 3)
#define VINDPM_400 (1 << 2)
#define VINDPM_200 (1 << 1)
#define VINDPM_100 (1 << 0)

typedef struct {
    uint8_t VINDPM : 7; // Absolute VINDPM Threshold, offset: +2600mV
    bool FORCE_VINDPM : 1; // VINDPM Threshold Setting Method
} REG0D;

typedef struct {
    uint8_t BATV : 7; // ADC conversion of Battery Voltage (VBAT), offset: +2304mV
    bool THERM_STAT : 1; // Thermal Regulation Status
} REG0E;

typedef struct {
    uint8_t SYSV : 7; // ADDC conversion of System Voltage (VSYS), offset: +2304mV
    uint8_t RES : 1; // Reserved: Always reads 0
} REG0F;

typedef struct {
    uint8_t TSPCT : 7; // ADC conversion of TS Voltage (TS) as percentage of REGN, offset: +21%
    uint8_t RES : 1; // Reserved: Always reads 0
} REG10;

typedef struct {
    uint8_t VBUSV : 7; // ADC conversion of VBUS voltage (VBUS), offset: +2600mV
    bool VBUS_GD : 1; // VBUS Good Status
} REG11;

typedef struct {
    uint8_t ICHGR : 7; // ADC conversion of Charge Current (IBAT) when VBAT > VBATSHORT
    uint8_t RES : 1; // Reserved: Always reads 0
} REG12;

typedef struct {
    uint8_t
        IDPM_LIM : 6; // Input Current Limit in effect while Input Current Optimizer (ICO) is enabled, offset: 100mA (default)
    bool IDPM_STAT : 1; // IINDPM Status
    bool VDPM_STAT : 1; // VINDPM Status
} REG13;

typedef struct {
    uint8_t DEV_REV : 2; // Device Revision
    bool TS_PROFILE : 1; // Temperature Profile
    uint8_t PN : 3; // Device Configuration
    bool ICO_OPTIMIZED : 1; // Input Current Optimizer (ICO) Status
    bool REG_RST : 1; // Register Reset
} REG14;
