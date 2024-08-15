#include <furi_hal.h>
#include <furi.h>

// Need access to u8g2
#include <gui/gui_i.h>
#include <gui/canvas_i.h>
#include <u8g2_glue.h>

#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>

#include "view_display_test.h"

#define TAG "DisplayTest"

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    ViewDisplayTest* view_display_test;
    VariableItemList* variable_item_list;
    Submenu* submenu;

    bool config_bias;
    uint8_t config_contrast;
    uint8_t config_regulation_ratio;
} DisplayTest;

typedef enum {
    DisplayTestViewSubmenu,
    DisplayTestViewConfigure,
    DisplayTestViewDisplayTest,
} DisplayTestView;

const bool config_bias_value[] = {
    true,
    false,
};
const char* const config_bias_text[] = {
    "1/7",
    "1/9",
};

const uint8_t config_regulation_ratio_value[] = {
    0b000,
    0b001,
    0b010,
    0b011,
    0b100,
    0b101,
    0b110,
    0b111,
};
const char* const config_regulation_ratio_text[] = {
    "3.0",
    "3.5",
    "4.0",
    "4.5",
    "5.0",
    "5.5",
    "6.0",
    "6.5",
};

static void display_test_submenu_callback(void* context, uint32_t index) {
    DisplayTest* instance = (DisplayTest*)context;
    view_dispatcher_switch_to_view(instance->view_dispatcher, index);
}

static uint32_t display_test_previous_callback(void* context) {
    UNUSED(context);
    return DisplayTestViewSubmenu;
}

static uint32_t display_test_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static void display_test_reload_config(DisplayTest* instance) {
    FURI_LOG_I(
        TAG,
        "contrast: %d, regulation_ratio: %d, bias: %d",
        instance->config_contrast,
        instance->config_regulation_ratio,
        instance->config_bias);
    u8x8_d_st756x_init(
        &instance->gui->canvas->fb.u8x8,
        instance->config_contrast,
        instance->config_regulation_ratio,
        instance->config_bias);
}

static void display_config_set_bias(VariableItem* item) {
    DisplayTest* instance = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, config_bias_text[index]);
    instance->config_bias = config_bias_value[index];
    display_test_reload_config(instance);
}

static void display_config_set_regulation_ratio(VariableItem* item) {
    DisplayTest* instance = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, config_regulation_ratio_text[index]);
    instance->config_regulation_ratio = config_regulation_ratio_value[index];
    display_test_reload_config(instance);
}

static void display_config_set_contrast(VariableItem* item) {
    DisplayTest* instance = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    FuriString* temp;
    temp = furi_string_alloc();
    furi_string_cat_printf(temp, "%d", index);
    variable_item_set_current_value_text(item, furi_string_get_cstr(temp));
    furi_string_free(temp);
    instance->config_contrast = index;
    display_test_reload_config(instance);
}

DisplayTest* display_test_alloc(void) {
    DisplayTest* instance = malloc(sizeof(DisplayTest));

    View* view = NULL;

    instance->gui = furi_record_open(RECORD_GUI);
    instance->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(
        instance->view_dispatcher, instance->gui, ViewDispatcherTypeFullscreen);

    // Test
    instance->view_display_test = view_display_test_alloc();
    view = view_display_test_get_view(instance->view_display_test);
    view_set_previous_callback(view, display_test_previous_callback);
    view_dispatcher_add_view(instance->view_dispatcher, DisplayTestViewDisplayTest, view);

    // Configure
    instance->variable_item_list = variable_item_list_alloc();
    view = variable_item_list_get_view(instance->variable_item_list);
    view_set_previous_callback(view, display_test_previous_callback);
    view_dispatcher_add_view(instance->view_dispatcher, DisplayTestViewConfigure, view);

    // Configuration items
    VariableItem* item;
    instance->config_bias = false;
    instance->config_contrast = 32;
    instance->config_regulation_ratio = 0b101;
    // Bias
    item = variable_item_list_add(
        instance->variable_item_list,
        "Bias:",
        COUNT_OF(config_bias_value),
        display_config_set_bias,
        instance);
    variable_item_set_current_value_index(item, 1);
    variable_item_set_current_value_text(item, config_bias_text[1]);
    // Regulation Ratio
    item = variable_item_list_add(
        instance->variable_item_list,
        "Reg Ratio:",
        COUNT_OF(config_regulation_ratio_value),
        display_config_set_regulation_ratio,
        instance);
    variable_item_set_current_value_index(item, 5);
    variable_item_set_current_value_text(item, config_regulation_ratio_text[5]);
    // Contrast
    item = variable_item_list_add(
        instance->variable_item_list, "Contrast:", 64, display_config_set_contrast, instance);
    variable_item_set_current_value_index(item, 32);
    variable_item_set_current_value_text(item, "32");

    // Menu
    instance->submenu = submenu_alloc();
    view = submenu_get_view(instance->submenu);
    view_set_previous_callback(view, display_test_exit_callback);
    view_dispatcher_add_view(instance->view_dispatcher, DisplayTestViewSubmenu, view);
    submenu_add_item(
        instance->submenu,
        "Test",
        DisplayTestViewDisplayTest,
        display_test_submenu_callback,
        instance);
    submenu_add_item(
        instance->submenu,
        "Configure",
        DisplayTestViewConfigure,
        display_test_submenu_callback,
        instance);

    return instance;
}

void display_test_free(DisplayTest* instance) {
    view_dispatcher_remove_view(instance->view_dispatcher, DisplayTestViewSubmenu);
    submenu_free(instance->submenu);

    view_dispatcher_remove_view(instance->view_dispatcher, DisplayTestViewConfigure);
    variable_item_list_free(instance->variable_item_list);

    view_dispatcher_remove_view(instance->view_dispatcher, DisplayTestViewDisplayTest);
    view_display_test_free(instance->view_display_test);

    view_dispatcher_free(instance->view_dispatcher);
    furi_record_close(RECORD_GUI);

    free(instance);
}

int32_t display_test_run(DisplayTest* instance) {
    UNUSED(instance);
    view_dispatcher_switch_to_view(instance->view_dispatcher, DisplayTestViewSubmenu);
    view_dispatcher_run(instance->view_dispatcher);

    return 0;
}

int32_t display_test_app(void* p) {
    UNUSED(p);

    DisplayTest* instance = display_test_alloc();

    int32_t ret = display_test_run(instance);

    display_test_free(instance);

    return ret;
}
