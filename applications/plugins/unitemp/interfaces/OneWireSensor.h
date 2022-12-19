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
#ifndef UNITEMP_OneWire
#define UNITEMP_OneWire

#include "../unitemp.h"

//Коды семейства устройств
typedef enum DallasFamilyCode {
    FC_DS18S20 = 0x10,
    FC_DS1822 = 0x22,
    FC_DS18B20 = 0x28,
} DallasFamilyCode;

//Режим питания датчка
typedef enum PowerMode {
    PWR_PASSIVE, //Питание от линии данных
    PWR_ACTIVE //Питание от источника питания
} PowerMode;

//Инстанс шины one wire
typedef struct {
    //Порт подключения датчика
    const GPIO* gpio;
    //Количество устройств на шине
    //Обновляется при ручном добавлении датчика на эту шину
    int8_t device_count;
    //Режим питания датчиков на шине
    PowerMode powerMode;
} OneWireBus;

//Инстанс датчика one wire
typedef struct OneWireSensor {
    //Указатель на шину OneWire
    OneWireBus* bus;
    //Текущий адрес устройства на шине OneWire
    uint8_t deviceID[8];
    //Код семейства устройств
    DallasFamilyCode familyCode;
} OneWireSensor;

/**
 * @brief Выделение памяти для датчика на шине OneWire
 * @param sensor Указатель на датчик
 * @param args Указатель на массив аргументов с параметрами датчика
 * @return Истина если всё ок
 */
bool unitemp_onewire_sensor_alloc(Sensor* sensor, char* args);

/**
 * @brief Высвобождение памяти инстанса датчика
 * @param sensor Указатель на датчик
 */
bool unitemp_onewire_sensor_free(Sensor* sensor);

/**
 * @brief Инициализации датчика на шине one wire
 * @param sensor Указатель на датчик
 * @return Истина если инициализация упспешная
 */
bool unitemp_onewire_sensor_init(Sensor* sensor);

/**
 * @brief Деинициализация датчика
 * @param sensor Указатель на датчик
 */
bool unitemp_onewire_sensor_deinit(Sensor* sensor);

/**
 * @brief Обновить значение с датчка
 * @param sensor Указатель на датчик
 * @return Статус обновления
 */
UnitempStatus unitemp_onewire_sensor_update(Sensor* sensor);

/**
 * @brief Выделение памяти для шины one wire и её инициализация
 * @param gpio Порт на котором необходимо создать шину
 * @return При успехе возвращает указатель на шину one wire
 */
OneWireBus* uintemp_onewire_bus_alloc(const GPIO* gpio);

/**
 * @brief Инициализация шины one wire
 * 
 * @param bus Указатель на шину
 * @return Истина если инициализация успешна
 */
bool unitemp_onewire_bus_init(OneWireBus* bus);

/**
 * @brief Деинициализация шины one wire
 * 
 * @param bus Указатель на шину
 * @return Истина если шина была деинициализирована, ложь если на шине остались устройства
 */
bool unitemp_onewire_bus_deinit(OneWireBus* bus);

/**
 * @brief Запуск общения с датчиками на шине one wire
 * @param bus Указатель на шину 
 * @return Истина если хотя бы одно устройство отозвалось
 */
bool unitemp_onewire_bus_start(OneWireBus* bus);

/**
 * @brief Отправить 1 бит данных на шину one wire
 * @param bus Указатель на шину
 * @param state Логический уровень
 */
void unitemp_onewire_bus_send_bit(OneWireBus* bus, bool state);

/**
 * @brief Запись байта на шину one wire
 * 
 * @param bus Указатель на шину one wire
 * @param data Записываемый байт
 */
void unitemp_onewire_bus_send_byte(OneWireBus* bus, uint8_t data);

/**
 * @brief Запись массива байт на шину one wire
 * 
 * @param bus Указатель на шину one wire
 * @param data Указатель на массив, откуда будут записаны данные
 * @param len Количество байт
 */
void unitemp_onewire_bus_send_byteArray(OneWireBus* bus, uint8_t* data, uint8_t len);

/**
 * @brief Чтение бита на шине one wire
 * 
 * @param bus Указатель на шину one wire
 * @return Логический уровень бита
 */
bool unitemp_onewire_bus_read_bit(OneWireBus* bus);

/**
 * @brief Чтение байта с шины One Wire
 * 
 * @param bus Указатель на шину one wire
 * @return Байт информации
 */
uint8_t unitemp_onewire_bus_read_byte(OneWireBus* bus);

/**
 * @brief Чтение массива байт с шины One Wire
 * 
 * @param bus Указатель на шину one wire
 * @param data Указатель на массив, куда будут записаны данные
 * @param len Количество байт
 */
void unitemp_onewire_bus_read_byteArray(OneWireBus* bus, uint8_t* data, uint8_t len);

/**
 * @brief Проверить контрольную сумму массива данных
 * 
 * @param data Указатель на массив данных
 * @param len Длина массива (включая байт CRC)
 * @return Истина если контрольная сумма корректная
 */
bool unitemp_onewire_CRC_check(uint8_t* data, uint8_t len);

/**
 * @brief Получить имя модели датчика на шине One Wire
 * 
 * @param sensor Указатель на датчик
 * @return Указатель на строку с названием
 */
char* unitemp_onewire_sensor_getModel(Sensor* sensor);

/**
 * @brief Чтение индификатора единственного датчика. ID запишется в инстанс датчика
 * 
 * @param instance Указатель на инстанс датчика
 * @return Истина, если код успешно прочитан, ложь если устройство отсутствует или устройств на шине больше одного
 */
bool unitemp_oneWire_sensor_readID(OneWireSensor* instance);

/**
 * @brief Команда выбора определённого датчка по его ID
 * @param instance Указатель на датчик one wire
 */
void unitemp_onewire_bus_select_sensor(OneWireSensor* instance);

/**
 * @brief Инициализация процесса поиска адресов на шине one wire
 */
void unitemp_onewire_bus_enum_init(void);

/**
 * @brief Перечисляет устройства на шине one wire и получает очередной адрес
 * @param bus Указатель на шину one wire
 * @return Возвращает указатель на буфер, содержащий восьмибайтовое значение адреса, либо NULL, если поиск завешён
 */
uint8_t* unitemp_onewire_bus_enum_next(OneWireBus* bus);

/**
 * @brief Сравнить ID датчиков
 * 
 * @param id1 Указатель на адрес первого датчика
 * @param id2 Указатель на адрес второго датчика
 * @return Истина если ID индентичны
 */
bool unitemp_onewire_id_compare(uint8_t* id1, uint8_t* id2);

extern const SensorType Dallas;
#endif