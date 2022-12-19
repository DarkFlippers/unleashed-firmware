/*
    Unitemp - Universal temperature reader
    Copyright (C) 2022  Victor Nikitchuk (https://github.com/quen0n)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "I2CSensor.h"

static uint8_t sensors_count = 0;

void unitemp_i2c_acquire(FuriHalI2cBusHandle* handle) {
    furi_hal_i2c_acquire(handle);
    LL_GPIO_SetPinPull(gpio_ext_pc1.port, gpio_ext_pc1.pin, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinPull(gpio_ext_pc0.port, gpio_ext_pc0.pin, LL_GPIO_PULL_UP);
}

bool unitemp_i2c_isDeviceReady(I2CSensor* i2c_sensor) {
    unitemp_i2c_acquire(i2c_sensor->i2c);
    bool status = furi_hal_i2c_is_device_ready(i2c_sensor->i2c, i2c_sensor->currentI2CAdr, 10);
    furi_hal_i2c_release(i2c_sensor->i2c);
    return status;
}

uint8_t unitemp_i2c_readReg(I2CSensor* i2c_sensor, uint8_t reg) {
    //Блокировка шины
    unitemp_i2c_acquire(i2c_sensor->i2c);
    uint8_t buff[1] = {0};
    furi_hal_i2c_read_mem(i2c_sensor->i2c, i2c_sensor->currentI2CAdr, reg, buff, 1, 10);
    furi_hal_i2c_release(i2c_sensor->i2c);
    return buff[0];
}

bool unitemp_i2c_readArray(I2CSensor* i2c_sensor, uint8_t len, uint8_t* data) {
    unitemp_i2c_acquire(i2c_sensor->i2c);
    bool status = furi_hal_i2c_rx(i2c_sensor->i2c, i2c_sensor->currentI2CAdr, data, len, 10);
    furi_hal_i2c_release(i2c_sensor->i2c);
    return status;
}

bool unitemp_i2c_readRegArray(I2CSensor* i2c_sensor, uint8_t startReg, uint8_t len, uint8_t* data) {
    unitemp_i2c_acquire(i2c_sensor->i2c);
    bool status =
        furi_hal_i2c_read_mem(i2c_sensor->i2c, i2c_sensor->currentI2CAdr, startReg, data, len, 10);
    furi_hal_i2c_release(i2c_sensor->i2c);
    return status;
}

bool unitemp_i2c_writeReg(I2CSensor* i2c_sensor, uint8_t reg, uint8_t value) {
    //Блокировка шины
    unitemp_i2c_acquire(i2c_sensor->i2c);
    uint8_t buff[1] = {value};
    bool status =
        furi_hal_i2c_write_mem(i2c_sensor->i2c, i2c_sensor->currentI2CAdr, reg, buff, 1, 10);
    furi_hal_i2c_release(i2c_sensor->i2c);
    return status;
}

bool unitemp_i2c_writeArray(I2CSensor* i2c_sensor, uint8_t len, uint8_t* data) {
    unitemp_i2c_acquire(i2c_sensor->i2c);
    bool status = furi_hal_i2c_tx(i2c_sensor->i2c, i2c_sensor->currentI2CAdr, data, len, 10);
    furi_hal_i2c_release(i2c_sensor->i2c);
    return status;
}

bool unitemp_i2c_writeRegArray(I2CSensor* i2c_sensor, uint8_t startReg, uint8_t len, uint8_t* data) {
    //Блокировка шины
    unitemp_i2c_acquire(i2c_sensor->i2c);
    bool status = furi_hal_i2c_write_mem(
        i2c_sensor->i2c, i2c_sensor->currentI2CAdr, startReg, data, len, 10);
    furi_hal_i2c_release(i2c_sensor->i2c);
    return status;
}

bool unitemp_I2C_sensor_alloc(Sensor* sensor, char* args) {
    bool status = false;
    I2CSensor* instance = malloc(sizeof(I2CSensor));
    if(instance == NULL) {
        FURI_LOG_E(APP_NAME, "Sensor %s instance allocation error", sensor->name);
        return false;
    }
    instance->i2c = &furi_hal_i2c_handle_external;
    sensor->instance = instance;

    //Указание функций инициализации, деинициализации и обновления данных, а так же адреса на шине I2C
    status = sensor->type->allocator(sensor, args);
    int i2c_addr;
    sscanf(args, "%X", &i2c_addr);

    //Установка адреса шины I2C
    if(i2c_addr >= instance->minI2CAdr && i2c_addr <= instance->maxI2CAdr) {
        instance->currentI2CAdr = i2c_addr;
    } else {
        instance->currentI2CAdr = instance->minI2CAdr;
    }

    //Блокировка портов GPIO
    sensors_count++;
    unitemp_gpio_lock(unitemp_gpio_getFromInt(15), &I2C);
    unitemp_gpio_lock(unitemp_gpio_getFromInt(16), &I2C);

    return status;
}

bool unitemp_I2C_sensor_free(Sensor* sensor) {
    bool status = sensor->type->mem_releaser(sensor);
    free(sensor->instance);
    if(--sensors_count == 0) {
        unitemp_gpio_unlock(unitemp_gpio_getFromInt(15));
        unitemp_gpio_unlock(unitemp_gpio_getFromInt(16));
    }

    return status;
}

UnitempStatus unitemp_I2C_sensor_update(Sensor* sensor) {
    if(sensor->status != UT_SENSORSTATUS_OK) {
        sensor->type->initializer(sensor);
    }
    return sensor->type->updater(sensor);
}