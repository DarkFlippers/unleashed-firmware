#include "../quenon_dht_mon.h"

static VariableItem* nameItem;
static VariableItemList* variable_item_list;

static const char* const sensorsTypes[2] = {
    "DHT11",
    "DHT22",
};

// /* ============== Добавление датчика ============== */
static uint32_t addSensor_exitCallback(void* context) {
    UNUSED(context);
    DHTMon_sensors_reload();
    return VIEW_NONE;
}

static void addSensor_sensorTypeChanged(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    PluginData* app = variable_item_get_context(item);
    variable_item_set_current_value_text(item, sensorsTypes[index]);
    app->currentSensorEdit->type = index;
}

static void addSensor_GPIOChanged(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, DHTMon_GPIO_getName(DHTMon_GPIO_from_index(index)));
    PluginData* app = variable_item_get_context(item);
    app->currentSensorEdit->GPIO = DHTMon_GPIO_from_index(index);
}

static void addSensor_sensorNameChanged(void* context) {
    PluginData* app = context;
    variable_item_set_current_value_text(nameItem, app->currentSensorEdit->name);
    view_dispatcher_switch_to_view(app->view_dispatcher, ADDSENSOR_MENU_VIEW);
}
static void addSensor_sensorNameChange(PluginData* app) {
    text_input_set_header_text(app->text_input, "Sensor name");
    //По неясной мне причине в длину строки входит терминатор. Поэтому при длине 10 приходится указывать 11
    text_input_set_result_callback(
        app->text_input, addSensor_sensorNameChanged, app, app->currentSensorEdit->name, 11, true);
    view_dispatcher_switch_to_view(app->view_dispatcher, TEXTINPUT_VIEW);
}

static void addSensor_enterCallback(void* context, uint32_t index) {
    PluginData* app = context;
    if(index == 0) {
        addSensor_sensorNameChange(app);
    }
    if(index == 3) {
        //Сохранение датчика
        DHTMon_sensors_save();
        DHTMon_sensors_reload();
        view_dispatcher_switch_to_view(app->view_dispatcher, VIEW_NONE);
    }
}

void sensorEdit_sceneCreate(PluginData* app) {
    variable_item_list = variable_item_list_alloc();

    variable_item_list_reset(variable_item_list);

    variable_item_list_set_enter_callback(variable_item_list, addSensor_enterCallback, app);

    app->view = variable_item_list_get_view(variable_item_list);

    view_set_previous_callback(app->view, addSensor_exitCallback);

    view_dispatcher_add_view(app->view_dispatcher, ADDSENSOR_MENU_VIEW, app->view);
}
void sensorEdit_scene(PluginData* app) {
    //Очистка списка
    variable_item_list_reset(variable_item_list);

    //Имя редактируемого датчика
    nameItem = variable_item_list_add(variable_item_list, "Name: ", 1, NULL, NULL);
    variable_item_set_current_value_index(nameItem, 0);
    variable_item_set_current_value_text(nameItem, app->currentSensorEdit->name);

    //Тип датчика
    app->item =
        variable_item_list_add(variable_item_list, "Type:", 2, addSensor_sensorTypeChanged, app);

    variable_item_set_current_value_index(nameItem, app->currentSensorEdit->type);
    variable_item_set_current_value_text(app->item, sensorsTypes[app->currentSensorEdit->type]);

    //GPIO
    app->item =
        variable_item_list_add(variable_item_list, "GPIO:", 13, addSensor_GPIOChanged, app);
    variable_item_set_current_value_index(
        app->item, DHTMon_GPIO_to_index(app->currentSensorEdit->GPIO));
    variable_item_set_current_value_text(
        app->item, DHTMon_GPIO_getName(app->currentSensorEdit->GPIO));
    variable_item_list_add(variable_item_list, "Save", 1, NULL, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, ADDSENSOR_MENU_VIEW);
}
void sensorEdit_sceneRemove(void) {
    variable_item_list_free(variable_item_list);
}