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
#include "unitemp_icons.h"

#include <assets_icons.h>

void unitemp_widgets_alloc(void) {
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, UnitempViewWidget, widget_get_view(app->widget));
}

void unitemp_widgets_free(void) {
    widget_free(app->widget);
}

/* ================== Подтверждение удаления ================== */
Sensor* current_sensor;
/**
 * @brief Функция обработки нажатия кнопки "Назад"
 * 
 * @param context Указатель на данные приложения
 * @return ID вида в который нужно переключиться
 */
static uint32_t _delete_exit_callback(void* context) {
    UNUSED(context);
    //Возвращаем ID вида, в который нужно вернуться
    return UnitempViewSensorActions;
}
/**
 * @brief Обработчик нажатий на кнопку в виджете
 * 
 * @param result Какая из кнопок была нажата
 * @param type Тип нажатия
 * @param context Указатель на данные плагина
 */
static void _delete_click_callback(GuiButtonType result, InputType type, void* context) {
    UNUSED(context);
    //Коротко нажата левая кнопка (Cancel)
    if(result == GuiButtonTypeLeft && type == InputTypeShort) {
        unitemp_SensorActions_switch(current_sensor);
    }
    //Коротко нажата правая кнопка (Delete)
    if(result == GuiButtonTypeRight && type == InputTypeShort) {
        //Удаление датчика
        unitemp_sensor_delete(current_sensor);
        //Выход из меню
        unitemp_General_switch();
    }
}
/**
 * @brief Переключение в виджет удаления датчика
 */
void unitemp_widget_delete_switch(Sensor* sensor) {
    current_sensor = sensor;
    //Очистка виджета
    widget_reset(app->widget);
    //Добавление кнопок
    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Cancel", _delete_click_callback, app);
    widget_add_button_element(
        app->widget, GuiButtonTypeRight, "Delete", _delete_click_callback, app);

    snprintf(app->buff, BUFF_SIZE, "\e#Delete %s?\e#", current_sensor->name);
    widget_add_text_box_element(
        app->widget, 0, 0, 128, 23, AlignCenter, AlignCenter, app->buff, false);

    if(current_sensor->type->interface == &ONE_WIRE) {
        OneWireSensor* s = current_sensor->instance;

        snprintf(
            app->buff,
            BUFF_SIZE,
            "\e#Type:\e# %s",
            unitemp_onewire_sensor_getModel(current_sensor));
        widget_add_text_box_element(
            app->widget, 0, 16, 128, 23, AlignLeft, AlignTop, app->buff, false);
        snprintf(app->buff, BUFF_SIZE, "\e#GPIO:\e# %s", s->bus->gpio->name);
        widget_add_text_box_element(
            app->widget, 0, 28, 128, 23, AlignLeft, AlignTop, app->buff, false);

        snprintf(
            app->buff,
            BUFF_SIZE,
            "\e#ID:\e# %02X%02X%02X%02X%02X%02X%02X%02X",
            s->deviceID[0],
            s->deviceID[1],
            s->deviceID[2],
            s->deviceID[3],
            s->deviceID[4],
            s->deviceID[5],
            s->deviceID[6],
            s->deviceID[7]);
        widget_add_text_box_element(
            app->widget, 0, 40, 128, 23, AlignLeft, AlignTop, app->buff, false);
    }

    if(current_sensor->type->interface == &SINGLE_WIRE) {
        snprintf(app->buff, BUFF_SIZE, "\e#Type:\e# %s", current_sensor->type->typename);
        widget_add_text_box_element(
            app->widget, 0, 16, 128, 23, AlignLeft, AlignTop, app->buff, false);
        snprintf(
            app->buff,
            BUFF_SIZE,
            "\e#GPIO:\e# %s",
            ((SingleWireSensor*)current_sensor->instance)->gpio->name);
        widget_add_text_box_element(
            app->widget, 0, 28, 128, 23, AlignLeft, AlignTop, app->buff, false);
    }

    if(current_sensor->type->interface == &I2C) {
        snprintf(app->buff, BUFF_SIZE, "\e#Type:\e# %s", current_sensor->type->typename);
        widget_add_text_box_element(
            app->widget, 0, 16, 128, 23, AlignLeft, AlignTop, app->buff, false);
        snprintf(
            app->buff,
            BUFF_SIZE,
            "\e#I2C addr:\e# 0x%02X",
            ((I2CSensor*)current_sensor->instance)->currentI2CAdr);
        widget_add_text_box_element(
            app->widget, 0, 28, 128, 23, AlignLeft, AlignTop, app->buff, false);
    }

    view_set_previous_callback(widget_get_view(app->widget), _delete_exit_callback);
    view_dispatcher_switch_to_view(app->view_dispatcher, UnitempViewWidget);
}

/* ========================== Помощь ========================== */

/**
 * @brief Функция обработки нажатия кнопки "Назад"
 * 
 * @param context Указатель на данные приложения
 * @return ID вида в который нужно переключиться
 */
static uint32_t _help_exit_callback(void* context) {
    UNUSED(context);
    //Возвращаем ID вида, в который нужно вернуться
    return UnitempViewGeneral;
}

/**
 * @brief Переключение в виджет помощи
 */
void unitemp_widget_help_switch(void) {
    //Очистка виджета
    widget_reset(app->widget);

    widget_add_icon_element(app->widget, 3, 7, &I_repo_qr_50x50);
    widget_add_icon_element(app->widget, 71, 15, &I_DolphinCommon_56x48);

    widget_add_string_multiline_element(
        app->widget, 55, 5, AlignLeft, AlignTop, FontSecondary, "You can find help\nthere");

    widget_add_frame_element(app->widget, 0, 0, 128, 63, 7);
    widget_add_frame_element(app->widget, 0, 0, 128, 64, 7);

    view_set_previous_callback(widget_get_view(app->widget), _help_exit_callback);
    view_dispatcher_switch_to_view(app->view_dispatcher, UnitempViewWidget);
}

/* ========================== О приложении ========================== */

/**
 * @brief Переключение в виджет о приложении
 */
void unitemp_widget_about_switch(void) {
    //Очистка виджета
    widget_reset(app->widget);

    widget_add_frame_element(app->widget, 0, 0, 128, 63, 7);
    widget_add_frame_element(app->widget, 0, 0, 128, 64, 7);

    snprintf(app->buff, BUFF_SIZE, "#Unitemp %s#", UNITEMP_APP_VER);
    widget_add_text_box_element(
        app->widget, 0, 4, 128, 12, AlignCenter, AlignCenter, app->buff, false);

    widget_add_text_scroll_element(
        app->widget,
        4,
        16,
        121,
        44,
        "Universal plugin for viewing the values of temperature\nsensors\n\e#Author: Quenon\ngithub.com/quen0n\n\e#Designer: Svaarich\ngithub.com/Svaarich\n\e#Issues & suggestions\ntiny.one/unitemp");

    view_set_previous_callback(widget_get_view(app->widget), _help_exit_callback);
    view_dispatcher_switch_to_view(app->view_dispatcher, UnitempViewWidget);
}