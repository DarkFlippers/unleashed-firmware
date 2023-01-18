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
#include "DHT20.h"
#include "../interfaces/I2CSensor.h"

const SensorType DHT20 = {
    .typename = "DHT20",
    .altname = "DHT20/AM2108/AHT20",
    .interface = &I2C,
    .datatype = UT_TEMPERATURE | UT_HUMIDITY,
    .pollingInterval = 1000,
    .allocator = unitemp_DHT20_I2C_alloc,
    .mem_releaser = unitemp_DHT20_I2C_free,
    .initializer = unitemp_DHT20_init,
    .deinitializer = unitemp_DHT20_I2C_deinit,
    .updater = unitemp_DHT20_I2C_update};
const SensorType AHT10 = {
    .typename = "AHT10",
    .interface = &I2C,
    .datatype = UT_TEMPERATURE | UT_HUMIDITY,
    .pollingInterval = 1000,
    .allocator = unitemp_DHT20_I2C_alloc,
    .mem_releaser = unitemp_DHT20_I2C_free,
    .initializer = unitemp_DHT20_init,
    .deinitializer = unitemp_DHT20_I2C_deinit,
    .updater = unitemp_DHT20_I2C_update};

static uint8_t DHT20_get_status(I2CSensor* i2c_sensor) {
    uint8_t status[1] = {0};
    unitemp_i2c_readArray(i2c_sensor, 1, status);
    return status[0];
}

static uint8_t DHT20_calc_CRC8(uint8_t* message, uint8_t Num) {
    uint8_t i;
    uint8_t byte;
    uint8_t crc = 0xFF;
    for(byte = 0; byte < Num; byte++) {
        crc ^= (message[byte]);
        for(i = 8; i > 0; --i) {
            if(crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

static void DHT20_reset_reg(I2CSensor* i2c_sensor, uint8_t addr) {
    uint8_t data[3] = {addr, 0x00, 0x00};

    unitemp_i2c_writeArray(i2c_sensor, 3, data);

    furi_delay_ms(5);

    unitemp_i2c_readArray(i2c_sensor, 3, data);

    furi_delay_ms(10);

    data[0] = 0xB0 | addr;
    unitemp_i2c_writeArray(i2c_sensor, 3, data);
}

bool unitemp_DHT20_I2C_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Адреса на шине I2C (7 бит)
    i2c_sensor->minI2CAdr = 0x38 << 1;
    i2c_sensor->maxI2CAdr = (sensor->type == &DHT20) ? (0x38 << 1) : (0x39 << 1);
    return true;
}

bool unitemp_DHT20_I2C_free(Sensor* sensor) {
    //Нечего высвобождать, так как ничего не было выделено
    UNUSED(sensor);
    return true;
}

bool unitemp_DHT20_init(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    uint8_t data[3] = {0xA8, 0x00, 0x00};
    if(!unitemp_i2c_writeArray(i2c_sensor, 3, data)) return false;
    furi_delay_ms(10);
    data[0] = (sensor->type == &DHT20) ? 0xBE : 0xE1;
    data[1] = 0x08;
    if(!unitemp_i2c_writeArray(i2c_sensor, 3, data)) return false;
    furi_delay_ms(10);

    return true;
}

bool unitemp_DHT20_I2C_deinit(Sensor* sensor) {
    //Нечего деинициализировать
    UNUSED(sensor);
    return true;
}

UnitempStatus unitemp_DHT20_I2C_update(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    if(DHT20_get_status(i2c_sensor) != 0x18) {
        DHT20_reset_reg(i2c_sensor, 0x1B);
        DHT20_reset_reg(i2c_sensor, 0x1C);
        DHT20_reset_reg(i2c_sensor, 0x1E);
    }
    furi_delay_ms(10);

    uint8_t data[7] = {0xAC, 0x33, 0x00};
    if(!unitemp_i2c_writeArray(i2c_sensor, 3, data)) return UT_SENSORSTATUS_TIMEOUT;
    furi_delay_ms(80);
    uint32_t t = furi_get_tick();
    while(DHT20_get_status(i2c_sensor) == 0x80) {
        if(furi_get_tick() - t > 10) return UT_SENSORSTATUS_TIMEOUT;
    }

    if(!unitemp_i2c_readArray(i2c_sensor, 7, data)) return UT_SENSORSTATUS_TIMEOUT;

    if(DHT20_calc_CRC8(data, 6) != data[6]) {
        return UT_SENSORSTATUS_BADCRC;
    }
    uint32_t RetuData = 0;
    RetuData = (RetuData | data[1]) << 8;
    RetuData = (RetuData | data[2]) << 8;
    RetuData = (RetuData | data[3]);
    RetuData = RetuData >> 4;
    sensor->hum = RetuData * 100 * 10 / 1024.0f / 1024.0f / 10.0f;

    RetuData = 0;
    RetuData = (RetuData | data[3]) << 8;
    RetuData = (RetuData | data[4]) << 8;
    RetuData = (RetuData | data[5]);
    RetuData = RetuData & 0xfffff;
    sensor->temp = (RetuData * 200 * 10.0f / 1024.0f / 1024.0f - 500) / 10.0f;

    return UT_SENSORSTATUS_OK;
}
