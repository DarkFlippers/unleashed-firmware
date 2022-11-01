#ifndef QUENON_DHT_MON
#define QUENON_DHT_MON

#include <stdlib.h>
#include <stdio.h>

#include <furi.h>
#include <furi_hal_power.h>

#include <gui/gui.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <gui/modules/text_input.h>
#include <gui/modules/widget.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>

#include <notification/notification.h>
#include <notification/notification_messages.h>

#include <toolbox/stream/file_stream.h>
#include <input/input.h>

#include "DHT.h"

#define APP_NAME "DHT_monitor"
#define APP_PATH_FOLDER "/ext/dht_monitor"
#define APP_FILENAME "sensors.txt"
#define MAX_SENSORS 5

// //Виды менюшек
typedef enum {
    MAIN_MENU_VIEW,
    ADDSENSOR_MENU_VIEW,
    TEXTINPUT_VIEW,
    SENSOR_ACTIONS_VIEW,
    WIDGET_VIEW,
} MENU_VIEWS;

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    const uint8_t num;
    const char* name;
    const GpioPin* pin;
} GpioItem;

//Структура с данными плагина
typedef struct {
    //Очередь сообщений
    FuriMessageQueue* event_queue;
    //Мутекс
    ValueMutex state_mutex;
    //Вьюпорт
    ViewPort* view_port;
    //GUI
    Gui* gui;
    NotificationApp* notifications;
    ViewDispatcher* view_dispatcher;
    View* view;
    TextInput* text_input;
    VariableItem* item;
    Widget* widget;

    char txtbuff[30]; //Буффер для печати строк на экране
    bool last_OTG_State; //Состояние OTG до запуска приложения
    Storage* storage; //Хранилище датчиков
    Stream* file_stream; //Поток файла с датчиками
    int8_t sensors_count; // Количество загруженных датчиков
    DHT_sensor sensors[MAX_SENSORS]; //Сохранённые датчики
    DHT_data data; //Инфа из датчика
    DHT_sensor* currentSensorEdit; //Указатель на редактируемый датчик

} PluginData;

/* ================== Работа с GPIO ================== */
/**
 * @brief Конвертация GPIO в его номер на корпусе FZ
 * 
 * @param gpio Указатель на преобразовываемый GPIO
 * @return Номер порта на корпусе FZ
 */
uint8_t DHTMon_GPIO_to_int(const GpioPin* gpio);
/**
 * @brief Конвертация номера порта на корпусе FZ в GPIO 
 * 
 * @param name Номер порта на корпусе FZ
 * @return Указатель на GPIO при успехе, NULL при ошибке
 */
const GpioPin* DHTMon_GPIO_form_int(uint8_t name);
/**
 * @brief Преобразование порядкового номера порта в GPIO
 * 
 * @param index Индекс порта от 0 до GPIO_ITEMS-1
 * @return Указатель на GPIO при успехе, NULL при ошибке
 */
const GpioPin* DHTMon_GPIO_from_index(uint8_t index);
/**
 * @brief Преобразование GPIO в порядковый номер порта
 * 
 * @param gpio Указатель на GPIO
 * @return index при успехе, 255 при ошибке
 */
uint8_t DHTMon_GPIO_to_index(const GpioPin* gpio);

/**
 * @brief Получить имя GPIO в виде строки
 * 
 * @param gpio Искомый порт
 * @return char* Указатель на строку с именем порта
 */
const char* DHTMon_GPIO_getName(const GpioPin* gpio);

/* ================== Работа с датчиками ================== */
/**
 * @brief Инициализация портов ввода/вывода датчиков
 */
void DHTMon_sensors_init(void);
/**
 * @brief Функция деинициализации портов ввода/вывода датчиков
 */
void DHTMon_sensors_deinit(void);
/**
 * @brief Проверка корректности параметров датчика
 * 
 * @param sensor Указатель на проверяемый датчик
 * @return true Параметры датчика корректные
 * @return false Параметры датчика некорректные
 */
bool DHTMon_sensor_check(DHT_sensor* sensor);
/**
 * @brief Удаление датчика из списка и перезагрузка
 * 
 * @param sensor Указатель на удаляемый датчик
 */
void DHTMon_sensor_delete(DHT_sensor* sensor);
/**
 * @brief Сохранение датчиков на SD-карту
 * 
 * @return Количество сохранённых датчиков
 */
uint8_t DHTMon_sensors_save(void);
/**
 * @brief Загрузка датчиков с SD-карты
 * 
 * @return true Был загружен хотя бы 1 датчик
 * @return false Датчики отсутствуют
 */
bool DHTMon_sensors_load(void);
/**
 * @brief Перезагрузка датчиков с SD-карты
 * 
 * @return true Когда был загружен хотя бы 1 датчик
 * @return false Ни один из датчиков не был загружен
 */
bool DHTMon_sensors_reload(void);

void scene_main(Canvas* const canvas, PluginData* app);
void mainMenu_scene(PluginData* app);

void sensorEdit_sceneCreate(PluginData* app);
void sensorEdit_scene(PluginData* app);
void sensorEdit_sceneRemove(void);

void sensorActions_sceneCreate(PluginData* app);
void sensorActions_scene(PluginData* app);
void sensorActions_screneRemove(void);
#endif