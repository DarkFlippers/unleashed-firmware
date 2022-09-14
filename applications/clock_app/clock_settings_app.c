#include <furi.h>
#include <gui/modules/variable_item_list.h>
#include <gui/view_dispatcher.h>
#include <lib/toolbox/value_index.h>
#include "clock_settings.h"

#define TAG "Clock"

typedef struct {
    ClockSettings clock_settings;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    VariableItemList* variable_item_list;
} ClockAppSettings;

static uint32_t clock_app_settings_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

#define TIME_FORMAT_COUNT 2
const char* const time_format_text[TIME_FORMAT_COUNT] = {
    "12h",
    "24h",
};

const uint32_t time_format_value[TIME_FORMAT_COUNT] = {H12, H24};

#define DATE_FORMAT_COUNT 2
const char* const date_format_text[DATE_FORMAT_COUNT] = {
    "mm-dd", // ISO 8601
    "dd-mm", // RFC 5322
};

const uint32_t date_format_value[DATE_FORMAT_COUNT] = {Iso, Rfc};

static void time_format_changed(VariableItem* item) {
    ClockAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, time_format_text[index]);
    app->clock_settings.time_format = time_format_value[index];
}

static void date_format_changed(VariableItem* item) {
    ClockAppSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, date_format_text[index]);
    app->clock_settings.date_format = date_format_value[index];
}

static ClockAppSettings* alloc_settings() {
    ClockAppSettings* app = malloc(sizeof(ClockAppSettings));
    LOAD_CLOCK_SETTINGS(&app->clock_settings);
    app->gui = furi_record_open(RECORD_GUI);
    app->variable_item_list = variable_item_list_alloc();
    View* view = variable_item_list_get_view(app->variable_item_list);
    view_set_previous_callback(view, clock_app_settings_exit);

    VariableItem* item;
    uint8_t value_index;

    item = variable_item_list_add(
        app->variable_item_list, "Clock format", TIME_FORMAT_COUNT, time_format_changed, app);
    value_index = value_index_uint32(
        (uint32_t)(app->clock_settings.time_format), time_format_value, TIME_FORMAT_COUNT);
    //FURI_LOG_T(TAG, "Time format index: %u", value_index);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, time_format_text[value_index]);

    item = variable_item_list_add(
        app->variable_item_list, "Date format", DATE_FORMAT_COUNT, date_format_changed, app);
    value_index = value_index_uint32(
        (uint32_t)(app->clock_settings.date_format), date_format_value, DATE_FORMAT_COUNT);
    //FURI_LOG_T(TAG, "Date format index: %u", value_index);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, date_format_text[value_index]);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_add_view(app->view_dispatcher, 0, view);
    view_dispatcher_switch_to_view(app->view_dispatcher, 0);

    return app;
}

static void free_settings(ClockAppSettings* app) {
    view_dispatcher_remove_view(app->view_dispatcher, 0);
    variable_item_list_free(app->variable_item_list);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    SAVE_CLOCK_SETTINGS(&app->clock_settings);
    free(app);
}

extern int32_t clock_settings_app(void* p) {
    UNUSED(p);
    ClockAppSettings* app = alloc_settings();
    view_dispatcher_run(app->view_dispatcher);
    free_settings(app);
    return 0;
}
