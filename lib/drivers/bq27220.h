#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <furi-hal-i2c.h>

#define BQ27220_ERROR 0x0
#define BQ27220_SUCCESS 0x1

typedef struct {
    // Low byte, Low bit first
    bool DSG : 1; // The device is in DISCHARGE
    bool SYSDWN : 1; // System down bit indicating the system should shut down
    bool TDA : 1; // Terminate Discharge Alarm
    bool BATTPRES : 1; // Battery Present detected
    bool AUTH_GD : 1; // Detect inserted battery
    bool OCVGD : 1; // Good OCV measurement taken
    bool TCA : 1; // Terminate Charge Alarm
    bool RSVD : 1; // Reserved
    // High byte, Low bit first
    bool CHGINH : 1; // Charge inhibit
    bool FC : 1; // Full-charged is detected
    bool OTD : 1; // Overtemperature in discharge condition is detected
    bool OTC : 1; // Overtemperature in charge condition is detected
    bool SLEEP : 1; // Device is operating in SLEEP mode when set
    bool OCVFAIL : 1; // Status bit indicating that the OCV reading failed due to current
    bool OCVCOMP : 1; // An OCV measurement update is complete
    bool FD : 1; // Full-discharge is detected
} BatteryStatus;

typedef struct {
    // Low byte, Low bit first
    bool CALMD : 1;
    bool SEC0 : 1;
    bool SEC1 : 1;
    bool EDV2 : 1;
    bool VDQ : 1;
    bool INITCOMP : 1;
    bool SMTH : 1;
    bool BTPINT : 1;
    // High byte, Low bit first
    uint8_t RSVD1 : 2;
    bool CFGUPDATE : 1;
    uint8_t RSVD0 : 5;
} OperationStatus;

typedef struct {
    // Low byte, Low bit first
    bool CCT : 1;
    bool CSYNC : 1;
    bool RSVD0 : 1;
    bool EDV_CMP : 1;
    bool SC : 1;
    bool FIXED_EDV0 : 1;
    uint8_t RSVD1 : 2;
    // High byte, Low bit first
    bool FCC_LIM : 1;
    bool RSVD2 : 1;
    bool FC_FOR_VDQ : 1;
    bool IGNORE_SD : 1;
    bool SME0 : 1;
    uint8_t RSVD3 : 3;
} GaugingConfig;


typedef struct {
    union {
        GaugingConfig gauge_conf;
        uint16_t gauge_conf_raw;
    } cedv_conf;
    uint16_t full_charge_cap;
    uint16_t design_cap;
    uint16_t EDV0;
    uint16_t EDV1;
    uint16_t EDV2;
    uint16_t EMF;
    uint16_t C0;
    uint16_t R0;
    uint16_t T0;
    uint16_t R1;
    uint8_t TC;
    uint8_t C1;
    uint16_t DOD0;
    uint16_t DOD10;
    uint16_t DOD20;
    uint16_t DOD30;
    uint16_t DOD40;
    uint16_t DOD50;
    uint16_t DOD60;
    uint16_t DOD70;
    uint16_t DOD80;
    uint16_t DOD90;
    uint16_t DOD100;
} ParamCEDV;

/** Initialize Driver
 * @return true on success, false otherwise
 */
bool bq27220_init(FuriHalI2cBusHandle* handle, const ParamCEDV* cedv);

/** Get battery voltage in mV or error */
uint16_t bq27220_get_voltage(FuriHalI2cBusHandle* handle);

/** Get current in mA or error*/
int16_t bq27220_get_current(FuriHalI2cBusHandle* handle);

/** Get battery status */
uint8_t bq27220_get_battery_status(FuriHalI2cBusHandle* handle, BatteryStatus* battery_status);

/** Get operation status */
uint8_t bq27220_get_operation_status(FuriHalI2cBusHandle* handle, OperationStatus* operation_status);

/** Get temperature in units of 0.1°K */
uint16_t bq27220_get_temperature(FuriHalI2cBusHandle* handle);

/** Get compensated full charge capacity in in mAh */
uint16_t bq27220_get_full_charge_capacity(FuriHalI2cBusHandle* handle);

/** Get design capacity in mAh */
uint16_t bq27220_get_design_capacity(FuriHalI2cBusHandle* handle);

/** Get remaining capacity in in mAh */
uint16_t bq27220_get_remaining_capacity(FuriHalI2cBusHandle* handle);

/** Get predicted remaining battery capacity in percents */
uint16_t bq27220_get_state_of_charge(FuriHalI2cBusHandle* handle);

/** Get ratio of full charge capacity over design capacity in percents */
uint16_t bq27220_get_state_of_health(FuriHalI2cBusHandle* handle);

void bq27220_change_design_capacity(FuriHalI2cBusHandle* handle, uint16_t capacity);
