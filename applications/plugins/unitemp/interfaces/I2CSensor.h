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
#ifndef UNITEMP_I2C
#define UNITEMP_I2C

#include "../unitemp.h"

#include <furi_hal_i2c.h>

//Структура I2C датчика
typedef struct I2CSensor {
    //Указатель на интерфейс I2C
    FuriHalI2cBusHandle* i2c;
    //Минимальный адрес устройства на шине I2C
    uint8_t minI2CAdr;
    //Максимальный адрес устройства на шине I2C
    uint8_t maxI2CAdr;
    //Текущий адрес устройства на шине I2C
    uint8_t currentI2CAdr;
    //Указатель на собственный экземпляр датчика
    void* sensorInstance;
} I2CSensor;

/**
 * @brief Заблокировать шину I2C
 * 
 * @param handle Указатель на шину
 */
void unitemp_i2c_acquire(FuriHalI2cBusHandle* handle);

/**
 * @brief Проверить наличие датчика на шине
 * 
 * @param i2c_sensor Указатель на датчик
 * @return Истина если устройство отозвалось
 */
bool unitemp_i2c_isDeviceReady(I2CSensor* i2c_sensor);

/**
 * @brief Выделение памяти для датчика на шине I2C
 * @param sensor Указатель на датчик
 * @param st Тип датчика
 * @return Истина если всё ок
 */
bool unitemp_I2C_sensor_alloc(Sensor* sensor, char* args);

/**
 * @brief Высвобождение памяти инстанса датчика
 * @param sensor Указатель на датчик
 */
bool unitemp_I2C_sensor_free(Sensor* sensor);

/**
 * @brief Обновить значение с датчка
 * @param sensor Указатель на датчик
 * @return Статус обновления
 */
UnitempStatus unitemp_I2C_sensor_update(Sensor* sensor);
/**
 * @brief Прочитать значение регистра reg
 * @param i2c_sensor Указатель на инстанс датчика
 * @param reg Номер регистра
 * @return Значение регистра
 */
uint8_t unitemp_i2c_readReg(I2CSensor* i2c_sensor, uint8_t reg);

/**
 * @brief Прочитать масссив значений из памяти
 * @param i2c_sensor Указатель на инстанс датчика
 * @param startReg Адрес регистра с которого начнётся чтение
 * @param len Количество байт для считывания из регистра
 * @param data Указатель на массив куда будут считаны данные
 * @return Истина если устройство вернуло данные
 */
bool unitemp_i2c_readRegArray(I2CSensor* i2c_sensor, uint8_t startReg, uint8_t len, uint8_t* data);

/**
 * @brief Записать значение в регистр
 * @param i2c_sensor Указатель на инстанс датчика
 * @param reg Номер регистра
 * @param value Значение для записи
 * @return Истина если значение записано
 */
bool unitemp_i2c_writeReg(I2CSensor* i2c_sensor, uint8_t reg, uint8_t value);

/**
 * @brief Записать масссив значений в память
 * @param i2c_sensor Указатель на инстанс датчика
 * @param startReg Адрес регистра с которого начнётся запись
 * @param len Количество байт для считывания из регистра
 * @param data Указатель на массив откуда будут записаны данные
 * @return Истина если устройство вернуло данные
 */
bool unitemp_i2c_writeRegArray(I2CSensor* i2c_sensor, uint8_t startReg, uint8_t len, uint8_t* data);

/**
 * @brief Прочитать массив данных по шине I2C
 * @param i2c_sensor Указатель на инстанс датчика
 * @param startReg Адрес регистра с которого начнётся чтение
 * @param data Указатель на массив куда будут считаны данные
 * @return Истина если устройство вернуло данные
 */
bool unitemp_i2c_readArray(I2CSensor* i2c_sensor, uint8_t len, uint8_t* data);

/**
 * @brief Записать масссив данных по шине I2C
 * @param i2c_sensor Указатель на инстанс датчика
 * @param len Количество байт для считывания из регистра
 * @param data Указатель на массив откуда будут записаны данные
 * @return Истина если устройство вернуло данные
 */
bool unitemp_i2c_writeArray(I2CSensor* i2c_sensor, uint8_t len, uint8_t* data);
#endif