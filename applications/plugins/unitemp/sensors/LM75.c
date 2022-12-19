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
#include "LM75.h"
#include "../interfaces/I2CSensor.h"

#define LM75_REG_TEMP 0x00
#define LM75_REG_CONFIG 0x01
#define LM75_REG_THYST 0x02
#define LM75_REG_TOS 0x03

#define LM75_CONFIG_SHUTDOWN 0b00000001
#define LM75_CONFIG_INTERRUPT 0b00000010
#define LM75_CONFIG_OSPOLARITY_HIGH 0b00000100
#define LM75_CONFIG_FAULTQUEUE_1 0b00000000
#define LM75_CONFIG_FAULTQUEUE_2 0b00001000
#define LM75_CONFIG_FAULTQUEUE_4 0b00010000
#define LM75_CONFIG_FAULTQUEUE_6 0b00011000

const SensorType LM75 = {
    .typename = "LM75",
    .interface = &I2C,
    .datatype = UT_DATA_TYPE_TEMP,
    .pollingInterval = 500,
    .allocator = unitemp_LM75_alloc,
    .mem_releaser = unitemp_LM75_free,
    .initializer = unitemp_LM75_init,
    .deinitializer = unitemp_LM75_deinit,
    .updater = unitemp_LM75_update};

bool unitemp_LM75_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Адреса на шине I2C (7 бит)
    i2c_sensor->minI2CAdr = 0b1001000 << 1;
    i2c_sensor->maxI2CAdr = 0b1001111 << 1;
    return true;
}

bool unitemp_LM75_free(Sensor* sensor) {
    //Нечего высвобождать, так как ничего не было выделено
    UNUSED(sensor);
    return true;
}

bool unitemp_LM75_init(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Выход если не удалось записать значение в датчик
    if(!unitemp_i2c_writeReg(i2c_sensor, LM75_REG_CONFIG, LM75_CONFIG_FAULTQUEUE_1)) return false;

    return true;
}

bool unitemp_LM75_deinit(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    if(!unitemp_i2c_writeReg(
           i2c_sensor, LM75_REG_CONFIG, LM75_CONFIG_FAULTQUEUE_1 | LM75_CONFIG_SHUTDOWN))
        return false;
    return true;
}

UnitempStatus unitemp_LM75_update(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    uint8_t buff[2];
    if(!unitemp_i2c_readRegArray(i2c_sensor, LM75_REG_TEMP, 2, buff))
        return UT_SENSORSTATUS_TIMEOUT;
    int16_t raw = (((uint16_t)buff[0] << 8) | buff[1]);
    sensor->temp = raw / 32 * 0.125;
    return UT_SENSORSTATUS_OK;
}
