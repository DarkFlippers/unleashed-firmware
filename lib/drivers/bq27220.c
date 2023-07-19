
#include "bq27220.h"
#include "bq27220_reg.h"
#include "bq27220_data_memory.h"

_Static_assert(sizeof(BQ27220DMGaugingConfig) == 2, "Incorrect structure size");

#include <furi.h>
#include <stdbool.h>

#define TAG "Gauge"

static uint16_t bq27220_read_word(FuriHalI2cBusHandle* handle, uint8_t address) {
    uint16_t buf = 0;

    furi_hal_i2c_read_mem(
        handle, BQ27220_ADDRESS, address, (uint8_t*)&buf, 2, BQ27220_I2C_TIMEOUT);

    return buf;
}

static bool bq27220_control(FuriHalI2cBusHandle* handle, uint16_t control) {
    bool ret = furi_hal_i2c_write_mem(
        handle, BQ27220_ADDRESS, CommandControl, (uint8_t*)&control, 2, BQ27220_I2C_TIMEOUT);

    return ret;
}

static uint8_t bq27220_get_checksum(uint8_t* data, uint16_t len) {
    uint8_t ret = 0;
    for(uint16_t i = 0; i < len; i++) {
        ret += data[i];
    }
    return 0xFF - ret;
}

static bool bq27220_parameter_check(
    FuriHalI2cBusHandle* handle,
    uint16_t address,
    uint32_t value,
    size_t size,
    bool update) {
    furi_assert(size == 1 || size == 2 || size == 4);
    bool ret = false;
    uint8_t buffer[6] = {0};
    uint8_t old_data[4] = {0};

    do {
        buffer[0] = address & 0xFF;
        buffer[1] = (address >> 8) & 0xFF;

        for(size_t i = 0; i < size; i++) {
            buffer[1 + size - i] = (value >> (i * 8)) & 0xFF;
        }

        if(update) {
            if(!furi_hal_i2c_write_mem(
                   handle,
                   BQ27220_ADDRESS,
                   CommandSelectSubclass,
                   buffer,
                   size + 2,
                   BQ27220_I2C_TIMEOUT)) {
                FURI_LOG_I(TAG, "DM write failed");
                break;
            }

            furi_delay_us(10000);

            uint8_t checksum = bq27220_get_checksum(buffer, size + 2);
            buffer[0] = checksum;
            buffer[1] = 4 + size; // TODO: why 4?
            if(!furi_hal_i2c_write_mem(
                   handle, BQ27220_ADDRESS, CommandMACDataSum, buffer, 2, BQ27220_I2C_TIMEOUT)) {
                FURI_LOG_I(TAG, "CRC write failed");
                break;
            }

            furi_delay_us(10000);
            ret = true;
        } else {
            if(!furi_hal_i2c_write_mem(
                   handle, BQ27220_ADDRESS, CommandSelectSubclass, buffer, 2, BQ27220_I2C_TIMEOUT)) {
                FURI_LOG_I(TAG, "DM SelectSubclass for read failed");
                break;
            }

            if(!furi_hal_i2c_rx(handle, BQ27220_ADDRESS, old_data, size, BQ27220_I2C_TIMEOUT)) {
                FURI_LOG_I(TAG, "DM read failed");
                break;
            }

            if(*(uint32_t*)&(old_data[0]) != *(uint32_t*)&(buffer[2])) {
                FURI_LOG_W( //-V641
                    TAG,
                    "Data at 0x%04x(%zu): 0x%08lx!=0x%08lx",
                    address,
                    size,
                    *(uint32_t*)&(old_data[0]),
                    *(uint32_t*)&(buffer[2]));
            } else {
                ret = true;
            }
        }
    } while(0);

    return ret;
}

static bool bq27220_data_memory_check(
    FuriHalI2cBusHandle* handle,
    const BQ27220DMData* data_memory,
    bool update) {
    if(update) {
        if(!bq27220_control(handle, Control_ENTER_CFG_UPDATE)) {
            FURI_LOG_E(TAG, "ENTER_CFG_UPDATE command failed");
            return false;
        };

        // Wait for enter CFG update mode
        uint32_t timeout = 100;
        OperationStatus status = {0};
        while((status.CFGUPDATE != true) && (timeout-- > 0)) {
            bq27220_get_operation_status(handle, &status);
        }

        if(timeout == 0) {
            FURI_LOG_E(TAG, "CFGUPDATE mode failed");
            return false;
        }
    }

    // Process data memory records
    bool result = true;
    while(data_memory->type != BQ27220DMTypeEnd) {
        if(data_memory->type == BQ27220DMTypeWait) {
            furi_delay_us(data_memory->value.u32);
        } else if(data_memory->type == BQ27220DMTypeU8) {
            result &= bq27220_parameter_check(
                handle, data_memory->address, data_memory->value.u8, 1, update);
        } else if(data_memory->type == BQ27220DMTypeU16) {
            result &= bq27220_parameter_check(
                handle, data_memory->address, data_memory->value.u16, 2, update);
        } else if(data_memory->type == BQ27220DMTypeU32) {
            result &= bq27220_parameter_check(
                handle, data_memory->address, data_memory->value.u32, 4, update);
        } else if(data_memory->type == BQ27220DMTypeI8) {
            result &= bq27220_parameter_check(
                handle, data_memory->address, data_memory->value.i8, 1, update);
        } else if(data_memory->type == BQ27220DMTypeI16) {
            result &= bq27220_parameter_check(
                handle, data_memory->address, data_memory->value.i16, 2, update);
        } else if(data_memory->type == BQ27220DMTypeI32) {
            result &= bq27220_parameter_check(
                handle, data_memory->address, data_memory->value.i32, 4, update);
        } else if(data_memory->type == BQ27220DMTypeF32) {
            result &= bq27220_parameter_check(
                handle, data_memory->address, data_memory->value.u32, 4, update);
        } else if(data_memory->type == BQ27220DMTypePtr8) {
            result &= bq27220_parameter_check(
                handle, data_memory->address, *(uint8_t*)data_memory->value.u32, 1, update);
        } else if(data_memory->type == BQ27220DMTypePtr16) {
            result &= bq27220_parameter_check(
                handle, data_memory->address, *(uint16_t*)data_memory->value.u32, 2, update);
        } else if(data_memory->type == BQ27220DMTypePtr32) {
            result &= bq27220_parameter_check(
                handle, data_memory->address, *(uint32_t*)data_memory->value.u32, 4, update);
        } else {
            furi_crash("Invalid DM Type");
        }
        data_memory++;
    }

    // Finalize configuration update
    if(update) {
        bq27220_control(handle, Control_EXIT_CFG_UPDATE_REINIT);
        furi_delay_us(10000);
    }

    return result;
}

bool bq27220_init(FuriHalI2cBusHandle* handle) {
    // Request device number(chip PN)
    if(!bq27220_control(handle, Control_DEVICE_NUMBER)) {
        FURI_LOG_E(TAG, "Device is not present");
        return false;
    };
    // Check control response
    uint16_t data = 0;
    data = bq27220_read_word(handle, CommandControl);
    if(data != 0xFF00) {
        FURI_LOG_E(TAG, "Invalid control response: %x", data);
        return false;
    };

    data = bq27220_read_word(handle, CommandMACData);
    FURI_LOG_I(TAG, "Device Number %04x", data);

    return data == 0x0220;
}

bool bq27220_apply_data_memory(FuriHalI2cBusHandle* handle, const BQ27220DMData* data_memory) {
    FURI_LOG_I(TAG, "Verifying data memory");
    if(!bq27220_data_memory_check(handle, data_memory, false)) {
        FURI_LOG_I(TAG, "Updating data memory");
        bq27220_data_memory_check(handle, data_memory, true);
    }
    FURI_LOG_I(TAG, "Data memory verification complete");

    return true;
}

uint16_t bq27220_get_voltage(FuriHalI2cBusHandle* handle) {
    return bq27220_read_word(handle, CommandVoltage);
}

int16_t bq27220_get_current(FuriHalI2cBusHandle* handle) {
    return bq27220_read_word(handle, CommandCurrent);
}

bool bq27220_get_battery_status(FuriHalI2cBusHandle* handle, BatteryStatus* battery_status) {
    uint16_t data = bq27220_read_word(handle, CommandBatteryStatus);
    if(data == BQ27220_ERROR) {
        return false;
    } else {
        *(uint16_t*)battery_status = data;
        return true;
    }
}

bool bq27220_get_operation_status(FuriHalI2cBusHandle* handle, OperationStatus* operation_status) {
    uint16_t data = bq27220_read_word(handle, CommandOperationStatus);
    if(data == BQ27220_ERROR) {
        return false;
    } else {
        *(uint16_t*)operation_status = data;
        return true;
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
