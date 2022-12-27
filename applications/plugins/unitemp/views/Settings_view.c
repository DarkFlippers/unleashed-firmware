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
#include "UnitempViews.h"
#include <gui/modules/variable_item_list.h>

//Текущий вид
static View* view;
//Список
static VariableItemList* variable_item_list;

static const char states[2][9] = {"Auto", "Infinity"};
static const char temp_units[UT_TEMP_COUNT][3] = {"*C", "*F"};
static const char pressure_units[UT_PRESSURE_COUNT][6] = {"mm Hg", "in Hg", "kPa"};

//Элемент списка - бесконечная подсветка
VariableItem* infinity_backlight_item;
//Единица измерения температуры
VariableItem* temperature_unit_item;
//Единица измерения давления
VariableItem* pressure_unit_item;
#define VIEW_ID UnitempViewSettings

/**
 * @brief Функция обработки нажатия кнопки "Назад"
 *
 * @param context Указатель на данные приложения
 * @return ID вида в который нужно переключиться
 */
static uint32_t _exit_callback(void* context) {
    UNUSED(context);
    //Костыль с зависающей подсветкой
    if((bool)variable_item_get_current_value_index(infinity_backlight_item) !=
       app->settings.infinityBacklight) {
        if((bool)variable_item_get_current_value_index(infinity_backlight_item)) {
            notification_message(app->notifications, &sequence_display_backlight_enforce_on);
        } else {
            notification_message(app->notifications, &sequence_display_backlight_enforce_auto);
        }
    }

    app->settings.infinityBacklight =
        (bool)variable_item_get_current_value_index(infinity_backlight_item);
    app->settings.temp_unit = variable_item_get_current_value_index(temperature_unit_item);
    app->settings.pressure_unit = variable_item_get_current_value_index(pressure_unit_item);
    unitemp_saveSettings();
    unitemp_loadSettings();

    //Возврат предыдущий вид
    return UnitempViewMainMenu;
}
/**
 * @brief Функция обработки нажатия средней кнопки
 *
 * @param context Указатель на данные приложения
 * @param index На каком элементе списка была нажата кнопка
 */
static void _enter_callback(void* context, uint32_t index) {
    UNUSED(context);
    UNUSED(index);
}

static void _setting_change_callback(VariableItem* item) {
    if(item == infinity_backlight_item) {
        variable_item_set_current_value_text(
            infinity_backlight_item,
            states[variable_item_get_current_value_index(infinity_backlight_item)]);
    }
    if(item == temperature_unit_item) {
        variable_item_set_current_value_text(
            temperature_unit_item,
            temp_units[variable_item_get_current_value_index(temperature_unit_item)]);
    }
    if(item == pressure_unit_item) {
        variable_item_set_current_value_text(
            pressure_unit_item,
            pressure_units[variable_item_get_current_value_index(pressure_unit_item)]);
    }
}

/**
 * @brief Создание меню редактирования настроек
 */
void unitemp_Settings_alloc(void) {
    variable_item_list = variable_item_list_alloc();
    //Сброс всех элементов меню
    variable_item_list_reset(variable_item_list);

    infinity_backlight_item = variable_item_list_add(
        variable_item_list, "Backlight time", UT_TEMP_COUNT, _setting_change_callback, app);
    temperature_unit_item =
        variable_item_list_add(variable_item_list, "Temp. unit", 2, _setting_change_callback, app);
    pressure_unit_item = variable_item_list_add(
        variable_item_list, "Press. unit", UT_PRESSURE_COUNT, _setting_change_callback, app);

    //Добавление колбека на нажатие средней кнопки
    variable_item_list_set_enter_callback(variable_item_list, _enter_callback, app);

    //Создание вида из списка
    view = variable_item_list_get_view(variable_item_list);
    //Добавление колбека на нажатие кнопки "Назад"
    view_set_previous_callback(view, _exit_callback);
    //Добавление вида в диспетчер
    view_dispatcher_add_view(app->view_dispatcher, VIEW_ID, view);
}

void unitemp_Settings_switch(void) {
    //Обнуление последнего выбранного пункта
    variable_item_list_set_selected_item(variable_item_list, 0);

    variable_item_set_current_value_index(
        infinity_backlight_item, (uint8_t)app->settings.infinityBacklight);
    variable_item_set_current_value_text(
        infinity_backlight_item,
        states[variable_item_get_current_value_index(infinity_backlight_item)]);

    variable_item_set_current_value_index(temperature_unit_item, (uint8_t)app->settings.temp_unit);
    variable_item_set_current_value_text(
        temperature_unit_item,
        temp_units[variable_item_get_current_value_index(temperature_unit_item)]);

    variable_item_set_current_value_index(
        pressure_unit_item, (uint8_t)app->settings.pressure_unit);
    variable_item_set_current_value_text(
        pressure_unit_item,
        pressure_units[variable_item_get_current_value_index(pressure_unit_item)]);

    view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_ID);
}

void unitemp_Settings_free(void) {
    //Очистка списка элементов
    variable_item_list_free(variable_item_list);
    //Очистка вида
    view_free(view);
    //Удаление вида после обработки
    view_dispatcher_remove_view(app->view_dispatcher, VIEW_ID);
}