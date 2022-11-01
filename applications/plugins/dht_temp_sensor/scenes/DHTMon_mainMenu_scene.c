#include "../quenon_dht_mon.h"
//Текущий вид
static View* view;
//Список
static VariableItemList* variable_item_list;

/**
 * @brief Функция обработки нажатия кнопки "Назад"
 * 
 * @param context Указатель на данные приложения
 * @return ID вида в который нужно переключиться
 */
static uint32_t actions_exitCallback(void* context) {
    PluginData* app = context;
    UNUSED(app);
    //Возвращаем ID вида, в который нужно вернуться
    return VIEW_NONE;
}
/**
 * @brief Функция обработки нажатия средней кнопки
 * 
 * @param context Указатель на данные приложения
 * @param index На каком элементе списка была нажата кнопка
 */
static void enterCallback(void* context, uint32_t index) {
    PluginData* app = context;
    if((uint8_t)index < (uint8_t)app->sensors_count) {
        app->currentSensorEdit = &app->sensors[index];
        sensorActions_scene(app);
    }
    if((uint8_t)index == (uint8_t)app->sensors_count) {
        app->currentSensorEdit = &app->sensors[app->sensors_count++];
        strcpy(app->currentSensorEdit->name, "NewSensor");
        app->currentSensorEdit->GPIO = DHTMon_GPIO_from_index(0);
        app->currentSensorEdit->type = DHT11;
        sensorEdit_scene(app);
    }
}

/**
 * @brief Создание списка действий с указанным датчиком
 * 
 * @param app Указатель на данные плагина
 */
void mainMenu_scene(PluginData* app) {
    variable_item_list = variable_item_list_alloc();
    //Сброс всех элементов меню
    variable_item_list_reset(variable_item_list);
    //Добавление названий датчиков в качестве элементов списка
    for(uint8_t i = 0; i < app->sensors_count; i++) {
        variable_item_list_add(variable_item_list, app->sensors[i].name, 1, NULL, NULL);
    }
    if(app->sensors_count < (uint8_t)MAX_SENSORS) {
        variable_item_list_add(variable_item_list, "       + Add new sensor +", 1, NULL, NULL);
    }

    //Добавление колбека на нажатие средней кнопки
    variable_item_list_set_enter_callback(variable_item_list, enterCallback, app);

    //Создание вида из списка
    view = variable_item_list_get_view(variable_item_list);
    //Добавление колбека на нажатие кнопки "Назад"
    view_set_previous_callback(view, actions_exitCallback);
    //Добавление вида в диспетчер
    view_dispatcher_add_view(app->view_dispatcher, MAIN_MENU_VIEW, view);

    view_dispatcher_enable_queue(app->view_dispatcher);

    //Переключение на наш вид
    view_dispatcher_switch_to_view(app->view_dispatcher, MAIN_MENU_VIEW);

    //Запуск диспетчера
    view_dispatcher_run(app->view_dispatcher);

    //Очистка списка элементов
    variable_item_list_free(variable_item_list);
    //Удаление вида после обработки
    view_dispatcher_remove_view(app->view_dispatcher, MAIN_MENU_VIEW);
}

/*








































*/

// static VariableItemList* variable_item_list;
// /* ============== Главное меню ============== */
// static uint32_t mainMenu_exitCallback(void* context) {
//     UNUSED(context);
//     variable_item_list_free(variable_item_list);
//     DHT_sensors_reload();
//     return VIEW_NONE;
// }
// static void mainMenu_enterCallback(void* context, uint32_t index) {
//     PluginData* app = context;
//     if((uint8_t)index == (uint8_t)app->sensors_count) {
//         addSensor_scene(app);
//         view_dispatcher_run(app->view_dispatcher);
//     }
// }
// void mainMenu_scene(PluginData* app) {
//     variable_item_list = variable_item_list_alloc();
//     variable_item_list_reset(variable_item_list);
//     for(uint8_t i = 0; i < app->sensors_count; i++) {
//         variable_item_list_add(variable_item_list, app->sensors[i].name, 1, NULL, NULL);
//     }
//     variable_item_list_add(variable_item_list, "+ Add new sensor +", 1, NULL, NULL);

//     app->view = variable_item_list_get_view(variable_item_list);
//     app->view_dispatcher = view_dispatcher_alloc();

//     view_dispatcher_enable_queue(app->view_dispatcher);
//     view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
//     view_dispatcher_add_view(app->view_dispatcher, MAIN_MENU_VIEW, app->view);
//     view_dispatcher_switch_to_view(app->view_dispatcher, MAIN_MENU_VIEW);

//     variable_item_list_set_enter_callback(variable_item_list, mainMenu_enterCallback, app);
//     view_set_previous_callback(app->view, mainMenu_exitCallback);
// }