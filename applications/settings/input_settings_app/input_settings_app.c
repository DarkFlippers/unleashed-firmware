#include <stdint.h>
#include "input_settings_app.h"

#define TAG "InputSettingsApp"

#define VIBRO_TOUCH_LEVEL_COUNT        10
#define VIBRO_TOUCH_TRIGGER_MASK_COUNT 3

// vibro touch human readable levels
const char* const vibro_touch_level_text[VIBRO_TOUCH_LEVEL_COUNT] = {
    "OFF",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
};
// vibro touch levels tick valies delay
const uint32_t vibro_touch_level_value[VIBRO_TOUCH_LEVEL_COUNT] =
    {0, 13, 16, 19, 21, 24, 27, 30, 33, 36};
// vibro touch trigger mask human readable values
const char* const vibro_touch_trigger_mask_text[VIBRO_TOUCH_TRIGGER_MASK_COUNT] = {
    "Press",
    "Release",
    "Both",
};
// vibro touch trigger mask values
const uint32_t vibro_touch_trigger_mask_value[VIBRO_TOUCH_TRIGGER_MASK_COUNT] = {
    (1 << InputTypePress),
    (1 << InputTypeRelease),
    (1 << InputTypePress) | (1 << InputTypeRelease),
};

static void input_settings_vibro_touch_level_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, vibro_touch_level_text[index]);

    InputSettingsApp* app = variable_item_get_context(item);
    app->settings->vibro_touch_level = vibro_touch_level_value[index];

    // use RECORD for access to input service instance and set settings
    InputSettings* service_settings = furi_record_open(RECORD_INPUT_SETTINGS);
    service_settings->vibro_touch_level = vibro_touch_level_value[index];
    furi_record_close(RECORD_INPUT_SETTINGS);
}

static void input_settings_vibro_touch_trigger_mask_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, vibro_touch_trigger_mask_text[index]);

    InputSettingsApp* app = variable_item_get_context(item);
    app->settings->vibro_touch_trigger_mask = vibro_touch_trigger_mask_value[index];

    // use RECORD for access to input service instance and set settings
    InputSettings* service_settings = furi_record_open(RECORD_INPUT_SETTINGS);
    service_settings->vibro_touch_trigger_mask = vibro_touch_trigger_mask_value[index];
    furi_record_close(RECORD_INPUT_SETTINGS);
}

static uint32_t input_settings_app_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

InputSettingsApp* input_settings_app_alloc(void) {
    InputSettingsApp* app = malloc(sizeof(InputSettingsApp));
    app->gui = furi_record_open(RECORD_GUI);

    app->settings = malloc(sizeof(InputSettings));
    input_settings_load(app->settings);

    app->variable_item_list = variable_item_list_alloc();
    View* view = variable_item_list_get_view(app->variable_item_list);
    view_set_previous_callback(view, input_settings_app_exit);

    VariableItem* item;
    uint8_t value_index;

    item = variable_item_list_add(
        app->variable_item_list,
        "Buttons Vibro",
        VIBRO_TOUCH_LEVEL_COUNT,
        input_settings_vibro_touch_level_changed,
        app);

    value_index = value_index_uint32(
        app->settings->vibro_touch_level, vibro_touch_level_value, VIBRO_TOUCH_LEVEL_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, vibro_touch_level_text[value_index]);

    item = variable_item_list_add(
        app->variable_item_list,
        "Vibro Trigger",
        VIBRO_TOUCH_TRIGGER_MASK_COUNT,
        input_settings_vibro_touch_trigger_mask_changed,
        app);

    value_index = value_index_uint32(
        app->settings->vibro_touch_trigger_mask,
        vibro_touch_trigger_mask_value,
        VIBRO_TOUCH_TRIGGER_MASK_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, vibro_touch_trigger_mask_text[value_index]);

    // create and setup view and view dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(app->view_dispatcher, InputSettingsViewVariableItemList, view);
    view_dispatcher_switch_to_view(app->view_dispatcher, InputSettingsViewVariableItemList);

    return app;
}

void input_settings_app_free(InputSettingsApp* app) {
    furi_assert(app);

    // Variable item list
    view_dispatcher_remove_view(app->view_dispatcher, InputSettingsViewVariableItemList);
    variable_item_list_free(app->variable_item_list);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);

    // Records
    furi_record_close(RECORD_GUI);
    free(app->settings);
    free(app);
}

// Enter point
int32_t input_settings_app(void* p) {
    UNUSED(p);
    InputSettingsApp* app = input_settings_app_alloc();

    view_dispatcher_run(app->view_dispatcher);

    //save current settings;
    input_settings_save(app->settings);

    input_settings_app_free(app);
    return 0;
}
