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
#ifndef UNITEMP_SENSORS
#define UNITEMP_SENSORS
#include <furi.h>
#include <input/input.h>

//Маски бит для определения типов возвращаемых значений
#define UT_TEMPERATURE 0b00000001
#define UT_HUMIDITY 0b00000010
#define UT_PRESSURE 0b00000100
#define UT_CO2 0b00001000

//Статусы опроса датчика
typedef enum {
    UT_DATA_TYPE_TEMP = UT_TEMPERATURE,
    UT_DATA_TYPE_TEMP_HUM = UT_TEMPERATURE | UT_HUMIDITY,
    UT_DATA_TYPE_TEMP_PRESS = UT_TEMPERATURE | UT_PRESSURE,
    UT_DATA_TYPE_TEMP_HUM_PRESS = UT_TEMPERATURE | UT_HUMIDITY | UT_PRESSURE,
    UT_DATA_TYPE_TEMP_HUM_CO2 = UT_TEMPERATURE | UT_HUMIDITY | UT_CO2,
} SensorDataType;

//Типы возвращаемых данных
typedef enum {
    UT_SENSORSTATUS_OK, //Всё хорошо, опрос успешен
    UT_SENSORSTATUS_TIMEOUT, //Датчик не отозвался
    UT_SENSORSTATUS_EARLYPOOL, //Опрос раньше положенной задержки
    UT_SENSORSTATUS_BADCRC, //Неверная контрольная сумма
    UT_SENSORSTATUS_ERROR, //Прочие ошибки
    UT_SENSORSTATUS_POLLING, //В датчике происходит преобразование
    UT_SENSORSTATUS_INACTIVE, //Датчик на редактировании или удалён

} UnitempStatus;

//Порт ввода/вывода Flipper Zero
typedef struct GPIO {
    const uint8_t num;
    const char* name;
    const GpioPin* pin;
} GPIO;

typedef struct Sensor Sensor;

/**
 * @brief Указатель функции выделения памяти и подготовки экземпляра датчика
 */
typedef bool(SensorAllocator)(Sensor* sensor, char* args);
/**
 * @brief Указатель на функцию высвобождении памяти датчика
 */
typedef bool(SensorFree)(Sensor* sensor);
/**
 * @brief Указатель функции инициализации датчика
 */
typedef bool(SensorInitializer)(Sensor* sensor);
/**
 * @brief Указатель функции деинициализации датчика
 */
typedef bool(SensorDeinitializer)(Sensor* sensor);
/**
 * @brief Указатель функции обновления значения датчика
 */
typedef UnitempStatus(SensorUpdater)(Sensor* sensor);

//Типы подключения датчиков
typedef struct Interface {
    //Имя интерфейса
    const char* name;
    //Функция выделения памяти интерфейса
    SensorAllocator* allocator;
    //Функция высвыбождения памяти интерфейса
    SensorFree* mem_releaser;
    //Функция обновления значения датчика по интерфейсу
    SensorUpdater* updater;
} Interface;

//Типы датчиков
typedef struct {
    //Модель датчика
    const char* typename;
    //Полное имя с аналогами
    const char* altname;
    //Тип возвращаемых данных
    SensorDataType datatype;
    //Интерфейс подключения
    const Interface* interface;
    //Интервал опроса датчика
    uint16_t pollingInterval;
    //Функция выделения памяти для датчика
    SensorAllocator* allocator;
    //Функция высвыбождения памяти для датчика
    SensorFree* mem_releaser;
    //Функция инициализации датчика
    SensorInitializer* initializer;
    //Функция деинициализация датчика
    SensorDeinitializer* deinitializer;
    //Функция обновления значения датчка
    SensorUpdater* updater;
} SensorType;

//Датчик
typedef struct Sensor {
    //Имя датчика
    char* name;
    //Температура
    float temp;
    //Относительная влажность
    float hum;
    //Атмосферное давление
    float pressure;
    // Концентрация CO2
    float co2;
    //Тип датчика
    const SensorType* type;
    //Статус последнего опроса датчика
    UnitempStatus status;
    //Время последнего опроса датчика
    uint32_t lastPollingTime;
    //Смещение по температуре (x10)
    int8_t temp_offset;
    //Экземпляр датчика
    void* instance;
} Sensor;

extern const Interface SINGLE_WIRE; //Собственный однопроводной протокол датчиков DHTXX и AM23XX
extern const Interface ONE_WIRE; //Однопроводной протокол Dallas
extern const Interface I2C; //I2C_2 (PC0, PC1)
extern const Interface SPI; //SPI_1 (MOSI - 2, MISO - 3, CS - 4, SCK - 5)

/* ============================= Датчик(и) ============================= */
/**
 * @brief Выделение памяти под датчик
 * 
 * @param name Имя датчика
 * @param type Тип датчика
 * @param args Указатель на строку с парамерами датчика
 * @return Указатель на датчик в случае успешного выделения памяти, NULL при ошибке
 */
Sensor* unitemp_sensor_alloc(char* name, const SensorType* type, char* args);

/**
 * @brief Высвыбождение памяти конкретного датчка
 * @param sensor Указатель на датчик
 */
void unitemp_sensor_free(Sensor* sensor);

/**
 * @brief Обновление данных указанного датчика
 * @param sensor Указатель на датчик
 * @return Статус опроса датчика
 */
UnitempStatus unitemp_sensor_updateData(Sensor* sensor);

/**
 * @brief Проверка наличия датчика в памяти
 * 
 * @param sensor Указатель на датчик
 * @return Истина если этот датчик уже загружен, ложь если это новый датчик
 */
bool unitemp_sensor_isContains(Sensor* sensor);

/**
 * @brief Получить датчик из списка по индексу
 * 
 * @param index Индекс датчика (0 - unitemp_sensors_getCount())
 * @return Указатель на датчик при успехе, NULL при неудаче
 */
Sensor* unitemp_sensor_getActive(uint8_t index);

/**
 * @brief Загрузка датчиков с SD-карты
 * @return Истина если загрузка прошла успешно
 */
bool unitemp_sensors_load();

/**
 * @brief Функция перезагрузки датчиков с SD-карты
*/
void unitemp_sensors_reload(void);

/**
 * @brief Сохранение датчиков на SD-карту
 * @return Истина если сохранение прошло успешно
 */
bool unitemp_sensors_save(void);

/**
 * @brief Удаление датчика
 * 
 * @param sensor Указатель на датчик
 */
void unitemp_sensor_delete(Sensor* sensor);

/**
 * @brief Инициализация загруженных датчиков
 * @return Истина если всё прошло успешно
 */
bool unitemp_sensors_init(void);

/**
 * @brief Деинициализация загруженных датчиков
 * @return Истина если всё прошло успешно
 */
bool unitemp_sensors_deInit(void);

/**
 * @brief Высвыбождение памяти всех датчиков
 */
void unitemp_sensors_free(void);

/**
 * @brief Обновить данные всех датчиков
 */
void unitemp_sensors_updateValues(void);

/**
 * @brief Получить количество загруженных датчиков
 * @return Количество датчиков
 */
uint8_t unitemp_sensors_getCount(void);

/**
 * @brief Добавить датчик в общий список
 * @param sensor Указатель на датчик
 */
void unitemp_sensors_add(Sensor* sensor);

/**
* @brief Получить списк доступных типов датчиков
* @return Указатель на список датчиков
*/
const SensorType** unitemp_sensors_getTypes(void);

/**
* @brief Получить количество доступных типов датчиков
* @return Количество доступных типов датчиков
*/
uint8_t unitemp_sensors_getTypesCount(void);

/**
 * @brief Получить тип сенсора по его индексу
 * @param index Индекс типа датчика (от 0 до SENSOR_TYPES_COUNT)
 * @return const SensorType* 
 */
const SensorType* unitemp_sensors_getTypeFromInt(uint8_t index);

/**
 * @brief Преобразовать строчное название датчка в указатель
 * 
 * @param str Имя датчика в виде строки
 * @return Указатель на тип датчика при успехе, иначе NULL
 */
const SensorType* unitemp_sensors_getTypeFromStr(char* str);

/**
 * @brief Получить количество активных датчиков
 * 
 * @return Количество активных датчиков
 */
uint8_t unitemp_sensors_getActiveCount(void);

/* ============================= GPIO ============================= */
/**
 * @brief Конвертация номера порта на корпусе FZ в GPIO 
 * @param name Номер порта на корпусе FZ
 * @return Указатель на GPIO при успехе, NULL при ошибке
 */
const GPIO* unitemp_gpio_getFromInt(uint8_t name);
/**
 * @brief Конвертация GPIO в номер на корпусе FZ
 * @param gpio Указатель на порт
 * @return Номер порта на корпусе FZ
 */
uint8_t unitemp_gpio_toInt(const GPIO* gpio);

/**
 * @brief Блокировка GPIO указанным интерфейсом
 * @param gpio Указатель на порт
 * @param interface Указатель на интерфейс, которым порт будет занят
 */
void unitemp_gpio_lock(const GPIO* gpio, const Interface* interface);

/**
 * @brief Разблокировка порта
 * @param gpio Указатель на порт
 */
void unitemp_gpio_unlock(const GPIO* gpio);
/**
 * @brief Получить количество доступных портов для указанного интерфейса
 * @param interface Указатель на интерфейс
 * @return Количество доступных портов
 */
uint8_t unitemp_gpio_getAviablePortsCount(const Interface* interface, const GPIO* extraport);
/**
 * @brief Получить указатель на доступный для интерфейса порт по индексу 
 * @param interface Указатель на интерфейс
 * @param index Номер порта (от 0 до unitemp_gpio_getAviablePortsCount())
 * @param extraport Указатель на дополнительный порт, который будет принудительно считаться доступным. Можно указать NULL если не требуется
 * @return Указатель на доступный порт
 */
const GPIO*
    unitemp_gpio_getAviablePort(const Interface* interface, uint8_t index, const GPIO* extraport);

/* Датчики */
//DHTxx и их производные
#include "./interfaces/SingleWireSensor.h"
//DS18x2x
#include "./interfaces/OneWireSensor.h"
#include "./sensors/LM75.h"
//BMP280, BME280, BME680
#include "./sensors/BMx280.h"
#include "./sensors/BME680.h"
#include "./sensors/AM2320.h"
#include "./sensors/DHT20.h"
#include "./sensors/SHT30.h"
#include "./sensors/BMP180.h"
#include "./sensors/HTU21x.h"
#include "./sensors/HDC1080.h"
#include "./sensors/MAX31855.h"
#include "./sensors/MAX6675.h"
#include "./sensors/SCD30.h"
#include "./sensors/SCD40.h"
#endif
