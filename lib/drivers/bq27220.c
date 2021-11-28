#include "bq27220.h"
#include "bq27220_reg.h"

#include <furi-hal-delay.h>
#include <furi/log.h>
#include <stdbool.h>

#define TAG "Gauge"

uint16_t bq27220_read_word(FuriHalI2cBusHandle* handle, uint8_t address) {
    uint8_t buffer[2] = {address};
    uint16_t ret = 0;

    if(furi_hal_i2c_trx(handle, BQ27220_ADDRESS, buffer, 1, buffer, 2, BQ27220_I2C_TIMEOUT)) {
        ret = *(uint16_t*)buffer;
    }

    return ret;
}

bool bq27220_control(FuriHalI2cBusHandle* handle, uint16_t control) {
    bool ret = false;
    uint8_t buffer[3];

    buffer[0] = CommandControl;
    buffer[1] = control & 0xFF;
    buffer[2] = (control >> 8) & 0xFF;
    ret = furi_hal_i2c_tx(handle, BQ27220_ADDRESS, buffer, 3, BQ27220_I2C_TIMEOUT);

    return ret;
}

uint8_t bq27220_get_checksum(uint8_t* data, uint16_t len) {
    uint8_t ret = 0;
    for(uint16_t i = 0; i < len; i++) {
        ret += data[i];
    }
    return 0xFF - ret;
}

bool bq27220_set_parameter_u16(FuriHalI2cBusHandle* handle, uint16_t address, uint16_t value) {
    bool ret;
    uint8_t buffer[5];

    buffer[0] = CommandSelectSubclass;
    buffer[1] = address & 0xFF;
    buffer[2] = (address >> 8) & 0xFF;
    buffer[3] = (value >> 8) & 0xFF;
    buffer[4] = value & 0xFF;
    ret = furi_hal_i2c_tx(handle, BQ27220_ADDRESS, buffer, 5, BQ27220_I2C_TIMEOUT);

    delay_us(10000);

    uint8_t checksum = bq27220_get_checksum(&buffer[1], 4);
    buffer[0] = CommandMACDataSum;
    buffer[1] = checksum;
    buffer[2] = 6;
    ret = furi_hal_i2c_tx(handle, BQ27220_ADDRESS, buffer, 3, BQ27220_I2C_TIMEOUT);

    delay_us(10000);
    return ret;
}

bool bq27220_init(FuriHalI2cBusHandle* handle, const ParamCEDV* cedv) {
    uint32_t timeout = 100;
    uint16_t design_cap = bq27220_get_design_capacity(handle);
    if(cedv->design_cap == design_cap) {
        FURI_LOG_I(TAG, "Skip battery profile update");
        return true;
    }
    FURI_LOG_I(TAG, "Start updating battery profile");
    OperationStatus status = {};
    if(!bq27220_control(handle, Control_ENTER_CFG_UPDATE)) {
        FURI_LOG_E(TAG, "Can't configure update");
        return false;
    };

    while((status.CFGUPDATE != 1) && (timeout-- > 0)) {
        bq27220_get_operation_status(handle, &status);
    }
    bq27220_set_parameter_u16(handle, AddressGaugingConfig, cedv->cedv_conf.gauge_conf_raw);
    bq27220_set_parameter_u16(handle, AddressFullChargeCapacity, cedv->full_charge_cap);
    bq27220_set_parameter_u16(handle, AddressDesignCapacity, cedv->design_cap);
    bq27220_set_parameter_u16(handle, AddressEMF, cedv->EMF);
    bq27220_set_parameter_u16(handle, AddressC0, cedv->C0);
    bq27220_set_parameter_u16(handle, AddressR0, cedv->R0);
    bq27220_set_parameter_u16(handle, AddressT0, cedv->T0);
    bq27220_set_parameter_u16(handle, AddressR1, cedv->R1);
    bq27220_set_parameter_u16(handle, AddressTC, (cedv->TC) << 8 | cedv->C1);
    bq27220_set_parameter_u16(handle, AddressStartDOD0, cedv->DOD0);
    bq27220_set_parameter_u16(handle, AddressStartDOD10, cedv->DOD10);
    bq27220_set_parameter_u16(handle, AddressStartDOD20, cedv->DOD20);
    bq27220_set_parameter_u16(handle, AddressStartDOD30, cedv->DOD30);
    bq27220_set_parameter_u16(handle, AddressStartDOD40, cedv->DOD40);
    bq27220_set_parameter_u16(handle, AddressStartDOD50, cedv->DOD40);
    bq27220_set_parameter_u16(handle, AddressStartDOD60, cedv->DOD60);
    bq27220_set_parameter_u16(handle, AddressStartDOD70, cedv->DOD70);
    bq27220_set_parameter_u16(handle, AddressStartDOD80, cedv->DOD80);
    bq27220_set_parameter_u16(handle, AddressStartDOD90, cedv->DOD90);
    bq27220_set_parameter_u16(handle, AddressStartDOD100, cedv->DOD100);
    bq27220_set_parameter_u16(handle, AddressEDV0, cedv->EDV0);
    bq27220_set_parameter_u16(handle, AddressEDV1, cedv->EDV1);
    bq27220_set_parameter_u16(handle, AddressEDV2, cedv->EDV2);

    bq27220_control(handle, Control_EXIT_CFG_UPDATE);
    delay_us(10000);
    design_cap = bq27220_get_design_capacity(handle);
    if(cedv->design_cap == design_cap) {
        FURI_LOG_I(TAG, "Battery profile update success");
        return true;
    } else {
        FURI_LOG_E(TAG, "Battery profile update failed");
        return false;
    }
}

uint16_t bq27220_get_voltage(FuriHalI2cBusHandle* handle) {
    return bq27220_read_word(handle, CommandVoltage);
}

int16_t bq27220_get_current(FuriHalI2cBusHandle* handle) {
    return bq27220_read_word(handle, CommandCurrent);
}

uint8_t bq27220_get_battery_status(FuriHalI2cBusHandle* handle, BatteryStatus* battery_status) {
    uint16_t data = bq27220_read_word(handle, CommandBatteryStatus);
    if(data == BQ27220_ERROR) {
        return BQ27220_ERROR;
    } else {
        *(uint16_t*)battery_status = data;
        return BQ27220_SUCCESS;
    }
}

uint8_t bq27220_get_operation_status(FuriHalI2cBusHandle* handle, OperationStatus* operation_status) {
    uint16_t data = bq27220_read_word(handle, CommandOperationStatus);
    if(data == BQ27220_ERROR) {
        return BQ27220_ERROR;
    } else {
        *(uint16_t*)operation_status = data;
        return BQ27220_SUCCESS;
    }
}

uint16_t bq27220_get_temperature(FuriHalI2cBusHandle* handle) {
    return bq27220_read_word(handle, CommandTemperature);
}

uint16_t bq27220_get_full_charge_capacity(FuriHalI2cBusHandle* handle) {
    return bq27220_read_word(handle, CommandFullChargeCapacity);
}

uint16_t bq27220_get_design_capacity(FuriHalI2cBusHandle* handle) {
    return bq27220_read_word(handle, CommandDesignCapacity);
}

uint16_t bq27220_get_remaining_capacity(FuriHalI2cBusHandle* handle) {
    return bq27220_read_word(handle, CommandRemainingCapacity);
}

uint16_t bq27220_get_state_of_charge(FuriHalI2cBusHandle* handle) {
    return bq27220_read_word(handle, CommandStateOfCharge);
}

uint16_t bq27220_get_state_of_health(FuriHalI2cBusHandle* handle) {
    return bq27220_read_word(handle, CommandStateOfHealth);
}
