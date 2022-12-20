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
#include <gui/modules/text_input.h>

//Окно ввода текста
static TextInput* text_input;
//Текущий редактируемый датчик
static Sensor* editable_sensor;

#define VIEW_ID UnitempViewSensorNameEdit

static void _sensor_name_changed_callback(void* context) {
    UNUSED(context);
    unitemp_SensorEdit_switch(editable_sensor);
}

void unitemp_SensorNameEdit_alloc(void) {
    text_input = text_input_alloc();
    view_dispatcher_add_view(app->view_dispatcher, VIEW_ID, text_input_get_view(text_input));
    text_input_set_header_text(text_input, "Sensor name");
}
void unitemp_SensorNameEdit_switch(Sensor* sensor) {
    editable_sensor = sensor;
    text_input_set_result_callback(
        text_input, _sensor_name_changed_callback, app, sensor->name, 11, true);
    view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_ID);
}
void unitemp_SensorNameEdit_free(void) {
    text_input_free(text_input);
}