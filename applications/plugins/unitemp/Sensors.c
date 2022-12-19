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
#include "Sensors.h"
#include <furi_hal_power.h>
#include <m-string.h>

//Порты ввода/вывода, которые не были обозначены в общем списке
const GpioPin SWC_10 = {.pin = LL_GPIO_PIN_14, .port = GPIOA};
const GpioPin SIO_12 = {.pin = LL_GPIO_PIN_13, .port = GPIOA};
const GpioPin TX_13 = {.pin = LL_GPIO_PIN_6, .port = GPIOB};
const GpioPin RX_14 = {.pin = LL_GPIO_PIN_7, .port = GPIOB};

//Количество доступных портов ввода/вывода
#define GPIO_ITEMS (sizeof(GPIOList) / sizeof(GPIO))
//Количество интерфейсов
#define INTERFACES_TYPES_COUNT (int)(sizeof(interfaces) / sizeof(const Interface*))
//Количество типов датчиков
#define SENSOR_TYPES_COUNT (int)(sizeof(sensorTypes) / sizeof(const SensorType*))

//Перечень достуных портов ввода/вывода
static const GPIO GPIOList[] = {
    {2, "2 (A7)", &gpio_ext_pa7},
    {3, "3 (A6)", &gpio_ext_pa6},
    {4, "4 (A4)", &gpio_ext_pa4},
    {5, "5 (B3)", &gpio_ext_pb3},
    {6, "6 (B2)", &gpio_ext_pb2},
    {7, "7 (C3)", &gpio_ext_pc3},
    {10, " 10(SWC) ", &SWC_10},
    {12, "12 (SIO)", &SIO_12},
    {13, "13 (TX)", &TX_13},
    {14, "14 (RX)", &RX_14},
    {15, "15 (C1)", &gpio_ext_pc1},
    {16, "16 (C0)", &gpio_ext_pc0},
    {17, "17 (1W)", &ibutton_gpio}};

//Список интерфейсов, которые прикреплены к GPIO (определяется индексом)
//NULL - порт свободен, указатель на интерфейс - порт занят этим интерфейсом
static const Interface* gpio_interfaces_list[GPIO_ITEMS] = {0};

const Interface SINGLE_WIRE = {
    .name = "Single wire",
    .allocator = unitemp_singlewire_alloc,
    .mem_releaser = unitemp_singlewire_free,
    .updater = unitemp_singlewire_update};
const Interface I2C = {
    .name = "I2C",
    .allocator = unitemp_I2C_sensor_alloc,
    .mem_releaser = unitemp_I2C_sensor_free,
    .updater = unitemp_I2C_sensor_update};
const Interface ONE_WIRE = {
    .name = "One wire",
    .allocator = unitemp_onewire_sensor_alloc,
    .mem_releaser = unitemp_onewire_sensor_free,
    .updater = unitemp_onewire_sensor_update};

//Перечень интерфейсов подключения
//static const Interface* interfaces[] = {&SINGLE_WIRE, &I2C, &ONE_WIRE};
//Перечень датчиков
static const SensorType* sensorTypes[] =
    {&DHT11, &DHT12_SW, &DHT21, &DHT22, &AM2320_SW, &AM2320_I2C, &LM75, &BMP280, &BME280, &Dallas};

const SensorType* unitemp_sensors_getTypeFromInt(uint8_t index) {
    if(index > SENSOR_TYPES_COUNT) return NULL;
    return sensorTypes[index];
}

const SensorType* unitemp_sensors_getTypeFromStr(char* str) {
    UNUSED(str);
    if(str == NULL) return NULL;
    for(uint8_t i = 0; i < unitemp_sensors_getTypesCount(); i++) {
        if(!strcmp(str, sensorTypes[i]->typename)) {
            return sensorTypes[i];
        }
    }
    return NULL;
}

uint8_t unitemp_sensors_getTypesCount(void) {
    return SENSOR_TYPES_COUNT;
}
const SensorType** unitemp_sensors_getTypes(void) {
    return sensorTypes;
}

int unitemp_getIntFromType(const SensorType* type) {
    for(int i = 0; i < SENSOR_TYPES_COUNT; i++) {
        if(!strcmp(type->typename, sensorTypes[i]->typename)) {
            return i;
        }
    }
    return 255;
}
const GPIO* unitemp_gpio_getFromInt(uint8_t name) {
    for(uint8_t i = 0; i < GPIO_ITEMS; i++) {
        if(GPIOList[i].num == name) {
            return &GPIOList[i];
        }
    }
    return NULL;
}

const GPIO* unitemp_gpio_getFromIndex(uint8_t index) {
    return &GPIOList[index];
}

uint8_t unitemp_gpio_toInt(const GPIO* gpio) {
    if(gpio == NULL) return 255;
    for(uint8_t i = 0; i < GPIO_ITEMS; i++) {
        if(GPIOList[i].pin->pin == gpio->pin->pin && GPIOList[i].pin->port == gpio->pin->port) {
            return GPIOList[i].num;
        }
    }
    return 255;
}

uint8_t unitemp_gpio_to_index(const GpioPin* gpio) {
    if(gpio == NULL) return 255;
    for(uint8_t i = 0; i < GPIO_ITEMS; i++) {
        if(GPIOList[i].pin->pin == gpio->pin && GPIOList[i].pin->port == gpio->port) {
            return i;
        }
    }
    return 255;
}

uint8_t unitemp_gpio_getAviablePortsCount(const Interface* interface, const GPIO* extraport) {
    uint8_t aviable_ports_count = 0;
    for(uint8_t i = 0; i < GPIO_ITEMS; i++) {
        //Проверка для one wire
        if(interface == &ONE_WIRE) {
            if(((gpio_interfaces_list[i] == NULL || gpio_interfaces_list[i] == &ONE_WIRE) &&
                (i != 12)) || //Почему-то не работает на 17 порте
               (unitemp_gpio_getFromIndex(i) == extraport)) {
                aviable_ports_count++;
            }
        }

        //Проверка для single wire
        if(interface == &SINGLE_WIRE) {
            if(gpio_interfaces_list[i] == NULL || (unitemp_gpio_getFromIndex(i) == extraport)) {
                aviable_ports_count++;
            }
        }

        if(interface == &I2C) {
            //У I2C два фиксированых порта
            return 0;
        }
    }
    return aviable_ports_count;
}

void unitemp_gpio_lock(const GPIO* gpio, const Interface* interface) {
    uint8_t i = unitemp_gpio_to_index(gpio->pin);
    if(i == 255) return;
    gpio_interfaces_list[i] = interface;
}

void unitemp_gpio_unlock(const GPIO* gpio) {
    uint8_t i = unitemp_gpio_to_index(gpio->pin);
    if(i == 255) return;
    gpio_interfaces_list[i] = NULL;
}

const GPIO*
    unitemp_gpio_getAviablePort(const Interface* interface, uint8_t index, const GPIO* extraport) {
    //Проверка для I2C
    if(interface == &I2C) {
        if((gpio_interfaces_list[10] == NULL || gpio_interfaces_list[10] == &I2C) &&
           (gpio_interfaces_list[11] == NULL || gpio_interfaces_list[11] == &I2C)) {
            //Возврат истины
            return unitemp_gpio_getFromIndex(0);
        } else {
            //Возврат лжи
            return NULL;
        }
    }

    uint8_t aviable_index = 0;
    for(uint8_t i = 0; i < GPIO_ITEMS; i++) {
        //Проверка для one wire
        if(interface == &ONE_WIRE) {
            //Почему-то не работает на 17 порте
            if(((gpio_interfaces_list[i] == NULL || gpio_interfaces_list[i] == &ONE_WIRE) &&
                (i != 12)) || //Почему-то не работает на 17 порте
               (unitemp_gpio_getFromIndex(i) == extraport)) {
                if(aviable_index == index) {
                    return unitemp_gpio_getFromIndex(i);
                } else {
                    aviable_index++;
                }
            }
        }
        //Проверка для single wire
        if(interface == &SINGLE_WIRE) {
            if(gpio_interfaces_list[i] == NULL || unitemp_gpio_getFromIndex(i) == extraport) {
                if(aviable_index == index) {
                    return unitemp_gpio_getFromIndex(i);
                } else {
                    aviable_index++;
                }
            }
        }
    }

    return NULL;
}

void unitemp_sensor_delete(Sensor* sensor) {
    for(uint8_t i = 0; i < app->sensors_count; i++) {
        if(app->sensors[i] == sensor) {
            app->sensors[i]->status = UT_SENSORSTATUS_INACTIVE;
            unitemp_sensors_save();
            unitemp_sensors_reload();
            return;
        }
    }
}

Sensor* unitemp_sensor_getActive(uint8_t index) {
    uint8_t aviable_index = 0;
    for(uint8_t i = 0; i < app->sensors_count; i++) {
        if(app->sensors[i]->status != UT_SENSORSTATUS_INACTIVE) {
            if(aviable_index == index) {
                return app->sensors[i];
            } else {
                aviable_index++;
            }
        }
    }
    return NULL;
}

uint8_t unitemp_sensors_getCount(void) {
    if(app->sensors == NULL) return 0;
    return app->sensors_count;
}

uint8_t unitemp_sensors_getActiveCount(void) {
    if(app->sensors == NULL) return 0;
    uint8_t counter = 0;
    for(uint8_t i = 0; i < unitemp_sensors_getCount(); i++) {
        if(app->sensors[i]->status != UT_SENSORSTATUS_INACTIVE) counter++;
    }
    return counter;
}

void unitemp_sensors_add(Sensor* sensor) {
    app->sensors =
        (Sensor**)realloc(app->sensors, (unitemp_sensors_getCount() + 1) * sizeof(Sensor*));
    app->sensors[unitemp_sensors_getCount()] = sensor;
    app->sensors_count++;
}

bool unitemp_sensors_load(void) {
#ifdef UNITEMP_DEBUG
    FURI_LOG_D(APP_NAME, "Loading sensors...");
#endif

    //Выделение памяти на поток
    app->file_stream = file_stream_alloc(app->storage);

    //Переменная пути к файлу
    FuriString* filepath = furi_string_alloc();
    //Составление пути к файлу
    furi_string_printf(filepath, "%s/%s", APP_PATH_FOLDER, APP_FILENAME_SENSORS);

    //Открытие потока к файлу с датчиками
    if(!file_stream_open(
           app->file_stream, furi_string_get_cstr(filepath), FSAM_READ_WRITE, FSOM_OPEN_EXISTING)) {
        if(file_stream_get_error(app->file_stream) == FSE_NOT_EXIST) {
            FURI_LOG_W(APP_NAME, "Missing sensors file");
            //Закрытие потока и освобождение памяти
            file_stream_close(app->file_stream);
            stream_free(app->file_stream);
            return false;
        } else {
            FURI_LOG_E(
                APP_NAME,
                "An error occurred while loading the sensors file: %d",
                file_stream_get_error(app->file_stream));
            //Закрытие потока и освобождение памяти
            file_stream_close(app->file_stream);
            stream_free(app->file_stream);
            return false;
        }
    }

    //Вычисление размера файла
    uint16_t file_size = stream_size(app->file_stream);
    //Если файл пустой, то:
    if(file_size == (uint8_t)0) {
        FURI_LOG_W(APP_NAME, "Sensors file is empty");
        //Закрытие потока и освобождение памяти
        file_stream_close(app->file_stream);
        stream_free(app->file_stream);
        return false;
    }
    //Выделение памяти под загрузку файла
    uint8_t* file_buf = malloc(file_size);
    //Опустошение буфера файла
    memset(file_buf, 0, file_size);
    //Загрузка файла
    if(stream_read(app->file_stream, file_buf, file_size) != file_size) {
        //Выход при ошибке чтения
        FURI_LOG_E(APP_NAME, "Error reading sensors file");
        //Закрытие потока и освобождение памяти
        file_stream_close(app->file_stream);
        stream_free(app->file_stream);
        free(file_buf);
        return false;
    }

    //Указатель на начало строки
    FuriString* file = furi_string_alloc_set_str((char*)file_buf);
    //Сколько байт до конца строки
    size_t line_end = 0;

    while(line_end != STRING_FAILURE && line_end != (size_t)(file_size - 1)) {
        //Имя датчика
        char name[11] = {0};
        //Тип датчика
        char type[11] = {0};
        //Смещение по температуре
        int temp_offset = 0;
        //Смещение по строке для отделения аргументов
        int offset = 0;
        //Чтение из строки
        sscanf(((char*)(file_buf + line_end)), "%s %s %d %n", name, type, &temp_offset, &offset);
        //Ограничение длины имени
        name[10] = '\0';

        //Замена ? на пробел
        for(uint8_t i = 0; i < 10; i++) {
            if(name[i] == '?') name[i] = ' ';
        }

        char* args = ((char*)(file_buf + line_end + offset));
        const SensorType* stype = unitemp_sensors_getTypeFromStr(type);

        //Проверка типа датчика
        if(stype != NULL && sizeof(name) > 0 && sizeof(name) <= 11) {
            Sensor* sensor =
                unitemp_sensor_alloc(name, unitemp_sensors_getTypeFromStr(type), args);
            if(sensor != NULL) {
                sensor->temp_offset = temp_offset;
                unitemp_sensors_add(sensor);
            } else {
                FURI_LOG_E(APP_NAME, "Failed sensor (%s:%s) mem allocation", name, type);
            }
        } else {
            FURI_LOG_E(APP_NAME, "Unsupported sensor name (%s) or sensor type (%s)", name, type);
        }
        //Вычисление конца строки
        line_end = furi_string_search_char(file, '\n', line_end + 1);
    }

    free(file_buf);
    file_stream_close(app->file_stream);
    stream_free(app->file_stream);

    FURI_LOG_I(APP_NAME, "Sensors have been successfully loaded");
    return true;
}

bool unitemp_sensors_save(void) {
#ifdef UNITEMP_DEBUG
    FURI_LOG_D(APP_NAME, "Saving sensors...");
#endif

    //Выделение памяти для потока
    app->file_stream = file_stream_alloc(app->storage);

    //Переменная пути к файлу
    FuriString* filepath = furi_string_alloc();
    //Составление пути к файлу
    furi_string_printf(filepath, "%s/%s", APP_PATH_FOLDER, APP_FILENAME_SENSORS);
    //Создание папки плагина
    storage_common_mkdir(app->storage, APP_PATH_FOLDER);
    //Открытие потока
    if(!file_stream_open(
           app->file_stream, furi_string_get_cstr(filepath), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS)) {
        FURI_LOG_E(
            APP_NAME,
            "An error occurred while saving the sensors file: %d",
            file_stream_get_error(app->file_stream));
        //Закрытие потока и освобождение памяти
        file_stream_close(app->file_stream);
        stream_free(app->file_stream);
        return false;
    }

    //Сохранение датчиков
    for(uint8_t i = 0; i < unitemp_sensors_getActiveCount(); i++) {
        Sensor* sensor = unitemp_sensor_getActive(i);
        //Замена пробела на ?
        for(uint8_t i = 0; i < 10; i++) {
            if(sensor->name[i] == ' ') sensor->name[i] = '?';
        }

        stream_write_format(
            app->file_stream,
            "%s %s %d ",
            sensor->name,
            sensor->type->typename,
            sensor->temp_offset);

        if(sensor->type->interface == &SINGLE_WIRE) {
            stream_write_format(
                app->file_stream, "%d\n", unitemp_singlewire_sensorGetGPIO(sensor)->num);
        }
        if(sensor->type->interface == &I2C) {
            stream_write_format(
                app->file_stream, "%X\n", ((I2CSensor*)sensor->instance)->currentI2CAdr);
        }
        if(sensor->type->interface == &ONE_WIRE) {
            stream_write_format(
                app->file_stream,
                "%d %02X%02X%02X%02X%02X%02X%02X%02X\n",
                ((OneWireSensor*)sensor->instance)->bus->gpio->num,
                ((OneWireSensor*)sensor->instance)->deviceID[0],
                ((OneWireSensor*)sensor->instance)->deviceID[1],
                ((OneWireSensor*)sensor->instance)->deviceID[2],
                ((OneWireSensor*)sensor->instance)->deviceID[3],
                ((OneWireSensor*)sensor->instance)->deviceID[4],
                ((OneWireSensor*)sensor->instance)->deviceID[5],
                ((OneWireSensor*)sensor->instance)->deviceID[6],
                ((OneWireSensor*)sensor->instance)->deviceID[7]);
        }
    }

    //Закрытие потока и освобождение памяти
    file_stream_close(app->file_stream);
    stream_free(app->file_stream);

    FURI_LOG_I(APP_NAME, "Sensors have been successfully saved");
    return true;
}
void unitemp_sensors_reload(void) {
    unitemp_sensors_deInit();
    unitemp_sensors_free();

    unitemp_sensors_load();
    unitemp_sensors_init();
}

bool unitemp_sensor_isContains(Sensor* sensor) {
    for(uint8_t i = 0; i < unitemp_sensors_getCount(); i++) {
        if(app->sensors[i] == sensor) return true;
    }
    return false;
}

Sensor* unitemp_sensor_alloc(char* name, const SensorType* type, char* args) {
    if(name == NULL || type == NULL) return NULL;
    bool status = false;
    //Выделение памяти под датчик
    Sensor* sensor = malloc(sizeof(Sensor));
    if(sensor == NULL) {
        FURI_LOG_E(APP_NAME, "Sensor %s allocation error", name);
        return false;
    }

    //Выделение памяти под имя
    sensor->name = malloc(11);
    if(sensor->name == NULL) {
        FURI_LOG_E(APP_NAME, "Sensor %s name allocation error", name);
        return false;
    }
    //Запись имени датчка
    strcpy(sensor->name, name);
    //Тип датчика
    sensor->type = type;
    //Статус датчика по умолчанию - ошибка
    sensor->status = UT_SENSORSTATUS_ERROR;
    //Время последнего опроса
    sensor->lastPollingTime =
        furi_get_tick() - 10000; //чтобы первый опрос произошёл как можно раньше

    sensor->temp = -128.0f;
    sensor->hum = -128.0f;
    sensor->pressure = -128.0f;
    sensor->temp_offset = 0;
    //Выделение памяти под инстанс датчика в зависимости от его интерфейса
    status = sensor->type->interface->allocator(sensor, args);

    //Выход если датчик успешно развёрнут
    if(status) {
        FURI_LOG_I(APP_NAME, "Sensor %s allocated", name);
        return sensor;
    }
    //Выход с очисткой если память для датчика не была выделена
    free(sensor->name);
    free(sensor);
    FURI_LOG_E(APP_NAME, "Sensor %s(%s) allocation error", name, type->typename);
    return NULL;
}

void unitemp_sensor_free(Sensor* sensor) {
    if(sensor == NULL) {
        FURI_LOG_E(APP_NAME, "Null pointer sensor releasing");
        return;
    }
    if(sensor->type == NULL) {
        FURI_LOG_E(APP_NAME, "Sensor type is null");
        return;
    }
    if(sensor->type->mem_releaser == NULL) {
        FURI_LOG_E(APP_NAME, "Sensor releaser is null");
        return;
    }
    bool status = false;
    //Высвобождение памяти под инстанс
    status = sensor->type->interface->mem_releaser(sensor);
    UNUSED(status);
#ifdef UNITEMP_DEBUG

    if(status) {
        FURI_LOG_D(APP_NAME, "Sensor %s memory successfully released", sensor->name);
    } else {
        FURI_LOG_E(APP_NAME, "Sensor %s memory is not released", sensor->name);
    }
#endif
    free(sensor->name);
    //free(sensor);
}

void unitemp_sensors_free(void) {
    for(uint8_t i = 0; i < unitemp_sensors_getCount(); i++) {
        unitemp_sensor_free(app->sensors[i]);
    }
    app->sensors_count = 0;
}

bool unitemp_sensors_init(void) {
    bool result = true;

    //Перебор датчиков из списка
    for(uint8_t i = 0; i < unitemp_sensors_getCount(); i++) {
        //Включение 5V если на порту 1 FZ его нет
        //Может пропасть при отключении USB
        if(furi_hal_power_is_otg_enabled() != true) {
            furi_hal_power_enable_otg();
#ifdef UNITEMP_DEBUG
            FURI_LOG_D(APP_NAME, "OTG enabled");
#endif
        }
        if(!(*app->sensors[i]->type->initializer)(app->sensors[i])) {
            FURI_LOG_E(
                APP_NAME,
                "An error occurred during sensor initialization %s",
                app->sensors[i]->name);
            result = false;
        }
#ifdef UNITEMP_DEBUG
        FURI_LOG_D(APP_NAME, "Sensor %s successfully initialized", app->sensors[i]->name);
#endif
    }
    app->sensors_ready = true;
    return result;
}

bool unitemp_sensors_deInit(void) {
    bool result = true;
    //Выключение 5 В если до этого оно не было включено
    if(app->settings.lastOTGState != true) {
        furi_hal_power_disable_otg();
#ifdef UNITEMP_DEBUG
        FURI_LOG_D(APP_NAME, "OTG disabled");
#endif
    }

    //Перебор датчиков из списка
    for(uint8_t i = 0; i < unitemp_sensors_getCount(); i++) {
        if(!(*app->sensors[i]->type->deinitializer)(app->sensors[i])) {
            FURI_LOG_E(
                APP_NAME,
                "An error occurred during sensor deinitialization %s",
                app->sensors[i]->name);
            result = false;
        }
    }
    return result;
}

UnitempStatus unitemp_sensor_updateData(Sensor* sensor) {
    if(sensor == NULL) return UT_SENSORSTATUS_ERROR;

    //Проверка на допустимость опроса датчика
    if(furi_get_tick() - sensor->lastPollingTime < sensor->type->pollingInterval) {
        //Возврат ошибки если последний опрос датчика был неудачным
        if(sensor->status == UT_SENSORSTATUS_TIMEOUT) {
            return UT_SENSORSTATUS_TIMEOUT;
        }
        return UT_SENSORSTATUS_EARLYPOOL;
    }

    sensor->lastPollingTime = furi_get_tick();

    if(!furi_hal_power_is_otg_enabled()) {
        furi_hal_power_enable_otg();
    }

    sensor->status = sensor->type->interface->updater(sensor);

#ifdef UNITEMP_DEBUG
    if(sensor->status != UT_SENSORSTATUS_OK && sensor->status != UT_SENSORSTATUS_POLLING)
        FURI_LOG_D(APP_NAME, "Sensor %s update status %d", sensor->name, sensor->status);
#endif

    if(app->settings.temp_unit == UT_TEMP_FAHRENHEIT && sensor->status == UT_SENSORSTATUS_OK)
        uintemp_celsiumToFarengate(sensor);
    if(sensor->status == UT_SENSORSTATUS_OK) {
        sensor->temp += sensor->temp_offset / 10.f;
        if(app->settings.pressure_unit == UT_PRESSURE_MM_HG) {
            unitemp_pascalToMmHg(sensor);
        } else if(app->settings.pressure_unit == UT_PRESSURE_IN_HG) {
            unitemp_pascalToInHg(sensor);
        } else if(app->settings.pressure_unit == UT_PRESSURE_KPA) {
            unitemp_pascalToKPa(sensor);
        }
    }
    return sensor->status;
}

void unitemp_sensors_updateValues(void) {
    for(uint8_t i = 0; i < unitemp_sensors_getCount(); i++) {
        unitemp_sensor_updateData(unitemp_sensor_getActive(i));
    }
}
