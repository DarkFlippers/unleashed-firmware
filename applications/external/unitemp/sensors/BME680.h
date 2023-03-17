/*
    Unitemp - Universal temperature reader
    Copyright (C) 2022-2023  Victor Nikitchuk (https://github.com/quen0n)
    Contributed by g0gg0 (https://github.com/g3gg0)

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
#ifndef UNITEMP_BME680
#define UNITEMP_BME680

#include "../unitemp.h"
#include "../Sensors.h"
#include "../interfaces/I2CSensor.h"

typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
} BME680_temp_cal;

typedef struct {
    uint16_t dig_GH1;
    int16_t dig_GH2;
    int16_t dig_GH3;
} BME680_gas_cal;

typedef struct {
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    int16_t dig_P10;
} BME680_press_cal;

typedef struct {
    uint16_t dig_H1;
    uint16_t dig_H2;
    int8_t dig_H3;
    int8_t dig_H4;
    int8_t dig_H5;
    uint8_t dig_H6;
    int8_t dig_H7;
} BME680_hum_cal;

typedef struct {
    //Калибровочные значения температуры
    BME680_temp_cal temp_cal;
    //Калибровочные значения давления
    BME680_press_cal press_cal;
    //Калибровочные значения влажности воздуха
    BME680_hum_cal hum_cal;
    BME680_gas_cal gas_cal;
    //Время последнего обновления калибровочных значений
    uint32_t last_cal_update_time;
    //Индификатор датчика
    uint8_t chip_id;
    //Корректировочное значение температуры
    int32_t t_fine;
} BME680_instance;

extern const SensorType BMP280;
extern const SensorType BME680;
/**
 * @brief Выделение памяти и установка начальных значений датчика BMP280
 * @param sensor Указатель на создаваемый датчик
 * @return Истина при успехе
 */
bool unitemp_BME680_alloc(Sensor* sensor, char* args);

/**
 * @brief Инициализации датчика BMP280
 * @param sensor Указатель на датчик
 * @return Истина если инициализация упспешная
 */
bool unitemp_BME680_init(Sensor* sensor);

/**
 * @brief Деинициализация датчика
 * @param sensor Указатель на датчик
 */
bool unitemp_BME680_deinit(Sensor* sensor);

/**
 * @brief Обновление значений из датчика
 * @param sensor Указатель на датчик
 * @return Статус опроса датчика
 */
UnitempStatus unitemp_BME680_update(Sensor* sensor);

/**
 * @brief Высвободить память датчика
 * @param sensor Указатель на датчик
 */
bool unitemp_BME680_free(Sensor* sensor);

#endif