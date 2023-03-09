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
//Использован код Дмитрия Погребняка: https://aterlux.ru/article/1wire

#include "OneWireSensor.h"
#include <furi.h>
#include <furi_hal.h>

const SensorType Dallas = {
    .typename = "Dallas",
    .altname = "Dallas (DS18x2x)",
    .interface = &ONE_WIRE,
    .datatype = UT_DATA_TYPE_TEMP,
    .pollingInterval = 1000,
    .allocator = unitemp_onewire_sensor_alloc,
    .mem_releaser = unitemp_onewire_sensor_free,
    .initializer = unitemp_onewire_sensor_init,
    .deinitializer = unitemp_onewire_sensor_deinit,
    .updater = unitemp_onewire_sensor_update};

// Переменные для хранения промежуточного результата сканирования шины
// найденный восьмибайтовый адрес
static uint8_t onewire_enum[8] = {0};

OneWireBus* uintemp_onewire_bus_alloc(const GPIO* gpio) {
    if(gpio == NULL) {
        return NULL;
    }

    //Проверка на наличие шины на этом порте
    for(uint8_t i = 0; i < unitemp_sensors_getActiveCount(); i++) {
        if(unitemp_sensor_getActive(i)->type->interface == &ONE_WIRE &&
           ((OneWireSensor*)unitemp_sensor_getActive(i)->instance)->bus->gpio->num == gpio->num) {
            //Если шина на этом порту уже есть, то возврат указателя на шину
            return ((OneWireSensor*)unitemp_sensor_getActive(i)->instance)->bus;
        }
    }

    OneWireBus* bus = malloc(sizeof(OneWireBus));

    bus->device_count = 0;
    bus->gpio = gpio;
    bus->powerMode = PWR_PASSIVE;

    UNITEMP_DEBUG("one wire bus (port %d) allocated", gpio->num);

    return bus;
}

bool unitemp_onewire_bus_init(OneWireBus* bus) {
    if(bus == NULL) return false;
    bus->device_count++;
    //Выход если шина уже была инициализирована
    if(bus->device_count > 1) return true;

    bus->host = onewire_host_alloc(bus->gpio->pin);

    unitemp_gpio_lock(bus->gpio, &ONE_WIRE);
    //Высокий уровень по умолчанию
    furi_hal_gpio_write(bus->gpio->pin, true);
    //Режим работы - OpenDrain, подтяжка включается на всякий случай
    furi_hal_gpio_init(
        bus->gpio->pin, //Порт FZ
        GpioModeOutputOpenDrain, //Режим работы - открытый сток
        GpioPullUp, //Принудительная подтяжка линии данных к питанию
        GpioSpeedVeryHigh); //Скорость работы - максимальная

    return true;
}

bool unitemp_onewire_bus_deinit(OneWireBus* bus) {
    UNITEMP_DEBUG("devices on wire %d: %d", bus->gpio->num, bus->device_count);
    bus->device_count--;
    if(bus->device_count <= 0) {
        bus->device_count = 0;
        unitemp_gpio_unlock(bus->gpio);
        //Режим работы - аналог, подтяжка выключена
        furi_hal_gpio_init(
            bus->gpio->pin, //Порт FZ
            GpioModeAnalog, //Режим работы - аналог
            GpioPullNo, //Подтяжка выключена
            GpioSpeedLow); //Скорость работы - минимальная
        //Низкий уровень по умолчанию
        furi_hal_gpio_write(bus->gpio->pin, false);
        return true;
    } else {
        return false;
    }
}
inline bool unitemp_onewire_bus_start(OneWireBus* bus) {
    return onewire_host_reset(bus->host);
}

inline void unitemp_onewire_bus_send_bit(OneWireBus* bus, bool state) {
    onewire_host_write_bit(bus->host, state);
}

inline void unitemp_onewire_bus_send_byte(OneWireBus* bus, uint8_t data) {
    onewire_host_write(bus->host, data);
}

void unitemp_onewire_bus_send_byteArray(OneWireBus* bus, uint8_t* data, uint8_t len) {
    for(uint8_t i = 0; i < len; i++) {
        onewire_host_write(bus->host, data[i]);
    }
}

inline bool unitemp_onewire_bus_read_bit(OneWireBus* bus) {
    return onewire_host_read_bit(bus->host);
}

inline uint8_t unitemp_onewire_bus_read_byte(OneWireBus* bus) {
    return onewire_host_read(bus->host);
}

void unitemp_onewire_bus_read_byteArray(OneWireBus* bus, uint8_t* data, uint8_t len) {
    onewire_host_read_bytes(bus->host, data, len);
}

static uint8_t onewire_CRC_update(uint8_t crc, uint8_t b) {
    for(uint8_t p = 8; p; p--) {
        crc = ((crc ^ b) & 1) ? (crc >> 1) ^ 0b10001100 : (crc >> 1);
        b >>= 1;
    }
    return crc;
}

bool unitemp_onewire_CRC_check(uint8_t* data, uint8_t len) {
    uint8_t crc = 0;
    for(uint8_t i = 0; i < len; i++) {
        crc = onewire_CRC_update(crc, data[i]);
    }
    return !crc;
}

char* unitemp_onewire_sensor_getModel(Sensor* sensor) {
    OneWireSensor* ow_sensor = sensor->instance;
    switch(ow_sensor->deviceID[0]) {
    case FC_DS18B20:
        return "DS18B20";
    case FC_DS18S20:
        return "DS18S20";
    case FC_DS1822:
        return "DS1822";
    default:
        return "unknown";
    }
}

bool unitemp_onewire_sensor_readID(OneWireSensor* instance) {
    if(!unitemp_onewire_bus_start(instance->bus)) return false;
    unitemp_onewire_bus_send_byte(instance->bus, 0x33); // Чтение ПЗУ
    unitemp_onewire_bus_read_byteArray(instance->bus, instance->deviceID, 8);
    if(!unitemp_onewire_CRC_check(instance->deviceID, 8)) {
        memset(instance->deviceID, 0, 8);
        return false;
    }
    instance->familyCode = instance->deviceID[0];
    return true;
}

void unitemp_onewire_bus_enum_init(OneWireBus* bus) {
    onewire_host_reset_search(bus->host);
}

uint8_t* unitemp_onewire_bus_enum_next(OneWireBus* bus) {
    if(onewire_host_search(bus->host, onewire_enum, OneWireHostSearchModeNormal)) {
        return onewire_enum;
    } else {
        return NULL;
    }
}

void unitemp_onewire_bus_select_sensor(OneWireSensor* instance) {
    unitemp_onewire_bus_send_byte(instance->bus, 0x55);
    unitemp_onewire_bus_send_byteArray(instance->bus, instance->deviceID, 8);
}

bool unitemp_onewire_sensor_alloc(Sensor* sensor, char* args) {
    OneWireSensor* instance = malloc(sizeof(OneWireSensor));
    if(instance == NULL) {
        FURI_LOG_E(APP_NAME, "Sensor %s instance allocation error", sensor->name);
        return false;
    }
    sensor->instance = instance;
    //Очистка адреса
    memset(instance->deviceID, 0, 8);

    int gpio, addr_0, addr_1, addr_2, addr_3, addr_4, addr_5, addr_6, addr_7;
    sscanf(
        args,
        "%d %2X%2X%2X%2X%2X%2X%2X%2X",
        &gpio,
        &addr_0,
        &addr_1,
        &addr_2,
        &addr_3,
        &addr_4,
        &addr_5,
        &addr_6,
        &addr_7);
    instance->deviceID[0] = addr_0;
    instance->deviceID[1] = addr_1;
    instance->deviceID[2] = addr_2;
    instance->deviceID[3] = addr_3;
    instance->deviceID[4] = addr_4;
    instance->deviceID[5] = addr_5;
    instance->deviceID[6] = addr_6;
    instance->deviceID[7] = addr_7;

    instance->familyCode = instance->deviceID[0];

    instance->bus = uintemp_onewire_bus_alloc(unitemp_gpio_getFromInt(gpio));

    if(instance != NULL) {
        return true;
    }
    FURI_LOG_E(APP_NAME, "Sensor %s bus allocation error", sensor->name);
    free(instance);
    return false;
}

bool unitemp_onewire_sensor_free(Sensor* sensor) {
    if(((OneWireSensor*)sensor->instance)->bus != NULL) {
        if(((OneWireSensor*)sensor->instance)->bus->device_count == 0) {
            free(((OneWireSensor*)sensor->instance)->bus);
        }
    }

    free(sensor->instance);

    return true;
}

bool unitemp_onewire_sensor_init(Sensor* sensor) {
    OneWireSensor* instance = sensor->instance;
    if(instance == NULL || instance->bus == NULL) {
        FURI_LOG_E(APP_NAME, "Sensor pointer is null!");
        return false;
    }

    unitemp_onewire_bus_init(instance->bus);

    if(instance->familyCode == FC_DS18B20 || instance->familyCode == FC_DS1822) {
        //Установка разрядности в 10 бит
        if(!unitemp_onewire_bus_start(instance->bus)) return false;
        unitemp_onewire_bus_select_sensor(instance);
        unitemp_onewire_bus_send_byte(instance->bus, 0x4E); // Запись в память
        uint8_t buff[3];
        //Значения тревоги
        buff[0] = 0x4B; //Значение нижнего предела температуры
        buff[1] = 0x46; //Значение верхнего предела температуры
        //Конфигурация
        buff[2] = 0b01111111; //12 бит разрядность преобразования
        unitemp_onewire_bus_send_byteArray(instance->bus, buff, 3);

        //Сохранение значений в EEPROM для автоматического восстановления после сбоев питания
        if(!unitemp_onewire_bus_start(instance->bus)) return false;
        unitemp_onewire_bus_select_sensor(instance);
        unitemp_onewire_bus_send_byte(instance->bus, 0x48); // Запись в EEPROM
    }

    return true;
}

bool unitemp_onewire_sensor_deinit(Sensor* sensor) {
    OneWireSensor* instance = sensor->instance;
    if(instance == NULL || instance->bus == NULL) return false;
    unitemp_onewire_bus_deinit(instance->bus);

    return true;
}

UnitempStatus unitemp_onewire_sensor_update(Sensor* sensor) {
    //Снятие особого статуса с датчика при пассивном режиме питания
    if(sensor->status == UT_SENSORSTATUS_EARLYPOOL) {
        return UT_SENSORSTATUS_POLLING;
    }

    OneWireSensor* instance = sensor->instance;
    uint8_t buff[9] = {0};
    if(sensor->status != UT_SENSORSTATUS_POLLING) {
        //Если датчик в прошлый раз не отозвался, проверка его наличия на шине
        if(sensor->status == UT_SENSORSTATUS_TIMEOUT || sensor->status == UT_SENSORSTATUS_BADCRC) {
            if(!unitemp_onewire_bus_start(instance->bus)) return UT_SENSORSTATUS_TIMEOUT;
            unitemp_onewire_bus_select_sensor(instance);
            unitemp_onewire_bus_send_byte(instance->bus, 0xBE); // Read Scratch-pad
            unitemp_onewire_bus_read_byteArray(instance->bus, buff, 9);
            if(!unitemp_onewire_CRC_check(buff, 9)) {
                UNITEMP_DEBUG("Sensor %s is not found", sensor->name);
                return UT_SENSORSTATUS_TIMEOUT;
            }
        }

        if(!unitemp_onewire_bus_start(instance->bus)) return UT_SENSORSTATUS_TIMEOUT;
        //Запуск преобразования на всех датчиках в режиме пассивного питания
        if(instance->bus->powerMode == PWR_PASSIVE) {
            unitemp_onewire_bus_send_byte(instance->bus, 0xCC); // skip addr
            //Установка на всех датчиках этой шины особого статуса, чтобы не запускать преобразование ещё раз
            for(uint8_t i = 0; i < unitemp_sensors_getActiveCount(); i++) {
                if(unitemp_sensor_getActive(i)->type->interface == &ONE_WIRE &&
                   ((OneWireSensor*)unitemp_sensor_getActive(i)->instance)->bus == instance->bus) {
                    unitemp_sensor_getActive(i)->status = UT_SENSORSTATUS_EARLYPOOL;
                }
            }

        } else {
            unitemp_onewire_bus_select_sensor(instance);
        }

        unitemp_onewire_bus_send_byte(instance->bus, 0x44); // convert t
        if(instance->bus->powerMode == PWR_PASSIVE) {
            furi_hal_gpio_write(instance->bus->gpio->pin, true);
            furi_hal_gpio_init(
                instance->bus->gpio->pin, GpioModeOutputPushPull, GpioPullUp, GpioSpeedVeryHigh);
        }
        return UT_SENSORSTATUS_POLLING;
    } else {
        if(instance->bus->powerMode == PWR_PASSIVE) {
            furi_hal_gpio_write(instance->bus->gpio->pin, true);
            furi_hal_gpio_init(
                instance->bus->gpio->pin, GpioModeOutputOpenDrain, GpioPullUp, GpioSpeedVeryHigh);
        }
        if(!unitemp_onewire_bus_start(instance->bus)) return UT_SENSORSTATUS_TIMEOUT;
        unitemp_onewire_bus_select_sensor(instance);
        unitemp_onewire_bus_send_byte(instance->bus, 0xBE); // Read Scratch-pad
        unitemp_onewire_bus_read_byteArray(instance->bus, buff, 9);
        if(!unitemp_onewire_CRC_check(buff, 9)) {
            UNITEMP_DEBUG("Failed CRC check: %s", sensor->name);
            return UT_SENSORSTATUS_BADCRC;
        }
        int16_t raw = buff[0] | ((int16_t)buff[1] << 8);
        if(instance->familyCode == FC_DS18S20) {
            //Песевдо-12-бит. Отключено из-за неестественности и нестабильности показаний по сравнению с DS18B20
            //sensor->temp = ((float)raw / 2.0f) - 0.25f + (16.0f - buff[6]) / 16.0f;
            //Честные 9 бит
            sensor->temp = ((float)raw / 2.0f);
        } else {
            sensor->temp = (float)raw / 16.0f;
        }
    }

    return UT_SENSORSTATUS_OK;
}

bool unitemp_onewire_id_compare(uint8_t* id1, uint8_t* id2) {
    if(id1 == NULL || id2 == NULL) return false;
    for(uint8_t i = 0; i < 8; i++) {
        if(id1[i] != id2[i]) return false;
    }
    return true;
}