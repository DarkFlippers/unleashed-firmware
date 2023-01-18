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
#ifndef UNITEMP_SPI
#define UNITEMP_SPI

#include "../unitemp.h"
#include <furi_hal_spi.h>

//Структура SPI датчика
typedef struct SPISensor {
    //Указатель на интерфейс SPI
    FuriHalSpiBusHandle* spi;
    //Порт подключения CS
    const GPIO* CS_pin;
} SPISensor;

/**
 * @brief Выделение памяти для датчика с интерфейсом SPI
 * @param sensor Указатель на датчик
 * @param args Указатель на массив аргументов с параметрами датчика
 * @return Истина если всё ок
 */
bool unitemp_spi_sensor_alloc(Sensor* sensor, char* args);

/**
 * @brief Высвобождение памяти инстанса датчика
 * @param sensor Указатель на датчик
 */
bool unitemp_spi_sensor_free(Sensor* sensor);

/**
 * @brief Инициализации датчика с интерфейсом one wire
 * @param sensor Указатель на датчик
 * @return Истина если инициализация упспешная
 */
bool unitemp_spi_sensor_init(Sensor* sensor);

/**
 * @brief Деинициализация датчика
 * @param sensor Указатель на датчик
 */
bool unitemp_spi_sensor_deinit(Sensor* sensor);

/**
 * @brief Обновить значение с датчка
 * @param sensor Указатель на датчик
 * @return Статус обновления
 */
UnitempStatus unitemp_spi_sensor_update(Sensor* sensor);

#endif