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
#include "HTU21x.h"
#include "../interfaces/I2CSensor.h"

const SensorType HTU21x = {
    .typename = "HTU21x",
    .altname = "HTU21x/SI70xx/SHT2x",
    .interface = &I2C,
    .datatype = UT_DATA_TYPE_TEMP_HUM,
    .pollingInterval = 250,
    .allocator = unitemp_HTU21x_alloc,
    .mem_releaser = unitemp_HTU21x_free,
    .initializer = unitemp_HTU21x_init,
    .deinitializer = unitemp_HTU21x_deinit,
    .updater = unitemp_HTU21x_update};

static uint8_t checkCRC(uint16_t data) {
    for(uint8_t i = 0; i < 16; i++) {
        if(data & 0x8000)
            data = (data << 1) ^ 0x13100;
        else
            data <<= 1;
    }
    return (data >> 8);
}

bool unitemp_HTU21x_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Адреса на шине I2C (7 бит)
    i2c_sensor->minI2CAdr = 0x40 << 1;
    i2c_sensor->maxI2CAdr = 0x41 << 1;
    return true;
}

bool unitemp_HTU21x_free(Sensor* sensor) {
    //Нечего высвобождать, так как ничего не было выделено
    UNUSED(sensor);
    return true;
}

bool unitemp_HTU21x_init(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    UNUSED(i2c_sensor);
    return true;
}

bool unitemp_HTU21x_deinit(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    UNUSED(i2c_sensor);
    return true;
}

UnitempStatus unitemp_HTU21x_update(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Датчик может быть всего один, так что норм
    static bool temp_hum = false;

    uint8_t data[3];

    if(sensor->status == UT_SENSORSTATUS_POLLING) {
        if(!unitemp_i2c_readArray(i2c_sensor, 3, data)) return UT_SENSORSTATUS_TIMEOUT;

        uint16_t raw = ((uint16_t)data[0] << 8) | data[1];
        if(checkCRC(raw) != data[2]) return UT_SENSORSTATUS_BADCRC;

        if(temp_hum) {
            sensor->temp = (0.002681f * raw - 46.85f);
        } else {
            sensor->hum = ((0.001907 * (raw ^ 0x02)) - 6);
        }
        temp_hum = !temp_hum;
        return UT_SENSORSTATUS_OK;
    }

    if(temp_hum) {
        //Запрос температуры
        data[0] = 0xF3;
        if(!unitemp_i2c_writeArray(i2c_sensor, 1, data)) return UT_SENSORSTATUS_TIMEOUT;
    } else {
        //Запрос влажности
        data[0] = 0xF5;
        if(!unitemp_i2c_writeArray(i2c_sensor, 1, data)) return UT_SENSORSTATUS_TIMEOUT;
    }

    return UT_SENSORSTATUS_POLLING;
}
