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
#include "UnitempViews.h"
#include <gui/modules/variable_item_list.h>

#include "../interfaces/SingleWireSensor.h"
#include "../interfaces/OneWireSensor.h"
#include "../interfaces/I2CSensor.h"

//Текущий вид
static View* view;
//Список
static VariableItemList* variable_item_list;
//Текущий редактируемый датчик
static Sensor* editable_sensor;
//Изначальный GPIO датчика
static const GPIO* initial_gpio = NULL;

//Элемент списка - имя датчика
static VariableItem* sensor_name_item;
//Элемент списка - адрес датчика one wire
static VariableItem* onewire_addr_item;
//Элемент списка - адрес датчика one wire
static VariableItem* onewire_type_item;
//Элемент списка - смещение температуры
VariableItem* temp_offset_item;

#define OFFSET_BUFF_SIZE 5
//Буффер для текста смещения
static char* offset_buff;

extern uint8_t generalview_sensor_index;

#define VIEW_ID UnitempViewSensorEdit

bool _onewire_id_exist(uint8_t* id) {
    if(id == NULL) return false;
    for(uint8_t i = 0; i < unitemp_sensors_getActiveCount(); i++) {
        if(unitemp_sensor_getActive(i)->type == &Dallas) {
            if(unitemp_onewire_id_compare(
                   id, ((OneWireSensor*)(unitemp_sensor_getActive(i)->instance))->deviceID)) {
                return true;
            }
        }
    }
    return false;
}

static void _onewire_scan(void) {
    OneWireSensor* ow_sensor = editable_sensor->instance;

    UNITEMP_DEBUG(
        "devices on wire %d: %d", ow_sensor->bus->gpio->num, ow_sensor->bus->device_count);

    //Сканирование шины one wire
    unitemp_onewire_bus_init(ow_sensor->bus);
    uint8_t* id = NULL;
    do {
        id = unitemp_onewire_bus_enum_next(ow_sensor->bus);
    } while(_onewire_id_exist(id));

    if(id == NULL) {
        unitemp_onewire_bus_enum_init(ow_sensor->bus);
        id = unitemp_onewire_bus_enum_next(ow_sensor->bus);
        if(_onewire_id_exist(id)) {
            do {
                id = unitemp_onewire_bus_enum_next(ow_sensor->bus);
            } while(_onewire_id_exist(id) && id != NULL);
        }
        if(id == NULL) {
            memset(ow_sensor->deviceID, 0, 8);
            ow_sensor->familyCode = 0;
            unitemp_onewire_bus_deinit(ow_sensor->bus);
            variable_item_set_current_value_text(onewire_addr_item, "empty");
            variable_item_set_current_value_text(
                onewire_type_item, unitemp_onewire_sensor_getModel(editable_sensor));
            return;
        }
    }

    unitemp_onewire_bus_deinit(ow_sensor->bus);

    memcpy(ow_sensor->deviceID, id, 8);
    ow_sensor->familyCode = id[0];

    UNITEMP_DEBUG(
        "Found sensor's ID: %02X%02X%02X%02X%02X%02X%02X%02X",
        id[0],
        id[1],
        id[2],
        id[3],
        id[4],
        id[5],
        id[6],
        id[7]);

    if(ow_sensor->familyCode != 0) {
        char id_buff[10];
        snprintf(
            id_buff,
            10,
            "%02X%02X%02X",
            ow_sensor->deviceID[1],
            ow_sensor->deviceID[2],
            ow_sensor->deviceID[3]);
        //А больше не лезет(
        variable_item_set_current_value_text(onewire_addr_item, id_buff);
    } else {
        variable_item_set_current_value_text(onewire_addr_item, "empty");
    }
    variable_item_set_current_value_text(
        onewire_type_item, unitemp_onewire_sensor_getModel(editable_sensor));
}

/**
 * @brief Функция обработки нажатия кнопки "Назад"
 *
 * @param context Указатель на данные приложения
 * @return ID вида в который нужно переключиться
 */
static uint32_t _exit_callback(void* context) {
    UNUSED(context);
    editable_sensor->status = UT_SENSORSTATUS_TIMEOUT;
    if(!unitemp_sensor_isContains(editable_sensor)) unitemp_sensor_free(editable_sensor);
    unitemp_sensors_reload();
    //Возврат предыдущий вид
    return UnitempViewGeneral;
}
/**
 * @brief Функция обработки нажатия средней кнопки
 *
 * @param context Указатель на данные приложения
 * @param index На каком элементе списка была нажата кнопка
 */
static void _enter_callback(void* context, uint32_t index) {
    UNUSED(context);
    //Смена имени
    if(index == 0) {
        unitemp_SensorNameEdit_switch(editable_sensor);
    }
    //Сохранение
    if((index == 4 && editable_sensor->type->interface != &ONE_WIRE) ||
       (index == 5 && editable_sensor->type->interface == &ONE_WIRE)) {
        //Выход если датчик one wire не имеет ID
        if(editable_sensor->type->interface == &ONE_WIRE &&
           ((OneWireSensor*)(editable_sensor->instance))->familyCode == 0) {
            return;
        }
        if(initial_gpio != NULL) {
            unitemp_gpio_unlock(initial_gpio);
            initial_gpio = NULL;
        }
        editable_sensor->status = UT_SENSORSTATUS_TIMEOUT;
        if(!unitemp_sensor_isContains(editable_sensor)) unitemp_sensors_add(editable_sensor);
        unitemp_sensors_save();
        unitemp_sensors_reload();

        generalview_sensor_index = unitemp_sensors_getActiveCount() - 1;
        unitemp_General_switch();
    }

    //Адрес устройства на шине one wire
    if(index == 4 && editable_sensor->type->interface == &ONE_WIRE) {
        _onewire_scan();
    }
}

/**
 * @brief Функция обработки изменения значения GPIO
 * 
 * @param item Указатель на элемент списка
 */
static void _gpio_change_callback(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    if(editable_sensor->type->interface == &SINGLE_WIRE) {
        SingleWireSensor* instance = editable_sensor->instance;
        instance->gpio =
            unitemp_gpio_getAviablePort(editable_sensor->type->interface, index, initial_gpio);
        variable_item_set_current_value_text(item, instance->gpio->name);
    }
    if(editable_sensor->type->interface == &SPI) {
        SPISensor* instance = editable_sensor->instance;
        instance->CS_pin =
            unitemp_gpio_getAviablePort(editable_sensor->type->interface, index, initial_gpio);
        variable_item_set_current_value_text(item, instance->CS_pin->name);
    }
    if(editable_sensor->type->interface == &ONE_WIRE) {
        OneWireSensor* instance = editable_sensor->instance;
        instance->bus->gpio =
            unitemp_gpio_getAviablePort(editable_sensor->type->interface, index, NULL);
        variable_item_set_current_value_text(item, instance->bus->gpio->name);
    }
}
/**
 * @brief Функция обработки изменения значения GPIO
 * 
 * @param item Указатель на элемент списка
 */
static void _i2caddr_change_callback(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    ((I2CSensor*)editable_sensor->instance)->currentI2CAdr =
        ((I2CSensor*)editable_sensor->instance)->minI2CAdr + index * 2;
    char buff[5];
    snprintf(buff, 5, "0x%2X", ((I2CSensor*)editable_sensor->instance)->currentI2CAdr >> 1);
    variable_item_set_current_value_text(item, buff);
}
/**
 * @brief Функция обработки изменения значения имени датчика
 * 
 * @param item Указатель на элемент списка
 */
static void _name_change_callback(VariableItem* item) {
    variable_item_set_current_value_index(item, 0);
    unitemp_SensorNameEdit_switch(editable_sensor);
}
/**
 * @brief Функция обработки изменения значения адреса датчика one wire
 * 
 * @param item Указатель на элемент списка
 */
static void _onwire_addr_change_callback(VariableItem* item) {
    variable_item_set_current_value_index(item, 0);
    _onewire_scan();
}

/**
 * @brief Функция обработки изменения значения смещения температуры
 * 
 * @param item Указатель на элемент списка
 */
static void _offset_change_callback(VariableItem* item) {
    editable_sensor->temp_offset = variable_item_get_current_value_index(item) - 20;
    snprintf(
        offset_buff, OFFSET_BUFF_SIZE, "%+1.1f", (double)(editable_sensor->temp_offset / 10.0));
    variable_item_set_current_value_text(item, offset_buff);
}

/**
 * @brief Создание меню редактирования датчка
 */
void unitemp_SensorEdit_alloc(void) {
    variable_item_list = variable_item_list_alloc();
    //Сброс всех элементов меню
    variable_item_list_reset(variable_item_list);

    //Добавление колбека на нажатие средней кнопки
    variable_item_list_set_enter_callback(variable_item_list, _enter_callback, app);

    //Создание вида из списка
    view = variable_item_list_get_view(variable_item_list);
    //Добавление колбека на нажатие кнопки "Назад"
    view_set_previous_callback(view, _exit_callback);
    //Добавление вида в диспетчер
    view_dispatcher_add_view(app->view_dispatcher, VIEW_ID, view);

    offset_buff = malloc(OFFSET_BUFF_SIZE);
}

void unitemp_SensorEdit_switch(Sensor* sensor) {
    editable_sensor = sensor;

    editable_sensor->status = UT_SENSORSTATUS_INACTIVE;

    //Сброс всех элементов меню
    variable_item_list_reset(variable_item_list);
    //Обнуление последнего выбранного пункта
    variable_item_list_set_selected_item(variable_item_list, 0);

    //Имя датчика
    sensor_name_item = variable_item_list_add(
        variable_item_list, "Name", strlen(sensor->name) > 7 ? 1 : 2, _name_change_callback, NULL);
    variable_item_set_current_value_index(sensor_name_item, 0);
    variable_item_set_current_value_text(sensor_name_item, sensor->name);

    //Тип датчика (не редактируется)
    onewire_type_item = variable_item_list_add(variable_item_list, "Type", 1, NULL, NULL);
    variable_item_set_current_value_index(onewire_type_item, 0);
    variable_item_set_current_value_text(
        onewire_type_item,
        (sensor->type->interface == &ONE_WIRE ? unitemp_onewire_sensor_getModel(editable_sensor) :
                                                sensor->type->typename));
    //Смещение температуры
    temp_offset_item = variable_item_list_add(
        variable_item_list, "Temp. offset", 41, _offset_change_callback, NULL);
    variable_item_set_current_value_index(temp_offset_item, sensor->temp_offset + 20);
    snprintf(
        offset_buff, OFFSET_BUFF_SIZE, "%+1.1f", (double)(editable_sensor->temp_offset / 10.0));
    variable_item_set_current_value_text(temp_offset_item, offset_buff);

    //Порт подключения датчка (для one wire, SPI и single wire)
    if(sensor->type->interface == &ONE_WIRE || sensor->type->interface == &SINGLE_WIRE ||
       sensor->type->interface == &SPI) {
        if(sensor->type->interface == &ONE_WIRE) {
            initial_gpio = ((OneWireSensor*)editable_sensor->instance)->bus->gpio;
        } else if(sensor->type->interface == &SINGLE_WIRE) {
            initial_gpio = ((SingleWireSensor*)editable_sensor->instance)->gpio;
        } else if(sensor->type->interface == &SPI) {
            initial_gpio = ((SPISensor*)editable_sensor->instance)->CS_pin;
        }

        uint8_t aviable_gpio_count =
            unitemp_gpio_getAviablePortsCount(sensor->type->interface, initial_gpio);
        VariableItem* item = variable_item_list_add(
            variable_item_list, "GPIO", aviable_gpio_count, _gpio_change_callback, app);

        uint8_t gpio_index = 0;
        if(unitemp_sensor_isContains(editable_sensor)) {
            for(uint8_t i = 0; i < aviable_gpio_count; i++) {
                if(unitemp_gpio_getAviablePort(sensor->type->interface, i, initial_gpio) ==
                   initial_gpio) {
                    gpio_index = i;
                    break;
                }
            }
        }
        variable_item_set_current_value_index(item, gpio_index);
        variable_item_set_current_value_text(
            item,
            unitemp_gpio_getAviablePort(sensor->type->interface, gpio_index, initial_gpio)->name);
    }
    //Адрес устройства на шине I2C (для датчиков I2C)
    if(sensor->type->interface == &I2C) {
        VariableItem* item = variable_item_list_add(
            variable_item_list,
            "I2C address",
            (((I2CSensor*)sensor->instance)->maxI2CAdr >> 1) -
                (((I2CSensor*)sensor->instance)->minI2CAdr >> 1) + 1,
            _i2caddr_change_callback,
            app);
        snprintf(app->buff, 5, "0x%2X", ((I2CSensor*)sensor->instance)->currentI2CAdr >> 1);
        variable_item_set_current_value_index(
            item,
            (((I2CSensor*)sensor->instance)->currentI2CAdr >> 1) -
                (((I2CSensor*)sensor->instance)->minI2CAdr >> 1));
        variable_item_set_current_value_text(item, app->buff);
    }

    //Адрес устройства на шине one wire (для датчиков one wire)
    if(sensor->type->interface == &ONE_WIRE) {
        onewire_addr_item = variable_item_list_add(
            variable_item_list, "Address", 2, _onwire_addr_change_callback, NULL);
        OneWireSensor* ow_sensor = sensor->instance;
        if(ow_sensor->familyCode == 0) {
            variable_item_set_current_value_text(onewire_addr_item, "Scan");
        } else {
            snprintf(
                app->buff,
                10,
                "%02X%02X%02X",
                ow_sensor->deviceID[1],
                ow_sensor->deviceID[2],
                ow_sensor->deviceID[3]);
            variable_item_set_current_value_text(onewire_addr_item, app->buff);
        }
    }
    variable_item_list_add(variable_item_list, "Save", 1, NULL, NULL);
    view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_ID);
}

void unitemp_SensorEdit_free(void) {
    //Очистка списка элементов
    variable_item_list_free(variable_item_list);
    //Очистка вида
    view_free(view);
    //Удаление вида после обработки
    view_dispatcher_remove_view(app->view_dispatcher, VIEW_ID);
    free(offset_buff);
}