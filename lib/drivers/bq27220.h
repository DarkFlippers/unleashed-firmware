#pragma once

#include <stdint.h>
#include <stdbool.h>

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

/* Initialize Driver */
void bq27220_init();

/* Get battery voltage in mV or error */
uint16_t bq27220_get_voltage();

/* Get current in mA or error*/
int16_t bq27220_get_current();

/* Get battery status */
uint8_t bq27220_get_battery_status(BatteryStatus* battery_status);

/* Get operation status */
uint8_t bq27220_get_operation_status(OperationStatus* operation_status);

/* Get temperature in units of 0.1°K */
uint16_t bq27220_get_temperature();

/* Get compensated full charge capacity in in mAh */
uint16_t bq27220_get_full_charge_capacity();

/* Get remaining capacity in in mAh */
uint16_t bq27220_get_remaining_capacity();

/* Get predicted remaining battery capacity in percents */
uint16_t bq27220_get_state_of_charge();

/* Get ratio of full charge capacity over design capacity in percents */
uint16_t bq27220_get_state_of_health();
