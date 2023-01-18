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
#include <stdio.h>

extern const Icon I_Cry_dolph_55x52;

//Текущий вид
static View* view;
//Список
static VariableItemList* variable_item_list;

#define VIEW_ID UnitempViewSensorsList

/**
 * @brief Функция обработки нажатия кнопки "Назад"
 *
 * @param context Указатель на данные приложения
 * @return ID вида в который нужно переключиться
 */
static uint32_t _exit_callback(void* context) {
    UNUSED(context);

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
    if(index == unitemp_sensors_getTypesCount()) {
        unitemp_widget_help_switch();
        return;
    }

    const SensorType* type = unitemp_sensors_getTypes()[index];
    uint8_t sensor_type_count = 0;

    //Подсчёт имеющихся датчиков данного типа
    for(uint8_t i = 0; i < unitemp_sensors_getActiveCount(); i++) {
        if(unitemp_sensor_getActive(i)->type == type) {
            sensor_type_count++;
        }
    }

    //Имя датчка
    char sensor_name[11];
    //Добавление счётчика к имени если такой датчик имеется
    if(sensor_type_count == 0)
        snprintf(sensor_name, 11, "%s", type->typename);
    else
        snprintf(sensor_name, 11, "%s_%d", type->typename, sensor_type_count);

    char args[22] = {0};

    //Проверка доступности датчика
    if(unitemp_gpio_getAviablePort(type->interface, 0, NULL) == NULL) {
        if(type->interface == &SINGLE_WIRE || type->interface == &ONE_WIRE) {
            unitemp_popup(
                &I_Cry_dolph_55x52, "Sensor is unavailable", "All GPIOs\nare busy", VIEW_ID);
        }
        if(type->interface == &I2C) {
            unitemp_popup(
                &I_Cry_dolph_55x52, "Sensor is unavailable", "GPIOs 15 or 16\nare busy", VIEW_ID);
        }
        return;
    }

    //Выбор первого доступного порта для датчика single wire и SPI
    if(type->interface == &SINGLE_WIRE || type->interface == &SPI) {
        snprintf(
            args,
            4,
            "%d",
            unitemp_gpio_toInt(unitemp_gpio_getAviablePort(type->interface, 0, NULL)));
    }
    //Выбор первого доступного порта для датчика one wire и запись нулевого ID
    if(type->interface == &ONE_WIRE) {
        snprintf(
            args,
            21,
            "%d %02X%02X%02X%02X%02X%02X%02X%02X",
            unitemp_gpio_toInt(unitemp_gpio_getAviablePort(type->interface, 0, NULL)),
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0);
    }
    //Для I2C адрес выберется автоматически

    unitemp_SensorEdit_switch(unitemp_sensor_alloc(sensor_name, type, args));
}

/**
 * @brief Создание меню редактирования настроек
 */
void unitemp_SensorsList_alloc(void) {
    variable_item_list = variable_item_list_alloc();
    //Сброс всех элементов меню
    variable_item_list_reset(variable_item_list);

    //Добавление в список доступных датчиков
    for(uint8_t i = 0; i < unitemp_sensors_getTypesCount(); i++) {
        if(unitemp_sensors_getTypes()[i]->altname == NULL) {
            variable_item_list_add(
                variable_item_list, unitemp_sensors_getTypes()[i]->typename, 1, NULL, app);
        } else {
            variable_item_list_add(
                variable_item_list, unitemp_sensors_getTypes()[i]->altname, 1, NULL, app);
        }
    }
    variable_item_list_add(variable_item_list, "I don't know what to choose", 1, NULL, app);

    //Добавление колбека на нажатие средней кнопки
    variable_item_list_set_enter_callback(variable_item_list, _enter_callback, app);

    //Создание вида из списка
    view = variable_item_list_get_view(variable_item_list);
    //Добавление колбека на нажатие кнопки "Назад"
    view_set_previous_callback(view, _exit_callback);
    //Добавление вида в диспетчер
    view_dispatcher_add_view(app->view_dispatcher, VIEW_ID, view);
}

void unitemp_SensorsList_switch(void) {
    //Обнуление последнего выбранного пункта
    variable_item_list_set_selected_item(variable_item_list, 0);

    view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_ID);
}

void unitemp_SensorsList_free(void) {
    //Очистка списка элементов
    variable_item_list_free(variable_item_list);
    //Очистка вида
    view_free(view);
    //Удаление вида после обработки
    view_dispatcher_remove_view(app->view_dispatcher, VIEW_ID);
}