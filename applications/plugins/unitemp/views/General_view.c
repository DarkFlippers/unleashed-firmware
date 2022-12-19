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

static View* view;

typedef enum general_views {
    G_NO_SENSORS_VIEW, //Нет датчиков
    G_LIST_VIEW, //Вид в ввиде списка
    G_CAROUSEL_VIEW, //Карусель
} general_view;

typedef enum carousel_info {
    CAROUSEL_VALUES, //Отображение значений датчиков
    CAROUSEL_INFO, //Отображение информации о датчике
} carousel_info;

static general_view current_view;

carousel_info carousel_info_selector = CAROUSEL_VALUES;
uint8_t generalview_sensor_index = 0;

static void _draw_temperature(Canvas* canvas, Sensor* sensor, uint8_t x, uint8_t y, Color color) {
    //Рисование рамки
    canvas_draw_rframe(canvas, x, y, 54, 20, 3);

    if(color == ColorBlack) {
        canvas_draw_rbox(canvas, x, y, 54, 19, 3);
        canvas_invert_color(canvas);
    } else {
        canvas_draw_rframe(canvas, x, y, 54, 19, 3);
    }

    int8_t temp_dec = abs((int16_t)(sensor->temp * 10) % 10);

    //Рисование иконки
    canvas_draw_icon(
        canvas,
        x + 3,
        y + 3,
        (app->settings.temp_unit == UT_TEMP_CELSIUS ? &I_temp_C_11x14 : &I_temp_F_11x14));

    if((int16_t)sensor->temp == -128 || sensor->status == UT_SENSORSTATUS_TIMEOUT) {
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, x + 27, y + 10, AlignCenter, AlignCenter, "--");
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, x + 50, y + 10 + 3, AlignRight, AlignCenter, ". -");
        if(color == ColorBlack) canvas_invert_color(canvas);
        return;
    }

    //Целая часть температуры
    //Костыль для отображения знака числа меньше 0
    uint8_t offset = 0;
    if(sensor->temp < 0 && sensor->temp > -1) {
        app->buff[0] = '-';
        offset = 1;
    }
    snprintf((char*)(app->buff + offset), BUFF_SIZE, "%d", (int8_t)sensor->temp);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(
        canvas,
        x + 27 + ((sensor->temp <= -10 || sensor->temp > 99) ? 5 : 0),
        y + 10,
        AlignCenter,
        AlignCenter,
        app->buff);
    //Печать дробной части температуры в диапазоне от -9 до 99 (когда два знака в числе)
    if(sensor->temp > -10 && sensor->temp <= 99) {
        uint8_t int_len = canvas_string_width(canvas, app->buff);
        snprintf(app->buff, BUFF_SIZE, ".%d", temp_dec);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, x + 27 + int_len / 2 + 2, y + 10 + 7, app->buff);
    }
    if(color == ColorBlack) canvas_invert_color(canvas);
}

static void _draw_humidity(Canvas* canvas, Sensor* sensor, const uint8_t pos[2]) {
    //Рисование рамки
    canvas_draw_rframe(canvas, pos[0], pos[1], 54, 20, 3);
    canvas_draw_rframe(canvas, pos[0], pos[1], 54, 19, 3);

    //Рисование иконки
    canvas_draw_icon(canvas, pos[0] + 3, pos[1] + 2, &I_hum_9x15);

    //Целая часть влажности
    snprintf(app->buff, BUFF_SIZE, "%d", (uint8_t)sensor->hum);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(canvas, pos[0] + 27, pos[1] + 10, AlignCenter, AlignCenter, app->buff);
    uint8_t int_len = canvas_string_width(canvas, app->buff);
    //Единица измерения
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, pos[0] + 27 + int_len / 2 + 4, pos[1] + 10 + 7, "%");
}

static void _draw_pressure(Canvas* canvas, Sensor* sensor) {
    const uint8_t x = 29, y = 39;
    //Рисование рамки
    canvas_draw_rframe(canvas, x, y, 69, 20, 3);
    canvas_draw_rframe(canvas, x, y, 69, 19, 3);

    //Рисование иконки
    canvas_draw_icon(canvas, x + 3, y + 4, &I_pressure_7x13);

    int16_t press_int = sensor->pressure;
    int8_t press_dec = (int16_t)(sensor->temp * 10) % 10;

    //Целая часть давления
    snprintf(app->buff, BUFF_SIZE, "%d", press_int);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str_aligned(
        canvas, x + 27 + ((press_int > 99) ? 5 : 0), y + 10, AlignCenter, AlignCenter, app->buff);
    //Печать дробной части давления в диапазоне от 0 до 99 (когда два знака в числе)
    if(press_int <= 99) {
        uint8_t int_len = canvas_string_width(canvas, app->buff);
        snprintf(app->buff, BUFF_SIZE, ".%d", press_dec);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, x + 27 + int_len / 2 + 2, y + 10 + 7, app->buff);
    }
    canvas_set_font(canvas, FontSecondary);
    //Единица измерения
    if(app->settings.pressure_unit == UT_PRESSURE_MM_HG) {
        canvas_draw_icon(canvas, x + 50, y + 2, &I_mm_hg_15x15);
    } else if(app->settings.pressure_unit == UT_PRESSURE_IN_HG) {
        canvas_draw_icon(canvas, x + 50, y + 2, &I_in_hg_15x15);
    } else if(app->settings.pressure_unit == UT_PRESSURE_KPA) {
        canvas_draw_str(canvas, x + 52, y + 13, "kPa");
    }
}

static void _draw_singleSensor(Canvas* canvas, Sensor* sensor, const uint8_t pos[2], Color color) {
    canvas_set_font(canvas, FontPrimary);

    const uint8_t max_width = 56;

    char sensor_name[12] = {0};
    memcpy(sensor_name, sensor->name, 10);

    if(canvas_string_width(canvas, sensor_name) > max_width) {
        uint8_t i = 10;
        while((canvas_string_width(canvas, sensor_name) > max_width - 6) && (i != 0)) {
            sensor_name[i--] = '\0';
        }
        sensor_name[++i] = '.';
        sensor_name[++i] = '.';
    }

    canvas_draw_str_aligned(
        canvas, pos[0] + 27, pos[1] + 3, AlignCenter, AlignCenter, sensor_name);
    _draw_temperature(canvas, sensor, pos[0], pos[1] + 8, color);
}

static void _draw_view_noSensors(Canvas* canvas) {
    canvas_draw_icon(canvas, 7, 17, &I_sherlok_53x45);
    //Рисование рамки
    canvas_draw_rframe(canvas, 0, 0, 128, 63, 7);
    canvas_draw_rframe(canvas, 0, 0, 128, 64, 7);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 63, 10, AlignCenter, AlignCenter, "No sensors found");
    canvas_set_font(canvas, FontSecondary);
    const uint8_t x = 65, y = 32;
    canvas_draw_rframe(canvas, x - 4, y - 11, 54, 33, 3);
    canvas_draw_rframe(canvas, x - 4, y - 11, 54, 34, 3);
    canvas_draw_str(canvas, x, y, "To add the");
    canvas_draw_str(canvas, x, y + 9, "new sensor");
    canvas_draw_str(canvas, x, y + 18, "press OK");

    canvas_draw_icon(canvas, x + 37, y + 10, &I_Ok_btn_9x9);
}

static void _draw_view_sensorsList(Canvas* canvas) {
    //Текущая страница
    uint8_t page = generalview_sensor_index / 4;
    //Количество датчиков, которые будут отображаться на странице
    uint8_t page_sensors_count;
    if((unitemp_sensors_getActiveCount() - page * 4) / 4) {
        page_sensors_count = 4;
    } else {
        page_sensors_count = (unitemp_sensors_getActiveCount() - page * 4) % 4;
    }

    //Количество страниц
    uint8_t pages =
        unitemp_sensors_getActiveCount() / 4 + (unitemp_sensors_getActiveCount() % 4 ? 1 : 0);

    //Стрелка влево
    if(page > 0) {
        canvas_draw_icon(canvas, 2, 32, &I_ButtonLeft_4x7);
    }
    //Стрелка вправо
    if(pages > 0 && page < pages - 1) {
        canvas_draw_icon(canvas, 122, 32, &I_ButtonRight_4x7);
    }

    const uint8_t value_positions[][4][2] = {
        {{36, 18}}, //1 датчик
        {{7, 18}, {67, 18}}, //2 датчика
        {{7, 3}, {67, 3}, {37, 33}}, //3 датчика
        {{7, 3}, {67, 3}, {7, 33}, {67, 33}}}; //4 датчика
    //Рисование рамки
    canvas_draw_rframe(canvas, 0, 0, 128, 63, 7);
    canvas_draw_rframe(canvas, 0, 0, 128, 64, 7);
    for(uint8_t i = 0; i < page_sensors_count; i++) {
        _draw_singleSensor(
            canvas,
            unitemp_sensor_getActive(page * 4 + i),
            value_positions[page_sensors_count - 1][i],
            ColorWhite);
    }
}

static void _draw_carousel_values(Canvas* canvas) {
    UnitempStatus sensor_status = unitemp_sensor_getActive(generalview_sensor_index)->status;
    if(sensor_status == UT_SENSORSTATUS_ERROR || sensor_status == UT_SENSORSTATUS_TIMEOUT) {
        const Icon* frames[] = {
            &I_flipper_happy_60x38, &I_flipper_happy_2_60x38, &I_flipper_sad_60x38};
        canvas_draw_icon(canvas, 34, 23, frames[furi_get_tick() % 2250 / 750]);

        canvas_set_font(canvas, FontSecondary);
        if(unitemp_sensor_getActive(generalview_sensor_index)->type->interface == &SINGLE_WIRE) {
            snprintf(
                app->buff,
                BUFF_SIZE,
                "Waiting for module on pin %d",
                ((SingleWireSensor*)unitemp_sensor_getActive(generalview_sensor_index)->instance)
                    ->gpio->num);
        }
        if(unitemp_sensor_getActive(generalview_sensor_index)->type->interface == &ONE_WIRE) {
            snprintf(
                app->buff,
                BUFF_SIZE,
                "Waiting for module on pin %d",
                ((OneWireSensor*)unitemp_sensor_getActive(generalview_sensor_index)->instance)
                    ->bus->gpio->num);
        }
        if(unitemp_sensor_getActive(generalview_sensor_index)->type->interface == &I2C) {
            snprintf(app->buff, BUFF_SIZE, "Waiting for module on I2C pins");
        }
        canvas_draw_str_aligned(canvas, 64, 19, AlignCenter, AlignCenter, app->buff);
        return;
    }

    static const uint8_t temp_positions[3][2] = {{37, 23}, {37, 16}, {9, 16}};
    static const uint8_t hum_positions[2][2] = {{37, 38}, {65, 16}};
    //Селектор значений для отображения
    switch(unitemp_sensor_getActive(generalview_sensor_index)->type->datatype) {
    case UT_DATA_TYPE_TEMP:
        _draw_temperature(
            canvas,
            unitemp_sensor_getActive(generalview_sensor_index),
            temp_positions[0][0],
            temp_positions[0][1],
            ColorWhite);
        break;
    case UT_DATA_TYPE_TEMP_HUM:
        _draw_temperature(
            canvas,
            unitemp_sensor_getActive(generalview_sensor_index),
            temp_positions[1][0],
            temp_positions[1][1],
            ColorWhite);
        _draw_humidity(
            canvas, unitemp_sensor_getActive(generalview_sensor_index), hum_positions[0]);
        break;
    case UT_DATA_TYPE_TEMP_PRESS:
        _draw_temperature(
            canvas,
            unitemp_sensor_getActive(generalview_sensor_index),
            temp_positions[1][0],
            temp_positions[1][1],
            ColorWhite);
        _draw_pressure(canvas, unitemp_sensor_getActive(generalview_sensor_index));
        break;
    case UT_DATA_TYPE_TEMP_HUM_PRESS:
        _draw_temperature(
            canvas,
            unitemp_sensor_getActive(generalview_sensor_index),
            temp_positions[2][0],
            temp_positions[2][1],
            ColorWhite);
        _draw_humidity(
            canvas, unitemp_sensor_getActive(generalview_sensor_index), hum_positions[1]);
        _draw_pressure(canvas, unitemp_sensor_getActive(generalview_sensor_index));
        break;
    }
}
static void _draw_carousel_info(Canvas* canvas) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 10, 23, "Type:");

    if(unitemp_sensor_getActive(generalview_sensor_index)->type->interface == &ONE_WIRE) {
        OneWireSensor* s = unitemp_sensor_getActive(generalview_sensor_index)->instance;
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 10, 35, "GPIO:");
        canvas_draw_str(canvas, 10, 47, "ID:");

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(
            canvas,
            41,
            23,
            unitemp_onewire_sensor_getModel(unitemp_sensor_getActive(generalview_sensor_index)));
        canvas_draw_str(canvas, 41, 35, s->bus->gpio->name);
        snprintf(
            app->buff,
            BUFF_SIZE,
            "%02X%02X%02X%02X%02X%02X%02X%02X",
            s->deviceID[0],
            s->deviceID[1],
            s->deviceID[2],
            s->deviceID[3],
            s->deviceID[4],
            s->deviceID[5],
            s->deviceID[6],
            s->deviceID[7]);
        canvas_draw_str(canvas, 24, 47, app->buff);
    }

    if(unitemp_sensor_getActive(generalview_sensor_index)->type->interface == &SINGLE_WIRE) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 10, 35, "GPIO:");

        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(
            canvas, 41, 23, unitemp_sensor_getActive(generalview_sensor_index)->type->typename);
        canvas_draw_str(
            canvas,
            41,
            35,
            ((SingleWireSensor*)unitemp_sensor_getActive(generalview_sensor_index)->instance)
                ->gpio->name);
    }

    if(unitemp_sensor_getActive(generalview_sensor_index)->type->interface == &I2C) {
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 10, 35, "I2C addr:");
        canvas_draw_str(canvas, 10, 46, "SDA pin:");
        canvas_draw_str(canvas, 10, 58, "SCL pin:");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(
            canvas, 41, 23, unitemp_sensor_getActive(generalview_sensor_index)->type->typename);
        snprintf(
            app->buff,
            BUFF_SIZE,
            "0x%02X",
            ((I2CSensor*)unitemp_sensor_getActive(generalview_sensor_index)->instance)
                ->currentI2CAdr);
        canvas_draw_str(canvas, 57, 35, app->buff);
        canvas_draw_str(canvas, 54, 46, "15 (C0)");
        canvas_draw_str(canvas, 54, 58, "16 (C1)");
    }
}
static void _draw_view_sensorsCarousel(Canvas* canvas) {
    //Рисование рамки
    canvas_draw_rframe(canvas, 0, 0, 128, 63, 7);
    canvas_draw_rframe(canvas, 0, 0, 128, 64, 7);

    //Печать имени
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(
        canvas,
        64,
        7,
        AlignCenter,
        AlignCenter,
        unitemp_sensor_getActive(generalview_sensor_index)->name);
    //Подчёркивание
    uint8_t line_len =
        canvas_string_width(canvas, unitemp_sensor_getActive(generalview_sensor_index)->name) + 2;
    canvas_draw_line(canvas, 64 - line_len / 2, 12, 64 + line_len / 2, 12);

    //Стрелка вправо
    if(unitemp_sensors_getTypesCount() > 0 &&
       generalview_sensor_index < unitemp_sensors_getActiveCount() - 1) {
        canvas_draw_icon(canvas, 122, 29, &I_ButtonRight_4x7);
    }
    //Стрелка влево
    if(generalview_sensor_index > 0) {
        canvas_draw_icon(canvas, 2, 29, &I_ButtonLeft_4x7);
    }

    switch(carousel_info_selector) {
    case CAROUSEL_VALUES:
        _draw_carousel_values(canvas);
        break;
    case CAROUSEL_INFO:
        _draw_carousel_info(canvas);
        break;
    }
}

static void _draw_callback(Canvas* canvas, void* _model) {
    UNUSED(_model);

    app->sensors_ready = true;

    uint8_t sensors_count = unitemp_sensors_getActiveCount();

    if(generalview_sensor_index + 1 > sensors_count) generalview_sensor_index = 0;

    if(sensors_count == 0) {
        current_view = G_NO_SENSORS_VIEW;
        _draw_view_noSensors(canvas);
    } else {
        if(sensors_count == 1) current_view = G_CAROUSEL_VIEW;
        if(current_view == G_NO_SENSORS_VIEW) current_view = G_CAROUSEL_VIEW;
        if(current_view == G_LIST_VIEW) _draw_view_sensorsList(canvas);
        if(current_view == G_CAROUSEL_VIEW) _draw_view_sensorsCarousel(canvas);
    }
}

static bool _input_callback(InputEvent* event, void* context) {
    UNUSED(context);

    //Обработка короткого нажатия "ок"
    if(event->key == InputKeyOk && event->type == InputTypeShort) {
        //Меню добавления датчика при их отсутствии
        if(current_view == G_NO_SENSORS_VIEW) {
            app->sensors_ready = false;
            unitemp_SensorsList_switch();
        } else if(current_view == G_LIST_VIEW) {
            //Переход в главное меню при выключенном селекторе
            app->sensors_ready = false;
            unitemp_MainMenu_switch();
        } else if(current_view == G_CAROUSEL_VIEW) {
            app->sensors_ready = false;
            unitemp_SensorActions_switch(unitemp_sensor_getActive(generalview_sensor_index));
        }
    }

    //Обработка короткого нажатия "вниз"
    if(event->key == InputKeyDown && event->type == InputTypeShort) {
        //Переход из значений в информацию в карусели
        if(current_view == G_CAROUSEL_VIEW && carousel_info_selector == CAROUSEL_VALUES) {
            carousel_info_selector = CAROUSEL_INFO;
            return true;
        }
        //Переход в карусель из списка
        if(current_view == G_LIST_VIEW) {
            current_view = G_CAROUSEL_VIEW;
            return true;
        }
    }

    //Обработка короткого нажатия "вверх"
    if(event->key == InputKeyUp && event->type == InputTypeShort) {
        //Переход из информации в значения в карусели
        if(current_view == G_CAROUSEL_VIEW && carousel_info_selector == CAROUSEL_INFO) {
            carousel_info_selector = CAROUSEL_VALUES;
            return true;
        }
        //Переход в список из карусели
        if(current_view == G_CAROUSEL_VIEW && carousel_info_selector == CAROUSEL_VALUES &&
           unitemp_sensors_getActiveCount() > 1) {
            current_view = G_LIST_VIEW;
            return true;
        }
    }

    //Обработка короткого нажатия "вправо"
    if(event->key == InputKeyRight && event->type == InputTypeShort) {
        //Пролистывание карусели вперёд
        if(current_view == G_CAROUSEL_VIEW) {
            if(++generalview_sensor_index >= unitemp_sensors_getActiveCount()) {
                generalview_sensor_index = 0;
                if(carousel_info_selector == CAROUSEL_VALUES) current_view = G_LIST_VIEW;
            }

            return true;
        }
        //Пролистывание списка вперёд
        if(current_view == G_LIST_VIEW) {
            generalview_sensor_index += 4;
            if(generalview_sensor_index >= unitemp_sensors_getActiveCount()) {
                generalview_sensor_index = 0;
                current_view = G_CAROUSEL_VIEW;
            }
            return true;
        }
    }

    //Обработка короткого нажатия "влево"
    if(event->key == InputKeyLeft && event->type == InputTypeShort) {
        //Пролистывание карусели назад
        if(current_view == G_CAROUSEL_VIEW) {
            if(--generalview_sensor_index >= unitemp_sensors_getActiveCount()) {
                generalview_sensor_index = unitemp_sensors_getActiveCount() - 1;
                if(carousel_info_selector == CAROUSEL_VALUES) current_view = G_LIST_VIEW;
            }

            return true;
        }
        //Пролистывание списка назад
        if(current_view == G_LIST_VIEW) {
            generalview_sensor_index -= 4;
            if(generalview_sensor_index >= unitemp_sensors_getActiveCount()) {
                generalview_sensor_index = unitemp_sensors_getActiveCount() - 1;
                current_view = G_CAROUSEL_VIEW;
            }

            return true;
        }
    }

    //Обработка короткого нажатия "назад"
    if(event->key == InputKeyBack && event->type == InputTypeShort) {
        //Выход из приложения при карусели или отсутствии датчиков
        if(current_view == G_NO_SENSORS_VIEW ||
           ((current_view == G_CAROUSEL_VIEW) && (carousel_info_selector == CAROUSEL_VALUES))) {
            app->processing = false;
            return true;
        }
        //Переключение селектора вида карусели
        if((current_view == G_CAROUSEL_VIEW) && (carousel_info_selector != CAROUSEL_VALUES)) {
            carousel_info_selector = CAROUSEL_VALUES;
            return true;
        }
        //Переход в карусель из списка
        if(current_view == G_LIST_VIEW) {
            current_view = G_CAROUSEL_VIEW;
            return true;
        }
    }

    return true;
}

void unitemp_General_alloc(void) {
    view = view_alloc();
    view_set_context(view, app);
    view_set_draw_callback(view, _draw_callback);
    view_set_input_callback(view, _input_callback);

    view_dispatcher_add_view(app->view_dispatcher, UnitempViewGeneral, view);
}

void unitemp_General_switch(void) {
    app->sensors_ready = true;
    view_dispatcher_switch_to_view(app->view_dispatcher, UnitempViewGeneral);
}

void unitemp_General_free(void) {
    view_free(view);
}
