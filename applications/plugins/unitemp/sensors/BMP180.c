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
#include "BMP180.h"
#include "../interfaces/I2CSensor.h"

typedef struct {
    int16_t AC1;
    int16_t AC2;
    int16_t AC3;
    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;
    int16_t B1;
    int16_t B2;
    int16_t MB;
    int16_t MC;
    int16_t MD;
} BMP180_cal;

typedef struct {
    //Калибровочные значения
    BMP180_cal bmp180_cal;
} BMP180_instance;

const SensorType BMP180 = {
    .typename = "BMP180",
    .interface = &I2C,
    .datatype = UT_TEMPERATURE | UT_PRESSURE,
    .pollingInterval = 1000,
    .allocator = unitemp_BMP180_I2C_alloc,
    .mem_releaser = unitemp_BMP180_I2C_free,
    .initializer = unitemp_BMP180_init,
    .deinitializer = unitemp_BMP180_I2C_deinit,
    .updater = unitemp_BMP180_I2C_update};

bool unitemp_BMP180_I2C_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Адреса на шине I2C (7 бит)
    i2c_sensor->minI2CAdr = 0x77 << 1;
    i2c_sensor->maxI2CAdr = 0x77 << 1;

    BMP180_instance* bmx280_instance = malloc(sizeof(BMP180_instance));
    i2c_sensor->sensorInstance = bmx280_instance;
    return true;
}

bool unitemp_BMP180_I2C_free(Sensor* sensor) {
    UNUSED(sensor);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    free(i2c_sensor->sensorInstance);
    return true;
}

bool unitemp_BMP180_init(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;

    //Перезагрузка
    if(!unitemp_i2c_writeReg(i2c_sensor, 0xE0, 0xB6)) return false;
    furi_delay_ms(100);

    //Проверка ID
    uint8_t id = unitemp_i2c_readReg(i2c_sensor, 0xD0);
    if(id != 0x55) {
        FURI_LOG_E(
            APP_NAME, "Sensor %s returned wrong ID 0x%02X, expected 0x55", sensor->name, id);
        return false;
    }

    BMP180_instance* bmp180_instance = i2c_sensor->sensorInstance;

    uint8_t buff[22] = {0};

    //Чтение калибровочных значений
    if(!unitemp_i2c_readRegArray(i2c_sensor, 0xAA, 22, buff)) return false;
    bmp180_instance->bmp180_cal.AC1 = (buff[0] << 8) | buff[1];
    bmp180_instance->bmp180_cal.AC2 = (buff[2] << 8) | buff[3];
    bmp180_instance->bmp180_cal.AC3 = (buff[4] << 8) | buff[5];
    bmp180_instance->bmp180_cal.AC4 = (buff[6] << 8) | buff[7];
    bmp180_instance->bmp180_cal.AC5 = (buff[8] << 8) | buff[9];
    bmp180_instance->bmp180_cal.AC6 = (buff[10] << 8) | buff[11];
    bmp180_instance->bmp180_cal.B1 = (buff[12] << 8) | buff[13];
    bmp180_instance->bmp180_cal.B2 = (buff[14] << 8) | buff[15];
    bmp180_instance->bmp180_cal.MB = (buff[16] << 8) | buff[17];
    bmp180_instance->bmp180_cal.MC = (buff[18] << 8) | buff[19];
    bmp180_instance->bmp180_cal.MD = (buff[20] << 8) | buff[21];

    UNITEMP_DEBUG(
        "Sensor BMP180 (0x%02X) calibration values: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
        i2c_sensor->currentI2CAdr,
        bmp180_instance->bmp180_cal.AC1,
        bmp180_instance->bmp180_cal.AC2,
        bmp180_instance->bmp180_cal.AC3,
        bmp180_instance->bmp180_cal.AC4,
        bmp180_instance->bmp180_cal.AC5,
        bmp180_instance->bmp180_cal.AC6,
        bmp180_instance->bmp180_cal.B1,
        bmp180_instance->bmp180_cal.B2,
        bmp180_instance->bmp180_cal.MB,
        bmp180_instance->bmp180_cal.MC,
        bmp180_instance->bmp180_cal.MD);
    return true;
}

bool unitemp_BMP180_I2C_deinit(Sensor* sensor) {
    //Нечего деинициализировать
    UNUSED(sensor);
    return true;
}

UnitempStatus unitemp_BMP180_I2C_update(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    BMP180_instance* bmp180_instance = i2c_sensor->sensorInstance;

    //Чтение температуры
    if(!unitemp_i2c_writeReg(i2c_sensor, 0xF4, 0x2E)) return UT_SENSORSTATUS_TIMEOUT;
    furi_delay_ms(5);
    uint8_t buff[3] = {0};
    if(!unitemp_i2c_readRegArray(i2c_sensor, 0xF6, 2, buff)) return UT_SENSORSTATUS_TIMEOUT;
    int32_t UT = ((uint16_t)buff[0] << 8) + buff[1];
    int32_t X1 = (UT - bmp180_instance->bmp180_cal.AC6) * bmp180_instance->bmp180_cal.AC5 >> 15;
    int32_t X2 = (bmp180_instance->bmp180_cal.MC << 11) / (X1 + bmp180_instance->bmp180_cal.MD);
    int32_t B5 = X1 + X2;
    sensor->temp = ((B5 + 8) / 16) * 0.1f;

    //Чтение давления
    if(!unitemp_i2c_writeReg(i2c_sensor, 0xF4, 0x34 + (0b11 << 6))) return UT_SENSORSTATUS_TIMEOUT;
    furi_delay_ms(26);
    if(!unitemp_i2c_readRegArray(i2c_sensor, 0xF6, 3, buff)) return UT_SENSORSTATUS_TIMEOUT;
    uint32_t UP = ((buff[0] << 16) + (buff[1] << 8) + buff[2]) >> (8 - 0b11);

    int32_t B6, X3, B3, P;
    uint32_t B4, B7;
    B6 = B5 - 4000;
    X1 = (bmp180_instance->bmp180_cal.B2 * ((B6 * B6) >> 12)) >> 11;
    X2 = (bmp180_instance->bmp180_cal.AC2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = (((bmp180_instance->bmp180_cal.AC1 * 4 + X3) << 0b11) + 2) >> 2;
    X1 = (bmp180_instance->bmp180_cal.AC3 * B6) >> 13;
    X2 = (bmp180_instance->bmp180_cal.B1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = (bmp180_instance->bmp180_cal.AC4 * (unsigned long)(X3 + 32768)) >> 15;
    B7 = ((unsigned long)UP - B3) * (50000 >> 0b11);
    if(B7 < 0x80000000)
        P = (B7 * 2) / B4;
    else
        P = (B7 / B4) * 2;
    X1 = (P >> 8) * (P >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * (P)) >> 16;
    P = P + ((X1 + X2 + 3791) >> 4);
    sensor->pressure = P;

    return UT_SENSORSTATUS_OK;
}
