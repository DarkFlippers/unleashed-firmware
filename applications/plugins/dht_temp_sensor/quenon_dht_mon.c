#include "quenon_dht_mon.h"
#include <m-string.h>

//Порты ввода/вывода, которые не были обозначены в общем списке
const GpioPin SWC_10 = {.pin = LL_GPIO_PIN_14, .port = GPIOA};
const GpioPin SIO_12 = {.pin = LL_GPIO_PIN_13, .port = GPIOA};
const GpioPin TX_13 = {.pin = LL_GPIO_PIN_6, .port = GPIOB};
const GpioPin RX_14 = {.pin = LL_GPIO_PIN_7, .port = GPIOB};

//Количество доступных портов ввода/вывода
#define GPIO_ITEMS (sizeof(gpio_item) / sizeof(GpioItem))

//Перечень достуных портов ввода/вывода
static const GpioItem gpio_item[] = {
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

//Данные плагина
static PluginData* app;

uint8_t DHTMon_GPIO_to_int(const GpioPin* gpio) {
    if(gpio == NULL) return 255;
    for(uint8_t i = 0; i < GPIO_ITEMS; i++) {
        if(gpio_item[i].pin->pin == gpio->pin && gpio_item[i].pin->port == gpio->port) {
            return gpio_item[i].num;
        }
    }
    return 255;
}

const GpioPin* DHTMon_GPIO_form_int(uint8_t name) {
    for(uint8_t i = 0; i < GPIO_ITEMS; i++) {
        if(gpio_item[i].num == name) {
            return gpio_item[i].pin;
        }
    }
    return NULL;
}

const GpioPin* DHTMon_GPIO_from_index(uint8_t index) {
    if(index > GPIO_ITEMS) return NULL;
    return gpio_item[index].pin;
}

uint8_t DHTMon_GPIO_to_index(const GpioPin* gpio) {
    if(gpio == NULL) return 255;
    for(uint8_t i = 0; i < GPIO_ITEMS; i++) {
        if(gpio_item[i].pin->pin == gpio->pin && gpio_item[i].pin->port == gpio->port) {
            return i;
        }
    }
    return 255;
}

const char* DHTMon_GPIO_getName(const GpioPin* gpio) {
    if(gpio == NULL) return NULL;
    for(uint8_t i = 0; i < GPIO_ITEMS; i++) {
        if(gpio_item[i].pin->pin == gpio->pin && gpio_item[i].pin->port == gpio->port) {
            return gpio_item[i].name;
        }
    }
    return NULL;
}

void DHTMon_sensors_init(void) {
    //Включение 5V если на порту 1 FZ его нет
    if(furi_hal_power_is_otg_enabled() != true) {
        furi_hal_power_enable_otg();
    }

    //Настройка GPIO загруженных датчиков
    for(uint8_t i = 0; i < app->sensors_count; i++) {
        //Высокий уровень по умолчанию
        furi_hal_gpio_write(app->sensors[i].GPIO, true);
        //Режим работы - OpenDrain, подтяжка включается на всякий случай
        furi_hal_gpio_init(
            app->sensors[i].GPIO, //Порт FZ
            GpioModeOutputOpenDrain, //Режим работы - открытый сток
            GpioPullUp, //Принудительная подтяжка линии данных к питанию
            GpioSpeedVeryHigh); //Скорость работы - максимальная
    }
}

void DHTMon_sensors_deinit(void) {
    //Возврат исходного состояния 5V
    if(app->last_OTG_State != true) {
        furi_hal_power_disable_otg();
    }

    //Перевод портов GPIO в состояние по умолчанию
    for(uint8_t i = 0; i < app->sensors_count; i++) {
        furi_hal_gpio_init(
            app->sensors[i].GPIO, //Порт FZ
            GpioModeAnalog, //Режим работы - аналог
            GpioPullNo, //Отключение подтяжки
            GpioSpeedLow); //Скорость работы - низкая
        //Установка низкого уровня
        furi_hal_gpio_write(app->sensors[i].GPIO, false);
    }
}

bool DHTMon_sensor_check(DHT_sensor* sensor) {
    /* Проверка имени */
    //1) Строка должна быть длиной от 1 до 10 символов
    //2) Первый символ строки должен быть только 0-9, A-Z, a-z и _
    if(strlen(sensor->name) == 0 || strlen(sensor->name) > 10 ||
       (!(sensor->name[0] >= '0' && sensor->name[0] <= '9') &&
        !(sensor->name[0] >= 'A' && sensor->name[0] <= 'Z') &&
        !(sensor->name[0] >= 'a' && sensor->name[0] <= 'z') && !(sensor->name[0] == '_'))) {
        FURI_LOG_D(APP_NAME, "Sensor [%s] name check failed\r\n", sensor->name);
        return false;
    }
    //Проверка GPIO
    if(DHTMon_GPIO_to_int(sensor->GPIO) == 255) {
        FURI_LOG_D(
            APP_NAME,
            "Sensor [%s] GPIO check failed: %d\r\n",
            sensor->name,
            DHTMon_GPIO_to_int(sensor->GPIO));
        return false;
    }
    //Проверка типа датчика
    if(sensor->type != DHT11 && sensor->type != DHT22) {
        FURI_LOG_D(APP_NAME, "Sensor [%s] type check failed: %d\r\n", sensor->name, sensor->type);
        return false;
    }

    //Возврат истины если всё ок
    FURI_LOG_D(APP_NAME, "Sensor [%s] all checks passed\r\n", sensor->name);
    return true;
}

void DHTMon_sensor_delete(DHT_sensor* sensor) {
    if(sensor == NULL) return;
    //Делаем параметры датчика неверными
    sensor->name[0] = '\0';
    sensor->type = 255;
    //Теперь сохраняем текущие датчики. Сохранятор не сохранит неисправный датчик
    DHTMon_sensors_save();
    //Перезагружаемся с SD-карты
    DHTMon_sensors_reload();
}

uint8_t DHTMon_sensors_save(void) {
    //Выделение памяти для потока
    app->file_stream = file_stream_alloc(app->storage);
    uint8_t savedSensorsCount = 0;
    //Переменная пути к файлу
    FuriString* filepath = furi_string_alloc();
    //Составление пути к файлу
    furi_string_printf(filepath, "%s/%s", APP_PATH_FOLDER, APP_FILENAME);

    //Открытие потока. Если поток открылся, то выполнение сохранения датчиков
    if(file_stream_open(
           app->file_stream, furi_string_get_cstr(filepath), FSAM_READ_WRITE, FSOM_CREATE_ALWAYS)) {
        const char template[] =
            "#DHT monitor sensors file\n#Name - name of sensor. Up to 10 sumbols\n#Type - type of sensor. DHT11 - 0, DHT22 - 1\n#GPIO - connection port. May being 2-7, 10, 12-17\n#Name Type GPIO\n";
        stream_write(app->file_stream, (uint8_t*)template, strlen(template));
        //Сохранение датчиков
        for(uint8_t i = 0; i < app->sensors_count; i++) {
            //Если параметры датчика верны, то сохраняемся
            if(DHTMon_sensor_check(&app->sensors[i])) {
                stream_write_format(
                    app->file_stream,
                    "%s %d %d\n",
                    app->sensors[i].name,
                    app->sensors[i].type,
                    DHTMon_GPIO_to_int(app->sensors[i].GPIO));
                savedSensorsCount++;
            }
        }
    } else {
        //TODO: печать ошибки на экран
        FURI_LOG_E(APP_NAME, "cannot create sensors file\r\n");
    }
    stream_free(app->file_stream);

    return savedSensorsCount;
}

bool DHTMon_sensors_load(void) {
    //Обнуление количества датчиков
    app->sensors_count = -1;
    //Очистка предыдущих датчиков
    memset(app->sensors, 0, sizeof(app->sensors));

    //Открытие файла на SD-карте
    //Выделение памяти для потока
    app->file_stream = file_stream_alloc(app->storage);
    //Переменная пути к файлу
    FuriString* filepath = furi_string_alloc();
    //Составление пути к файлу
    furi_string_printf(filepath, "%s/%s", APP_PATH_FOLDER, APP_FILENAME);
    //Открытие потока к файлу
    if(!file_stream_open(
           app->file_stream, furi_string_get_cstr(filepath), FSAM_READ_WRITE, FSOM_OPEN_EXISTING)) {
        //Если файл отсутствует, то создание болванки
        FURI_LOG_W(APP_NAME, "Missing sensors file. Creating new file\r\n");
        app->sensors_count = 0;
        stream_free(app->file_stream);
        DHTMon_sensors_save();
        return false;
    }
    //Вычисление размера файла
    size_t file_size = stream_size(app->file_stream);
    if(file_size == (size_t)0) {
        //Выход если файл пустой
        FURI_LOG_W(APP_NAME, "Sensors file is empty\r\n");
        app->sensors_count = 0;
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
        FURI_LOG_E(APP_NAME, "Error reading sensor file\r\n");
        app->sensors_count = 0;
        stream_free(app->file_stream);
        return false;
    }
    //Построчное чтение файла
    //Указатель на начало строки
    FuriString* file = furi_string_alloc_set_str((char*)file_buf);
    //Сколько байт до конца строки
    size_t line_end = 0;
    while(line_end != STRING_FAILURE && app->sensors_count < MAX_SENSORS) {
        if(((char*)(file_buf + line_end))[1] != '#') {
            DHT_sensor s = {0};
            int type, port;
            char name[11] = {0};
            sscanf(((char*)(file_buf + line_end)), "%s %d %d", name, &type, &port);
            s.type = type;
            s.GPIO = DHTMon_GPIO_form_int(port);

            name[10] = '\0';
            strcpy(s.name, name);
            //Если данные корректны, то
            if(DHTMon_sensor_check(&s) == true) {
                //Установка нуля при первом датчике
                if(app->sensors_count == -1) app->sensors_count = 0;
                //Добавление датчика в общий список
                app->sensors[app->sensors_count] = s;
                //Увеличение количества загруженных датчиков
                app->sensors_count++;
            }
        }
        line_end = furi_string_search_char(file, '\n', line_end + 1);
    }
    stream_free(app->file_stream);
    free(file_buf);

    //Обнуление количества датчиков если ни один из них не был загружен
    if(app->sensors_count == -1) app->sensors_count = 0;

    //Инициализация портов датчиков если таковые есть
    if(app->sensors_count > 0) {
        DHTMon_sensors_init();
        return true;
    } else {
        return false;
    }
    return false;
}

bool DHTMon_sensors_reload(void) {
    DHTMon_sensors_deinit();
    return DHTMon_sensors_load();
}

/**
 * @brief Обработчик отрисовки экрана
 * 
 * @param canvas Указатель на холст
 * @param ctx Данные плагина
 */
static void render_callback(Canvas* const canvas, void* ctx) {
    PluginData* app = acquire_mutex((ValueMutex*)ctx, 25);
    if(app == NULL) {
        return;
    }
    //Вызов отрисовки главного экрана
    scene_main(canvas, app);

    release_mutex((ValueMutex*)ctx, app);
}

/**
 * @brief Обработчик нажатия кнопок главного экрана
 * 
 * @param input_event Указатель на событие
 * @param event_queue Указатель на очередь событий
 */
static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

/**
 * @brief Выделение места под переменные плагина
 * 
 * @return true Если всё прошло успешно
 * @return false Если в процессе загрузки произошла ошибка
 */
static bool DHTMon_alloc(void) {
    //Выделение места под данные плагина
    app = malloc(sizeof(PluginData));
    //Выделение места под очередь событий
    app->event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    //Обнуление количества датчиков
    app->sensors_count = -1;

    //Инициализация мутекса
    if(!init_mutex(&app->state_mutex, app, sizeof(PluginData))) {
        FURI_LOG_E(APP_NAME, "cannot create mutex\r\n");
        return false;
    }

    // Set system callbacks
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, render_callback, &app->state_mutex);
    view_port_input_callback_set(app->view_port, input_callback, app->event_queue);

    // Open GUI and register view_port
    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    app->view_dispatcher = view_dispatcher_alloc();

    sensorActions_sceneCreate(app);
    sensorEdit_sceneCreate(app);

    app->widget = widget_alloc();
    view_dispatcher_add_view(app->view_dispatcher, WIDGET_VIEW, widget_get_view(app->widget));

    app->text_input = text_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, TEXTINPUT_VIEW, text_input_get_view(app->text_input));

    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    //Уведомления
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    //Подготовка хранилища
    app->storage = furi_record_open(RECORD_STORAGE);
    storage_common_mkdir(app->storage, APP_PATH_FOLDER);
    app->file_stream = file_stream_alloc(app->storage);

    return true;
}

/**
 * @brief Освыбождение памяти после работы приложения
 */
static void DHTMon_free(void) {
    //Автоматическое управление подсветкой
    notification_message(app->notifications, &sequence_display_backlight_enforce_auto);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_NOTIFICATION);

    text_input_free(app->text_input);
    widget_free(app->widget);
    sensorEdit_sceneRemove();
    sensorActions_screneRemove();
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);

    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);

    view_port_free(app->view_port);
    furi_message_queue_free(app->event_queue);
    delete_mutex(&app->state_mutex);

    free(app);
}

/**
 * @brief Точка входа в приложение
 * 
 * @return Код ошибки
 */
int32_t quenon_dht_mon_app() {
    if(!DHTMon_alloc()) {
        DHTMon_free();
        return 255;
    }
    //Постоянное свечение подсветки
    notification_message(app->notifications, &sequence_display_backlight_enforce_on);
    //Сохранение состояния наличия 5V на порту 1 FZ
    app->last_OTG_State = furi_hal_power_is_otg_enabled();

    //Загрузка датчиков с SD-карты
    DHTMon_sensors_load();

    app->currentSensorEdit = &app->sensors[0];

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(app->event_queue, &event, 100);

        acquire_mutex_block(&app->state_mutex);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        break;
                    case InputKeyDown:
                        break;
                    case InputKeyRight:
                        break;
                    case InputKeyLeft:
                        break;
                    case InputKeyMAX:
                        break;
                    case InputKeyOk:
                        view_port_update(app->view_port);
                        release_mutex(&app->state_mutex, app);
                        mainMenu_scene(app);
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    default:
                        break;
                    }
                }
            }
        } else {
            FURI_LOG_D(APP_NAME, "FuriMessageQueue: event timeout");
            // event timeout
        }

        view_port_update(app->view_port);
        release_mutex(&app->state_mutex, app);
    }
    //Освобождение памяти и деинициализация
    DHTMon_sensors_deinit();
    DHTMon_free();

    return 0;
}
//TODO: Обработка ошибок
//TODO: Пропуск использованных портов в меню добавления датчиков