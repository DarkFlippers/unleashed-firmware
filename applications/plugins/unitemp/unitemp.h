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
#ifndef UNITEMP
#define UNITEMP

/* Подключение стандартных библиотек */

/* Подключение API Flipper Zero */
//Файловый поток
#include <toolbox/stream/file_stream.h>
//Экран
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/widget.h>
#include <gui/modules/popup.h>
//Уведомления
#include <notification/notification.h>
#include <notification/notification_messages.h>

/* Внутренние библиотеки */
//Интерфейсы подключения датчиков
#include "Sensors.h"

/* Объявление макроподстановок */
//Имя приложения
#define APP_NAME "Unitemp"
//Путь хранения файлов плагина
#define APP_PATH_FOLDER "/ext/unitemp"
//Имя файла с настройками
#define APP_FILENAME_SETTINGS "settings.cfg"
//Имя файла с датчиками
#define APP_FILENAME_SENSORS "sensors.cfg"
//Версия приложения
#define UNITEMP_APP_VER "1.0"

//Размер буффера текста
#define BUFF_SIZE 32

#define UNITEMP_DEBUG

/* Объявление перечислений */
//Единицы измерения температуры
typedef enum { UT_TEMP_CELSIUS, UT_TEMP_FAHRENHEIT, UT_TEMP_COUNT } tempMeasureUnit;
//Единицы измерения давления
typedef enum {
    UT_PRESSURE_MM_HG,
    UT_PRESSURE_IN_HG,
    UT_PRESSURE_KPA,

    UT_PRESSURE_COUNT
} pressureMeasureUnit;
/* Объявление структур */
//Настройки плагина
typedef struct {
    //Бесконечная работа подсветки
    bool infinityBacklight;
    //Единица измерения температуры
    tempMeasureUnit temp_unit;
    //Единица измерения давления
    pressureMeasureUnit pressure_unit;
    //Последнее состояние OTG
    bool lastOTGState;
} UnitempSettings;

//Основная структура плагина
typedef struct {
    //Система
    bool processing; //Флаг работы приложения. При ложном значении приложение закрывается
    bool sensors_ready; //Флаг готовности датчиков к опросу
    //Основные настройки
    UnitempSettings settings;
    //Массив указателей на датчики
    Sensor** sensors;
    //Количество загруженных датчиков
    uint8_t sensors_count;
    //SD-карта
    Storage* storage; //Хранилище
    Stream* file_stream; //Файловый поток

    //Экран
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    NotificationApp* notifications;
    Widget* widget;
    Popup* popup;
    //Буффер для различного текста
    char* buff;
} Unitemp;

/* Объявление прототипов функций */

/**
 * @brief Перевод значения температуры датчика из Цельсия в Фаренгейты
 * 
 * @param sensor Указатель на датчик
 */
void uintemp_celsiumToFarengate(Sensor* sensor);

/**
 * @brief Конвертация давления из паскалей в мм рт.ст.
 * 
 * @param sensor Указатель на датчик
 */
void unitemp_pascalToMmHg(Sensor* sensor);

/**
 * @brief Конвертация давления из паскалей в килопаскали
 * 
 * @param sensor Указатель на датчик
 */
void unitemp_pascalToKPa(Sensor* sensor);
/**
 * @brief Конвертация давления из паскалей в дюйм рт.ст.
 * 
 * @param sensor Указатель на датчик
 */
void unitemp_pascalToInHg(Sensor* sensor);

/**
 * @brief Сохранение настроек на SD-карту
 * 
 * @return Истина если сохранение успешное
 */
bool unitemp_saveSettings(void);
/**
 * @brief Загрузка настроек с SD-карты
 * 
 * @return Истина если загрузка успешная
 */
bool unitemp_loadSettings(void);

extern Unitemp* app;
#endif