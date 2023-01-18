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
#include "AM2320.h"
#include "../interfaces/I2CSensor.h"

const SensorType AM2320_I2C = {
    .typename = "AM2320_I2C",
    .altname = "AM2320 (I2C)",
    .interface = &I2C,
    .datatype = UT_TEMPERATURE | UT_HUMIDITY,
    .pollingInterval = 2000,
    .allocator = unitemp_AM2320_I2C_alloc,
    .mem_releaser = unitemp_AM2320_I2C_free,
    .initializer = unitemp_AM2320_init,
    .deinitializer = unitemp_AM2320_I2C_deinit,
    .updater = unitemp_AM2320_I2C_update};

static uint16_t AM2320_calc_CRC(uint8_t* ptr, uint8_t len) {
    uint16_t crc = 0xFFFF;
    uint8_t i;
    while(len--) {
        crc ^= *ptr++;
        for(i = 0; i < 8; i++) {
            if(crc & 0x01) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool unitemp_AM2320_I2C_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Адреса на шине I2C (7 бит)
    i2c_sensor->minI2CAdr = 0x5C << 1;
    i2c_sensor->maxI2CAdr = 0x5C << 1;
    return true;
}

bool unitemp_AM2320_I2C_free(Sensor* sensor) {
    //Нечего высвобождать, так как ничего не было выделено
    UNUSED(sensor);
    return true;
}

bool unitemp_AM2320_init(Sensor* sensor) {
    //Нечего инициализировать
    UNUSED(sensor);
    return true;
}

bool unitemp_AM2320_I2C_deinit(Sensor* sensor) {
    //Нечего деинициализировать
    UNUSED(sensor);
    return true;
}

UnitempStatus unitemp_AM2320_I2C_update(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    uint8_t data[8] = {0x03, 0x00, 0x04};

    //Wake up
    unitemp_i2c_isDeviceReady(i2c_sensor);
    furi_delay_ms(1);

    //Запрос
    if(!unitemp_i2c_writeArray(i2c_sensor, 3, data)) return UT_SENSORSTATUS_TIMEOUT;
    furi_delay_ms(2);
    //Ответ
    if(!unitemp_i2c_readArray(i2c_sensor, 8, data)) return UT_SENSORSTATUS_TIMEOUT;

    if(AM2320_calc_CRC(data, 6) != ((data[7] << 8) | data[6])) {
        return UT_SENSORSTATUS_BADCRC;
    }

    sensor->hum = (float)(((uint16_t)data[2] << 8) | data[3]) / 10;
    //Проверка на отрицательность температуры
    if(!(data[4] & (1 << 7))) {
        sensor->temp = (float)(((uint16_t)data[4] << 8) | data[5]) / 10;
    } else {
        data[4] &= ~(1 << 7);
        sensor->temp = (float)(((uint16_t)data[4] << 8) | data[5]) / -10;
    }
    return UT_SENSORSTATUS_OK;
}
