/*
    Unitemp - Universal temperature reader
    Copyright (C) 2023  Victor Nikitchuk (https://github.com/quen0n)

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
#include "HDC1080.h"
#include "../interfaces/I2CSensor.h"

const SensorType HDC1080 = {
    .typename = "HDC1080",
    .interface = &I2C,
    .datatype = UT_DATA_TYPE_TEMP_HUM,
    .pollingInterval = 250,
    .allocator = unitemp_HDC1080_alloc,
    .mem_releaser = unitemp_HDC1080_free,
    .initializer = unitemp_HDC1080_init,
    .deinitializer = unitemp_HDC1080_deinit,
    .updater = unitemp_HDC1080_update};

bool unitemp_HDC1080_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Адреса на шине I2C (7 бит)
    i2c_sensor->minI2CAdr = 0x40 << 1;
    i2c_sensor->maxI2CAdr = 0x40 << 1;
    return true;
}

bool unitemp_HDC1080_free(Sensor* sensor) {
    //Нечего высвобождать, так как ничего не было выделено
    UNUSED(sensor);
    return true;
}

bool unitemp_HDC1080_init(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    uint8_t data[2];
    if(!unitemp_i2c_readRegArray(i2c_sensor, 0xFF, 2, data)) return UT_SENSORSTATUS_TIMEOUT;
    uint16_t device_id = ((uint16_t)data[0] << 8) | data[1];
    if(device_id != 0x1050) {
        FURI_LOG_E(
            APP_NAME,
            "Sensor %s returned wrong ID 0x%02X, expected 0x1050",
            sensor->name,
            device_id);
        return false;
    }
    data[0] = 0b0001000;
    data[1] = 0;
    //Установка режима работы и разрядности измерений
    if(!unitemp_i2c_writeRegArray(i2c_sensor, 0x02, 2, data)) return UT_SENSORSTATUS_TIMEOUT;

    return true;
}

bool unitemp_HDC1080_deinit(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    UNUSED(i2c_sensor);
    return true;
}

UnitempStatus unitemp_HDC1080_update(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    uint8_t data[2] = {0};
    //Запуск измерения
    if(!unitemp_i2c_writeArray(i2c_sensor, 1, data)) return UT_SENSORSTATUS_TIMEOUT;
    furi_delay_ms(10);
    if(!unitemp_i2c_readArray(i2c_sensor, 2, data)) return UT_SENSORSTATUS_TIMEOUT;

    sensor->temp = ((float)(((uint16_t)data[0] << 8) | data[1]) / 65536) * 165 - 40;

    data[0] = 1;
    if(!unitemp_i2c_writeArray(i2c_sensor, 1, data)) return UT_SENSORSTATUS_TIMEOUT;
    furi_delay_ms(10);
    if(!unitemp_i2c_readArray(i2c_sensor, 2, data)) return UT_SENSORSTATUS_TIMEOUT;
    sensor->hum = ((float)(((uint16_t)data[0] << 8) | data[1]) / 65536) * 100;

    return UT_SENSORSTATUS_OK;
}
