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
#include "BMx280.h"

const SensorType BMP280 = {
    .typename = "BMP280",
    .interface = &I2C,
    .datatype = UT_TEMPERATURE | UT_PRESSURE,
    .pollingInterval = 500,
    .allocator = unitemp_BMx280_alloc,
    .mem_releaser = unitemp_BMx280_free,
    .initializer = unitemp_BMx280_init,
    .deinitializer = unitemp_BMx280_deinit,
    .updater = unitemp_BMx280_update};
const SensorType BME280 = {
    .typename = "BME280",
    .interface = &I2C,
    .datatype = UT_TEMPERATURE | UT_HUMIDITY | UT_PRESSURE,

    .pollingInterval = 500,
    .allocator = unitemp_BMx280_alloc,
    .mem_releaser = unitemp_BMx280_free,
    .initializer = unitemp_BMx280_init,
    .deinitializer = unitemp_BMx280_deinit,
    .updater = unitemp_BMx280_update};

//Интервал обновления калибровочных значений
#define BOSCH_CAL_UPDATE_INTERVAL 60000

#define TEMP_CAL_START_ADDR 0x88
#define PRESS_CAL_START_ADDR 0x8E
#define HUM_CAL_H1_ADDR 0xA1
#define HUM_CAL_H2_ADDR 0xE1

#define BMP280_ID 0x58
#define BME280_ID 0x60

#define BMx280_I2C_ADDR_MIN (0x76 << 1)
#define BMx280_I2C_ADDR_MAX (0x77 << 1)

#define BMx280_REG_STATUS 0xF3
#define BMx280_REG_CTRL_MEAS 0xF4
#define BMx280_REG_CONFIG 0xF5
#define BME280_REG_CTRL_HUM 0xF2
//Преддескретизация температуры
#define BMx280_TEMP_OVERSAMPLING_SKIP 0b00000000
#define BMx280_TEMP_OVERSAMPLING_1 0b00100000
#define BMx280_TEMP_OVERSAMPLING_2 0b01000000
#define BMx280_TEMP_OVERSAMPLING_4 0b01100000
#define BMx280_TEMP_OVERSAMPLING_8 0b10000000
#define BMx280_TEMP_OVERSAMPLING_16 0b10100000
//Преддескретизация давления
#define BMx280_PRESS_OVERSAMPLING_SKIP 0b00000000
#define BMx280_PRESS_OVERSAMPLING_1 0b00000100
#define BMx280_PRESS_OVERSAMPLING_2 0b00001000
#define BMx280_PRESS_OVERSAMPLING_4 0b00001100
#define BMx280_PRESS_OVERSAMPLING_8 0b00010000
#define BMx280_PRESS_OVERSAMPLING_16 0b00010100
//Преддескретизация влажности
#define BME280_HUM_OVERSAMPLING_SKIP 0b00000000
#define BME280_HUM_OVERSAMPLING_1 0b00000001
#define BME280_HUM_OVERSAMPLING_2 0b00000010
#define BME280_HUM_OVERSAMPLING_4 0b00000011
#define BME280_HUM_OVERSAMPLING_8 0b00000100
#define BME280_HUM_OVERSAMPLING_16 0b00000101u
//Режимы работы датчика
#define BMx280_MODE_SLEEP 0b00000000 //Наелся и спит
#define BMx280_MODE_FORCED 0b00000001 //Обновляет значения 1 раз, после чего уходит в сон
#define BMx280_MODE_NORMAL 0b00000011 //Регулярно обновляет значения
//Период обновления в нормальном режиме
#define BMx280_STANDBY_TIME_0_5 0b00000000
#define BMx280_STANDBY_TIME_62_5 0b00100000
#define BMx280_STANDBY_TIME_125 0b01000000
#define BMx280_STANDBY_TIME_250 0b01100000
#define BMx280_STANDBY_TIME_500 0b10000000
#define BMx280_STANDBY_TIME_1000 0b10100000
#define BMx280_STANDBY_TIME_2000 0b11000000
#define BMx280_STANDBY_TIME_4000 0b11100000
//Коэффициент фильтрации значений
#define BMx280_FILTER_COEFF_1 0b00000000
#define BMx280_FILTER_COEFF_2 0b00000100
#define BMx280_FILTER_COEFF_4 0b00001000
#define BMx280_FILTER_COEFF_8 0b00001100
#define BMx280_FILTER_COEFF_16 0b00010000
//Разрешить работу по SPI
#define BMx280_SPI_3W_ENABLE 0b00000001
#define BMx280_SPI_3W_DISABLE 0b00000000

static float BMx280_compensate_temperature(I2CSensor* i2c_sensor, int32_t adc_T) {
    BMx280_instance* bmx280_instance = (BMx280_instance*)i2c_sensor->sensorInstance;
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)bmx280_instance->temp_cal.dig_T1 << 1))) *
            ((int32_t)bmx280_instance->temp_cal.dig_T2)) >>
           11;
    var2 = (((((adc_T >> 4) - ((int32_t)bmx280_instance->temp_cal.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)bmx280_instance->temp_cal.dig_T1))) >>
             12) *
            ((int32_t)bmx280_instance->temp_cal.dig_T3)) >>
           14;
    bmx280_instance->t_fine = var1 + var2;
    return ((bmx280_instance->t_fine * 5 + 128) >> 8) / 100.0f;
}

static float BMx280_compensate_pressure(I2CSensor* i2c_sensor, int32_t adc_P) {
    BMx280_instance* bmx280_instance = (BMx280_instance*)i2c_sensor->sensorInstance;

    int32_t var1, var2;
    uint32_t p;
    var1 = (((int32_t)bmx280_instance->t_fine) >> 1) - (int32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)bmx280_instance->press_cal.dig_P6);
    var2 = var2 + ((var1 * ((int32_t)bmx280_instance->press_cal.dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)bmx280_instance->press_cal.dig_P4) << 16);
    var1 = (((bmx280_instance->press_cal.dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) +
            ((((int32_t)bmx280_instance->press_cal.dig_P2) * var1) >> 1)) >>
           18;
    var1 = ((((32768 + var1)) * ((int32_t)bmx280_instance->press_cal.dig_P1)) >> 15);
    if(var1 == 0) {
        return 0; // avoid exception caused by division by zero
    }
    p = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;
    if(p < 0x80000000) {
        p = (p << 1) / ((uint32_t)var1);
    } else {
        p = (p / (uint32_t)var1) * 2;
    }
    var1 = (((int32_t)bmx280_instance->press_cal.dig_P9) *
            ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >>
           12;
    var2 = (((int32_t)(p >> 2)) * ((int32_t)bmx280_instance->press_cal.dig_P8)) >> 13;
    p = (uint32_t)((int32_t)p + ((var1 + var2 + bmx280_instance->press_cal.dig_P7) >> 4));
    return p;
}

static float BMx280_compensate_humidity(I2CSensor* i2c_sensor, int32_t adc_H) {
    BMx280_instance* bmx280_instance = (BMx280_instance*)i2c_sensor->sensorInstance;
    int32_t v_x1_u32r;
    v_x1_u32r = (bmx280_instance->t_fine - ((int32_t)76800));

    v_x1_u32r =
        (((((adc_H << 14) - (((int32_t)bmx280_instance->hum_cal.dig_H4) << 20) -
            (((int32_t)bmx280_instance->hum_cal.dig_H5) * v_x1_u32r)) +
           ((int32_t)16384)) >>
          15) *
         (((((((v_x1_u32r * ((int32_t)bmx280_instance->hum_cal.dig_H6)) >> 10) *
              (((v_x1_u32r * ((int32_t)bmx280_instance->hum_cal.dig_H3)) >> 11) +
               ((int32_t)32768))) >>
             10) +
            ((int32_t)2097152)) *
               ((int32_t)bmx280_instance->hum_cal.dig_H2) +
           8192) >>
          14));

    v_x1_u32r =
        (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
                       ((int32_t)bmx280_instance->hum_cal.dig_H1)) >>
                      4));

    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return ((uint32_t)(v_x1_u32r >> 12)) / 1024.0f;
}

static bool bmx280_readCalValues(I2CSensor* i2c_sensor) {
    BMx280_instance* bmx280_instance = (BMx280_instance*)i2c_sensor->sensorInstance;
    if(!unitemp_i2c_readRegArray(
           i2c_sensor, TEMP_CAL_START_ADDR, 6, (uint8_t*)&bmx280_instance->temp_cal))
        return false;
#ifdef UNITEMP_DEBUG
    FURI_LOG_D(
        APP_NAME,
        "Sensor BMx280 (0x%02X) T1-T3: %d, %d, %d",
        i2c_sensor->currentI2CAdr,
        bmx280_instance->temp_cal.dig_T1,
        bmx280_instance->temp_cal.dig_T2,
        bmx280_instance->temp_cal.dig_T3);
#endif

    if(!unitemp_i2c_readRegArray(
           i2c_sensor, PRESS_CAL_START_ADDR, 18, (uint8_t*)&bmx280_instance->press_cal))
        return false;
#ifdef UNITEMP_DEBUG
    FURI_LOG_D(
        APP_NAME,
        "Sensor BMx280 (0x%02X): P1-P9: %d, %d, %d, %d, %d, %d, %d, %d, %d",
        i2c_sensor->currentI2CAdr,
        bmx280_instance->press_cal.dig_P1,
        bmx280_instance->press_cal.dig_P2,
        bmx280_instance->press_cal.dig_P3,
        bmx280_instance->press_cal.dig_P4,
        bmx280_instance->press_cal.dig_P5,
        bmx280_instance->press_cal.dig_P6,
        bmx280_instance->press_cal.dig_P7,
        bmx280_instance->press_cal.dig_P8,
        bmx280_instance->press_cal.dig_P9);
#endif

    if(bmx280_instance->chip_id == BME280_ID) {
        uint8_t buff[7] = {0};
        if(!unitemp_i2c_readRegArray(i2c_sensor, HUM_CAL_H1_ADDR, 1, buff)) return false;
        bmx280_instance->hum_cal.dig_H1 = buff[0];

        if(!unitemp_i2c_readRegArray(i2c_sensor, HUM_CAL_H2_ADDR, 7, buff)) return false;
        bmx280_instance->hum_cal.dig_H2 = (uint16_t)(buff[0] | ((uint16_t)buff[1] << 8));
        bmx280_instance->hum_cal.dig_H3 = buff[2];
        bmx280_instance->hum_cal.dig_H4 = ((int16_t)buff[3] << 4) | (buff[4] & 0x0F);
        bmx280_instance->hum_cal.dig_H5 = (buff[4] & 0x0F) | ((int16_t)buff[5] << 4);
        bmx280_instance->hum_cal.dig_H6 = buff[6];

#ifdef UNITEMP_DEBUG
        FURI_LOG_D(
            APP_NAME,
            "Sensor BMx280 (0x%02X): H1-H6: %d, %d, %d, %d, %d, %d",
            i2c_sensor->currentI2CAdr,
            bmx280_instance->hum_cal.dig_H1,
            bmx280_instance->hum_cal.dig_H2,
            bmx280_instance->hum_cal.dig_H3,
            bmx280_instance->hum_cal.dig_H4,
            bmx280_instance->hum_cal.dig_H5,
            bmx280_instance->hum_cal.dig_H6);
#endif
    }

    bmx280_instance->last_cal_update_time = furi_get_tick();
    return true;
}
static bool bmp280_isMeasuring(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    return (bool)((unitemp_i2c_readReg(i2c_sensor, BMx280_REG_STATUS) & 0x08) >> 3);
}

bool unitemp_BMx280_alloc(Sensor* sensor, char* args) {
    UNUSED(args);
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    BMx280_instance* bmx280_instance = malloc(sizeof(BMx280_instance));
    if(bmx280_instance == NULL) {
        FURI_LOG_E(APP_NAME, "Failed to allocation sensor %s instance", sensor->name);
        return false;
    }

    if(sensor->type == &BMP280) bmx280_instance->chip_id = BMP280_ID;
    if(sensor->type == &BME280) bmx280_instance->chip_id = BME280_ID;

    i2c_sensor->sensorInstance = bmx280_instance;

    i2c_sensor->minI2CAdr = BMx280_I2C_ADDR_MIN;
    i2c_sensor->maxI2CAdr = BMx280_I2C_ADDR_MAX;
    return true;
}

bool unitemp_BMx280_init(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    //Перезагрузка
    unitemp_i2c_writeReg(i2c_sensor, 0xE0, 0xB6);
    //Чтение ID датчика
    uint8_t id = unitemp_i2c_readReg(i2c_sensor, 0xD0);
    if(id != BMP280_ID && id != BME280_ID) {
        FURI_LOG_E(
            APP_NAME,
            "Sensor %s returned wrong ID 0x%02X, expected 0x%02X or 0x%02X",
            sensor->name,
            id,
            BMP280_ID,
            BME280_ID);
        return false;
    }

    //Настройка режимов работы
    if(id == BME280_ID) {
        unitemp_i2c_writeReg(i2c_sensor, BME280_REG_CTRL_HUM, BME280_HUM_OVERSAMPLING_1);
        unitemp_i2c_writeReg(
            i2c_sensor, BME280_REG_CTRL_HUM, unitemp_i2c_readReg(i2c_sensor, BME280_REG_CTRL_HUM));
    }
    unitemp_i2c_writeReg(
        i2c_sensor,
        BMx280_REG_CTRL_MEAS,
        BMx280_TEMP_OVERSAMPLING_2 | BMx280_PRESS_OVERSAMPLING_4 | BMx280_MODE_NORMAL);
    //Настройка периода опроса и фильтрации значений
    unitemp_i2c_writeReg(
        i2c_sensor,
        BMx280_REG_CONFIG,
        BMx280_STANDBY_TIME_500 | BMx280_FILTER_COEFF_16 | BMx280_SPI_3W_DISABLE);
    //Чтение калибровочных значений
    if(!bmx280_readCalValues(i2c_sensor)) {
        FURI_LOG_E(APP_NAME, "Failed to read calibration values sensor %s", sensor->name);
        return false;
    }
    return true;
}

bool unitemp_BMx280_deinit(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    //Перевод в сон
    unitemp_i2c_writeReg(i2c_sensor, BMx280_REG_CTRL_MEAS, BMx280_MODE_SLEEP);
    return true;
}

UnitempStatus unitemp_BMx280_update(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    BMx280_instance* instance = i2c_sensor->sensorInstance;

    uint32_t t = furi_get_tick();

    uint8_t buff[3];
    //Проверка инициализированности датчика
    unitemp_i2c_readRegArray(i2c_sensor, 0xF4, 2, buff);
    if(buff[0] == 0) {
        FURI_LOG_W(APP_NAME, "Sensor %s is not initialized!", sensor->name);
        return UT_SENSORSTATUS_ERROR;
    }

    while(bmp280_isMeasuring(sensor)) {
        if(furi_get_tick() - t > 100) {
            return UT_SENSORSTATUS_TIMEOUT;
        }
    }

    if(furi_get_tick() - instance->last_cal_update_time > BOSCH_CAL_UPDATE_INTERVAL) {
        bmx280_readCalValues(i2c_sensor);
    }

    if(!unitemp_i2c_readRegArray(i2c_sensor, 0xFA, 3, buff)) return UT_SENSORSTATUS_TIMEOUT;
    int32_t adc_T = ((int32_t)buff[0] << 12) | ((int32_t)buff[1] << 4) | ((int32_t)buff[2] >> 4);
    if(!unitemp_i2c_readRegArray(i2c_sensor, 0xF7, 3, buff)) return UT_SENSORSTATUS_TIMEOUT;
    int32_t adc_P = ((int32_t)buff[0] << 12) | ((int32_t)buff[1] << 4) | ((int32_t)buff[2] >> 4);
    if(!unitemp_i2c_readRegArray(i2c_sensor, 0xFD, 2, buff)) return UT_SENSORSTATUS_TIMEOUT;
    int32_t adc_H = ((uint16_t)buff[0] << 8) | buff[1];
    sensor->temp = BMx280_compensate_temperature(i2c_sensor, adc_T);
    sensor->pressure = BMx280_compensate_pressure(i2c_sensor, adc_P);
    sensor->hum = BMx280_compensate_humidity(i2c_sensor, adc_H);
    return UT_SENSORSTATUS_OK;
}

bool unitemp_BMx280_free(Sensor* sensor) {
    I2CSensor* i2c_sensor = (I2CSensor*)sensor->instance;
    free(i2c_sensor->sensorInstance);
    return true;
}