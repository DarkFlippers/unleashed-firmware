/*
    Unitemp - Universal temperature reader
    Copyright (C) 2022-2023  Victor Nikitchuk (https://github.com/quen0n)

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
#include "SHT30.h"
#include "../interfaces/I2CSensor.h"

const SensorType SHT30 = {
    .typename = "SHT30",
    .altname = "SHT30/31/35",
    .interface = &I2C,
    .datatype = UT_TEMPERATURE | UT_HUMIDITY,
    .pollingInterval = 1000,
    .allocator = unitemp_SHT30_I2C_alloc,
    .mem_releaser = unitemp_SHT30_I2C_free,
    .initializer = unitemp_SHT30_init,
    .deinitializer = unitemp_SHT30_I2C_deinit,
    .updater = unitemp_SHT30_I2C_update};
const SensorType GXHT30 = {
    .typename = "GXHT30",
    .altname = "GXHT30/31/35",
    .interface = &I2C,
    .datatype = UT_TEMPERATURE | UT_HUMIDITY,
    .pollingInterval = 1000,
    .allocator = unitemp_SHT30_I2C_alloc,
    .mem_releaser = unitemp_SHT30_I2C_free,
    .initializer = unitemp_GXHT30_init,
    .deinitializer = unitemp_SHT30_I2C_deinit,
    .updater = unitemp_SHT30_I2C_update};

bool unitemp_SHT30_I2C_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Адреса на шине I2C (7 бит)
    i2c_sensor->minI2CAdr = 0x44 << 1;
    i2c_sensor->maxI2CAdr = 0x45 << 1;
    return true;
}

bool unitemp_SHT30_I2C_free(Sensor* sensor) {
    //Нечего высвобождать, так как ничего не было выделено
    UNUSED(sensor);
    return true;
}

bool unitemp_SHT30_init(Sensor* sensor) {
    UNUSED(sensor);
    return true;
}

bool unitemp_GXHT30_init(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    //Включение режима автоматического преобразования 2 раза в сек
    uint8_t data[2] = {0x22, 0x36};
    if(!unitemp_i2c_writeArray(i2c_sensor, 2, data)) return false;
    return true;
}

bool unitemp_SHT30_I2C_deinit(Sensor* sensor) {
    //Нечего деинициализировать
    UNUSED(sensor);
    return true;
}

UnitempStatus unitemp_SHT30_I2C_update(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    //Получение данных
    uint8_t data[6] = {0xE0, 0x00};
    if(!unitemp_i2c_writeArray(i2c_sensor, 2, data)) return UT_SENSORSTATUS_TIMEOUT;
    if(!unitemp_i2c_readArray(i2c_sensor, 6, data)) return UT_SENSORSTATUS_TIMEOUT;

    sensor->temp = -45 + 175 * (((uint16_t)(data[0] << 8) | data[1]) / 65535.0f);
    sensor->hum = 100 * (((uint16_t)(data[3] << 8) | data[4]) / 65535.0f);

    return UT_SENSORSTATUS_OK;
}
