#include "bq27220.h"
#include "bq27220_reg.h"

#include <api-hal-i2c.h>
#include <stdbool.h>

uint16_t bq27220_read_word(uint8_t address) {
    uint8_t buffer[2] = { address };
    uint16_t ret;
    with_api_hal_i2c(uint16_t, &ret, (){
        api_hal_i2c_trx(
            POWER_I2C, BQ27220_ADDRESS, 
            buffer, 1, buffer, 2
        );
        return *(uint16_t*)buffer;
    });
    return ret;
}

bool bq27220_control(uint16_t control) {
    bool ret;
    with_api_hal_i2c(bool, &ret, (){
        uint8_t buffer[3];
        buffer[0] = CommandControl;
        buffer[1] = (control>>8) & 0xFF;
        buffer[2] = control & 0xFF;
        api_hal_i2c_tx(POWER_I2C, BQ27220_ADDRESS, buffer, 3);
        return true;
    });
    return ret;
}

void bq27220_init() {
    bq27220_control(Control_ENTER_CFG_UPDATE);
    bq27220_control(Control_SET_PROFILE_2);
    bq27220_control(Control_EXIT_CFG_UPDATE);
}

uint16_t bq27220_get_voltage() {
    return bq27220_read_word(CommandVoltage);
}

int16_t bq27220_get_current() {
    return bq27220_read_word(CommandCurrent);
}

uint8_t bq27220_get_battery_status(BatteryStatus* battery_status) {
    uint16_t data = bq27220_read_word(CommandBatteryStatus);
    if (data == BQ27220_ERROR) {
        return BQ27220_ERROR;
    } else {
        *(uint16_t*)battery_status = data;
        return BQ27220_SUCCESS;
    }
}

uint8_t bq27220_get_operation_status(OperationStatus* operation_status) {
    uint16_t data = bq27220_read_word(CommandOperationStatus);
    if (data == BQ27220_ERROR) {
        return BQ27220_ERROR;
    } else {
        *(uint16_t*)operation_status = data;
        return BQ27220_SUCCESS;
    }
}

uint16_t bq27220_get_temperature() {
    return bq27220_read_word(CommandTemperature);
}

uint16_t bq27220_get_full_charge_capacity() {
    return bq27220_read_word(CommandFullChargeCapacity);
}

uint16_t bq27220_get_remaining_capacity() {
    return bq27220_read_word(CommandRemainingCapacity);
}

uint16_t bq27220_get_state_of_charge() {
    return bq27220_read_word(CommandStateOfCharge);
}
