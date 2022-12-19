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
#include "DHT20.h"
#include "../interfaces/I2CSensor.h"

const SensorType DHT20 = {
    .typename = "DHT20",
    .altname = "DHT20 (AM2108)",
    .interface = &I2C,
    .datatype = UT_TEMPERATURE | UT_HUMIDITY,
    .pollingInterval = 2000,
    .allocator = unitemp_DHT20_I2C_alloc,
    .mem_releaser = unitemp_DHT20_I2C_free,
    .initializer = unitemp_DHT20_init,
    .deinitializer = unitemp_DHT20_I2C_deinit,
    .updater = unitemp_DHT20_I2C_update};

bool unitemp_DHT20_I2C_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Адреса на шине I2C (7 бит)
    i2c_sensor->minI2CAdr = 0x38 << 1;
    i2c_sensor->maxI2CAdr = 0x38 << 1;
    return true;
}

bool unitemp_DHT20_I2C_free(Sensor* sensor) {
    //Нечего высвобождать, так как ничего не было выделено
    UNUSED(sensor);
    return true;
}

bool unitemp_DHT20_init(Sensor* sensor) {
    //Нечего инициализировать
    UNUSED(sensor);
    return true;
}

bool unitemp_DHT20_I2C_deinit(Sensor* sensor) {
    //Нечего деинициализировать
    UNUSED(sensor);
    return true;
}

UnitempStatus unitemp_DHT20_I2C_update(Sensor* sensor) {
    //I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    UNUSED(sensor);
    return UT_SENSORSTATUS_OK;
}
