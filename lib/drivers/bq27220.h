/** 
 * @file bq27220.h
 * 
 * Quite problematic chip with quite bad documentation.
 * 
 * Couple things to keep in mind:
 * 
 * - Datasheet and technical reference manual are full of bullshit
 * - bqstudio is ignoring them
 * - bqstudio i2c exchange tracing gives some ideas on timings that works, but there is a catch
 * - bqstudio timings contradicts to gm.fs file specification
 * - it's impossible to reproduce all situations in bqstudio
 * - experiments with blackbox can not cover all edge cases
 * - final timings are kinda blend between all of those sources
 * - device behavior differs depending on i2c clock speed
 * - The Hero Himmel would not have used this gauge in the first place
 * 
 * Couple advises if you'll need to modify this driver:
 * - Reset and wait for INITCOMP if something is not right.
 * - Do not do partial config update, it takes unpredictable amount of time to apply.
 * - Don't forget to reset chip before writing new config.
 * - If something fails at config update stage, wait for 4 seconds before doing next cycle.
 * - If you can program and lock chip at factory stage - do it. It will save you a lot of time.
 * - Keep sealed or strange things may happen.
 * - There is a condition when it may stuck at INITCOMP state, just "press reset button".
 * 
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <furi_hal_i2c.h>

#define BQ27220_ERROR   0x0
#define BQ27220_SUCCESS 0x1

typedef struct {
    // Low byte, Low bit first
    uint8_t BATT_ID : 3; /**< Battery Identification */
    bool SNOOZE     : 1; /**< SNOOZE mode is enabled */
    bool BCA        : 1; /**< fuel gauge board calibration routine is active */
    bool CCA        : 1; /**< Coulomb Counter Calibration routine is active */
    uint8_t RSVD0   : 2; /**< Reserved */
    // High byte, Low bit first
    uint8_t RSVD1; /**< Reserved */
} Bq27220ControlStatus;

_Static_assert(sizeof(Bq27220ControlStatus) == 2, "Incorrect Bq27220ControlStatus structure size");

typedef struct {
    // Low byte, Low bit first
    bool DSG      : 1; /**< The device is in DISCHARGE */
    bool SYSDWN   : 1; /**< System down bit indicating the system should shut down */
    bool TDA      : 1; /**< Terminate Discharge Alarm */
    bool BATTPRES : 1; /**< Battery Present detected */
    bool AUTH_GD  : 1; /**< Detect inserted battery */
    bool OCVGD    : 1; /**< Good OCV measurement taken */
    bool TCA      : 1; /**< Terminate Charge Alarm */
    bool RSVD     : 1; /**< Reserved */
    // High byte, Low bit first
    bool CHGINH   : 1; /**< Charge inhibit */
    bool FC       : 1; /**< Full-charged is detected */
    bool OTD      : 1; /**< Overtemperature in discharge condition is detected */
    bool OTC      : 1; /**< Overtemperature in charge condition is detected */
    bool SLEEP    : 1; /**< Device is operating in SLEEP mode when set */
    bool OCVFAIL  : 1; /**< Status bit indicating that the OCV reading failed due to current */
    bool OCVCOMP  : 1; /**< An OCV measurement update is complete */
    bool FD       : 1; /**< Full-discharge is detected */
} Bq27220BatteryStatus;

_Static_assert(sizeof(Bq27220BatteryStatus) == 2, "Incorrect Bq27220BatteryStatus structure size");

typedef enum {
    Bq27220OperationStatusSecSealed = 0b11,
    Bq27220OperationStatusSecUnsealed = 0b10,
    Bq27220OperationStatusSecFull = 0b01,
} Bq27220OperationStatusSec;

typedef struct {
    // Low byte, Low bit first
    bool CALMD  : 1; /**< Calibration mode enabled */
    uint8_t SEC : 2; /**< Current security access */
    bool EDV2   : 1; /**< EDV2 threshold exceeded */
    bool VDQ : 1; /**< Indicates if Current discharge cycle is NOT qualified or qualified for an FCC updated */
    bool INITCOMP  : 1; /**< gauge initialization is complete */
    bool SMTH      : 1; /**< RemainingCapacity is scaled by smooth engine */
    bool BTPINT    : 1; /**< BTP threshold has been crossed */
    // High byte, Low bit first
    uint8_t RSVD1  : 2; /**< Reserved */
    bool CFGUPDATE : 1; /**< Gauge is in CONFIG UPDATE mode */
    uint8_t RSVD0  : 5; /**< Reserved */
} Bq27220OperationStatus;

_Static_assert(
    sizeof(Bq27220OperationStatus) == 2,
    "Incorrect Bq27220OperationStatus structure size");

typedef struct {
    // Low byte, Low bit first
    bool FD       : 1; /**< Full Discharge */
    bool FC       : 1; /**< Full Charge */
    bool TD       : 1; /**< Terminate Discharge */
    bool TC       : 1; /**< Terminate Charge */
    bool RSVD0    : 1; /**< Reserved */
    bool EDV      : 1; /**< Cell voltage is above or below EDV0 threshold */
    bool DSG      : 1; /**< DISCHARGE or RELAXATION */
    bool CF       : 1; /**< Battery conditioning is needed */
    // High byte, Low bit first
    uint8_t RSVD1 : 2; /**< Reserved */
    bool FCCX     : 1; /**< fcc1hz clock going into CC: 0 = 1 Hz, 1 = 16 Hz*/
    uint8_t RSVD2 : 2; /**< Reserved */
    bool EDV1     : 1; /**< Cell voltage is above or below EDV1 threshold */
    bool EDV2     : 1; /**< Cell voltage is above or below EDV2 threshold */
    bool VDQ      : 1; /**< Charge cycle FCC update qualification */
} Bq27220GaugingStatus;

_Static_assert(sizeof(Bq27220GaugingStatus) == 2, "Incorrect Bq27220GaugingStatus structure size");

typedef struct BQ27220DMData BQ27220DMData;

/** Initialize Driver
 * 
 * This routine performs a lot of things under the hood:
 * - Verifies that gauge is present on i2c bus and got correct ID(0220)
 * - Unseals gauge
 * - Checks various internal statuses
 * - Checks that current profile is 0
 * - Checks configuration again provided data_memory
 * - Reset gauge if something on previous stages was fishy
 * - Updates configuration if needed
 * - Sealing gauge to prevent configuration and state from accidental damage
 *
 * @param      handle       The I2C Bus handle
 * @param[in]  data_memory  The data memory to be uploaded into gauge
 *
 * @return     true on success, false otherwise
 */
bool bq27220_init(FuriHalI2cBusHandle* handle, const BQ27220DMData* data_memory);

/** Reset gauge
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     true on success, false otherwise
 */
bool bq27220_reset(FuriHalI2cBusHandle* handle);

/** Seal gauge access
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     true on success, false otherwise
 */
bool bq27220_seal(FuriHalI2cBusHandle* handle);

/** Unseal gauge access
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     true on success, false otherwise
 */
bool bq27220_unseal(FuriHalI2cBusHandle* handle);

/** Get full access
 *
 * @warning    must be done in unsealed state
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     true on success, false otherwise
 */
bool bq27220_full_access(FuriHalI2cBusHandle* handle);

/** Get battery voltage
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     voltage in mV or BQ27220_ERROR
 */
uint16_t bq27220_get_voltage(FuriHalI2cBusHandle* handle);

/** Get current
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     current in mA or BQ27220_ERROR
 */
int16_t bq27220_get_current(FuriHalI2cBusHandle* handle);

/** Get control status
 *
 * @param      handle          The handle
 * @param      control_status  The control status
 *
 * @return     true on success, false otherwise
 */
bool bq27220_get_control_status(FuriHalI2cBusHandle* handle, Bq27220ControlStatus* control_status);

/** Get battery status
 *
 * @param      handle          The handle
 * @param      battery_status  The battery status
 *
 * @return     true on success, false otherwise
 */
bool bq27220_get_battery_status(FuriHalI2cBusHandle* handle, Bq27220BatteryStatus* battery_status);

/** Get operation status
 *
 * @param      handle            The handle
 * @param      operation_status  The operation status
 *
 * @return     true on success, false otherwise
 */
bool bq27220_get_operation_status(
    FuriHalI2cBusHandle* handle,
    Bq27220OperationStatus* operation_status);

/** Get gauging status
 *
 * @param      handle          The handle
 * @param      gauging_status  The gauging status
 *
 * @return     true on success, false otherwise
 */
bool bq27220_get_gauging_status(FuriHalI2cBusHandle* handle, Bq27220GaugingStatus* gauging_status);

/** Get temperature
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     temperature in units of 0.1°K
 */
uint16_t bq27220_get_temperature(FuriHalI2cBusHandle* handle);

/** Get compensated full charge capacity
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     full charge capacity in mAh or BQ27220_ERROR
 */
uint16_t bq27220_get_full_charge_capacity(FuriHalI2cBusHandle* handle);

/** Get design capacity
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     design capacity in mAh or BQ27220_ERROR
 */
uint16_t bq27220_get_design_capacity(FuriHalI2cBusHandle* handle);

/** Get remaining capacity
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     remaining capacity in mAh or BQ27220_ERROR
 */
uint16_t bq27220_get_remaining_capacity(FuriHalI2cBusHandle* handle);

/** Get predicted remaining battery capacity
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     state of charge in percents or BQ27220_ERROR
 */
uint16_t bq27220_get_state_of_charge(FuriHalI2cBusHandle* handle);

/** Get ratio of full charge capacity over design capacity
 *
 * @param      handle  The I2C Bus handle
 *
 * @return     state of health in percents or BQ27220_ERROR
 */
uint16_t bq27220_get_state_of_health(FuriHalI2cBusHandle* handle);
