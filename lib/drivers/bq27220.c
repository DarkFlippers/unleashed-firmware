#include "bq27220.h"
#include "bq27220_reg.h"

#include <api-hal-i2c.h>
#include <api-hal-delay.h>
#include <stdbool.h>

uint16_t bq27220_read_word(uint8_t address) {
    uint8_t buffer[2] = {address};
    uint16_t ret;
    with_api_hal_i2c(
        uint16_t, &ret, () {
            if(api_hal_i2c_trx(
                   POWER_I2C, BQ27220_ADDRESS, buffer, 1, buffer, 2, BQ27220_I2C_TIMEOUT)) {
                return *(uint16_t*)buffer;
            } else {
                return 0;
            }
        });
    return ret;
}

bool bq27220_control(uint16_t control) {
    bool ret;
    with_api_hal_i2c(
        bool, &ret, () {
            uint8_t buffer[3];
            buffer[0] = CommandControl;
            buffer[1] = control & 0xFF;
            buffer[2] = (control >> 8) & 0xFF;
            return api_hal_i2c_tx(POWER_I2C, BQ27220_ADDRESS, buffer, 3, BQ27220_I2C_TIMEOUT);
        });
    return ret;
}

uint8_t bq27220_get_checksum(uint8_t* data, uint16_t len) {
    uint8_t ret = 0;
    for(uint16_t i = 0; i < len; i++) {
        ret += data[i];
    }
    return 0xFF - ret;
}

bool bq27220_set_parameter_u16(uint16_t address, uint16_t value) {
    bool ret;
    uint8_t buffer[5];
    with_api_hal_i2c(
        bool, &ret, () {
            buffer[0] = CommandSelectSubclass;
            buffer[1] = address & 0xFF;
            buffer[2] = (address >> 8) & 0xFF;
            buffer[3] = (value >> 8) & 0xFF;
            buffer[4] = value & 0xFF;
            return api_hal_i2c_tx(POWER_I2C, BQ27220_ADDRESS, buffer, 5, BQ27220_I2C_TIMEOUT);
        });
    delay_us(10000);
    uint8_t checksum = bq27220_get_checksum(&buffer[1], 4);
    with_api_hal_i2c(
        bool, &ret, () {
            buffer[0] = CommandMACDataSum;
            buffer[1] = checksum;
            buffer[2] = 6;
            return api_hal_i2c_tx(POWER_I2C, BQ27220_ADDRESS, buffer, 3, BQ27220_I2C_TIMEOUT);
        });
    delay_us(10000);
    return ret;
}

bool bq27220_init(const ParamCEDV* cedv) {
    uint32_t timeout = 100;
    uint16_t design_cap = bq27220_get_design_capacity();
    if(cedv->design_cap == design_cap) {
        FURI_LOG_I("gauge", "Skip battery profile update");
        return true;
    }
    FURI_LOG_I("gauge", "Start updating battery profile");
    OperationStatus status = {};
    if(!bq27220_control(Control_ENTER_CFG_UPDATE)) {
        FURI_LOG_E("gauge", "Can't configure update");
        return false;
    };

    while((status.CFGUPDATE != 1) && (timeout-- > 0)) {
        bq27220_get_operation_status(&status);
    }
    bq27220_set_parameter_u16(AddressGaugingConfig, cedv->cedv_conf.gauge_conf_raw);
    bq27220_set_parameter_u16(AddressFullChargeCapacity, cedv->full_charge_cap);
    bq27220_set_parameter_u16(AddressDesignCapacity, cedv->design_cap);
    bq27220_set_parameter_u16(AddressEMF, cedv->EMF);
    bq27220_set_parameter_u16(AddressC0, cedv->C0);
    bq27220_set_parameter_u16(AddressR0, cedv->R0);
    bq27220_set_parameter_u16(AddressT0, cedv->T0);
    bq27220_set_parameter_u16(AddressR1, cedv->R1);
    bq27220_set_parameter_u16(AddressTC, (cedv->TC) << 8 | cedv->C1);
    bq27220_set_parameter_u16(AddressStartDOD0, cedv->DOD0);
    bq27220_set_parameter_u16(AddressStartDOD10, cedv->DOD10);
    bq27220_set_parameter_u16(AddressStartDOD20, cedv->DOD20);
    bq27220_set_parameter_u16(AddressStartDOD30, cedv->DOD30);
    bq27220_set_parameter_u16(AddressStartDOD40, cedv->DOD40);
    bq27220_set_parameter_u16(AddressStartDOD50, cedv->DOD40);
    bq27220_set_parameter_u16(AddressStartDOD60, cedv->DOD60);
    bq27220_set_parameter_u16(AddressStartDOD70, cedv->DOD70);
    bq27220_set_parameter_u16(AddressStartDOD80, cedv->DOD80);
    bq27220_set_parameter_u16(AddressStartDOD90, cedv->DOD90);
    bq27220_set_parameter_u16(AddressStartDOD100, cedv->DOD100);
    bq27220_set_parameter_u16(AddressEDV0, cedv->EDV0);
    bq27220_set_parameter_u16(AddressEDV1, cedv->EDV1);
    bq27220_set_parameter_u16(AddressEDV2, cedv->EDV2);

    bq27220_control(Control_EXIT_CFG_UPDATE);
    delay_us(10000);
    design_cap = bq27220_get_design_capacity();
    if(cedv->design_cap == design_cap) {
        FURI_LOG_I("gauge", "Battery profile update success");
        return true;
    } else {
        FURI_LOG_E("gauge", "Battery profile update failed");
        return false;
    }
}

uint16_t bq27220_get_voltage() {
    return bq27220_read_word(CommandVoltage);
}

int16_t bq27220_get_current() {
    return bq27220_read_word(CommandCurrent);
}

uint8_t bq27220_get_battery_status(BatteryStatus* battery_status) {
    uint16_t data = bq27220_read_word(CommandBatteryStatus);
    if(data == BQ27220_ERROR) {
        return BQ27220_ERROR;
    } else {
        *(uint16_t*)battery_status = data;
        return BQ27220_SUCCESS;
    }
}

uint8_t bq27220_get_operation_status(OperationStatus* operation_status) {
    uint16_t data = bq27220_read_word(CommandOperationStatus);
    if(data == BQ27220_ERROR) {
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

uint16_t bq27220_get_design_capacity() {
    return bq27220_read_word(CommandDesignCapacity);
}

uint16_t bq27220_get_remaining_capacity() {
    return bq27220_read_word(CommandRemainingCapacity);
}

uint16_t bq27220_get_state_of_charge() {
    return bq27220_read_word(CommandStateOfCharge);
}

uint16_t bq27220_get_state_of_health() {
    return bq27220_read_word(CommandStateOfHealth);
}
