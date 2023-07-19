#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <furi_hal_i2c.h>

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

_Static_assert(sizeof(BatteryStatus) == 2, "Incorrect structure size");

typedef struct {
    // Low byte, Low bit first
    bool CALMD : 1; /**< Calibration mode enabled */
    uint8_t SEC : 2; /**< Current security access */
    bool EDV2 : 1; /**< EDV2 threshold exceeded */
    bool VDQ : 1; /**< Indicates if Current discharge cycle is NOT qualified or qualified for an FCC updated */
    bool INITCOMP : 1; /**< gauge initialization is complete */
    bool SMTH : 1; /**< RemainingCapacity is scaled by smooth engine */
    bool BTPINT : 1; /**< BTP threshold has been crossed */
    // High byte, Low bit first
    uint8_t RSVD1 : 2;
    bool CFGUPDATE : 1; /**< Gauge is in CONFIG UPDATE mode */
    uint8_t RSVD0 : 5;
} OperationStatus;

_Static_assert(sizeof(OperationStatus) == 2, "Incorrect structure size");

typedef struct BQ27220DMData BQ27220DMData;

/** Initialize Driver
 * @return true on success, false otherwise
 */
bool bq27220_init(FuriHalI2cBusHandle* handle);

/** Initialize Driver
 * @return true on success, false otherwise
 */
bool bq27220_apply_data_memory(FuriHalI2cBusHandle* handle, const BQ27220DMData* data_memory);

/** Get battery voltage in mV or error */
uint16_t bq27220_get_voltage(FuriHalI2cBusHandle* handle);

/** Get current in mA or error*/
int16_t bq27220_get_current(FuriHalI2cBusHandle* handle);

/** Get battery status */
bool bq27220_get_battery_status(FuriHalI2cBusHandle* handle, BatteryStatus* battery_status);

/** Get operation status */
bool bq27220_get_operation_status(FuriHalI2cBusHandle* handle, OperationStatus* operation_status);

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
