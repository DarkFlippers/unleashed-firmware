#include "system_settings.h"
#include <loader/loader.h>
#include <lib/toolbox/value_index.h>
#include <locale/locale.h>

const char* const log_level_text[] = {
    "Default",
    "None",
    "Error",
    "Warning",
    "Info",
    "Debug",
    "Trace",
};

const uint32_t log_level_value[] = {
    FuriLogLevelDefault,
    FuriLogLevelNone,
    FuriLogLevelError,
    FuriLogLevelWarn,
    FuriLogLevelInfo,
    FuriLogLevelDebug,
    FuriLogLevelTrace,
};

static void log_level_changed(VariableItem* item) {
    // SystemSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, log_level_text[index]);
    furi_hal_rtc_set_log_level(log_level_value[index]);
}

const char* const debug_text[] = {
    "OFF",
    "ON",
};

static void debug_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, debug_text[index]);
    if(index) {
        furi_hal_rtc_set_flag(FuriHalRtcFlagDebug);
    } else {
        furi_hal_rtc_reset_flag(FuriHalRtcFlagDebug);
    }
    loader_update_menu();
}

const char* const heap_trace_mode_text[] = {
    "None",
    "Main",
#if FURI_DEBUG
    "Tree",
    "All",
#endif
};

const uint32_t heap_trace_mode_value[] = {
    FuriHalRtcHeapTrackModeNone,
    FuriHalRtcHeapTrackModeMain,
#if FURI_DEBUG
    FuriHalRtcHeapTrackModeTree,
    FuriHalRtcHeapTrackModeAll,
#endif
};

static void heap_trace_mode_changed(VariableItem* item) {
    // SystemSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, heap_trace_mode_text[index]);
    furi_hal_rtc_set_heap_track_mode(heap_trace_mode_value[index]);
}

const char* const mesurement_units_text[] = {
    "Metric",
    "Imperial",
};

const uint32_t mesurement_units_value[] = {
    LocaleMeasurementUnitsMetric,
    LocaleMeasurementUnitsImperial,
};

static void mesurement_units_changed(VariableItem* item) {
    // SystemSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, mesurement_units_text[index]);
    locale_set_measurement_unit(mesurement_units_value[index]);
}

const char* const time_format_text[] = {
    "24h",
    "12h",
};

const uint32_t time_format_value[] = {
    LocaleTimeFormat24h,
    LocaleTimeFormat12h,
};

static void time_format_changed(VariableItem* item) {
    // SystemSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, time_format_text[index]);
    locale_set_time_format(time_format_value[index]);
}

const char* const date_format_text[] = {
    "D/M/Y",
    "M/D/Y",
    "Y/M/D",
};

const uint32_t date_format_value[] = {
    LocaleDateFormatDMY,
    LocaleDateFormatMDY,
    LocaleDateFormatYMD,
};

static void date_format_changed(VariableItem* item) {
    // SystemSettings* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, date_format_text[index]);
    locale_set_date_format(date_format_value[index]);
}

const char* const hand_mode[] = {
    "Righty",
    "Lefty",
};

static void hand_orient_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, hand_mode[index]);
    if(index) {
        furi_hal_rtc_set_flag(FuriHalRtcFlagHandOrient);
    } else {
        furi_hal_rtc_reset_flag(FuriHalRtcFlagHandOrient);
    }

    loader_update_menu();
}

static uint32_t system_settings_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

SystemSettings* system_settings_alloc() {
    SystemSettings* app = malloc(sizeof(SystemSettings));

    // Load settings
    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    VariableItem* item;
    uint8_t value_index;
    app->var_item_list = variable_item_list_alloc();

    item = variable_item_list_add(
        app->var_item_list, "Hand Orient", COUNT_OF(hand_mode), hand_orient_changed, app);
    value_index = furi_hal_rtc_is_flag_set(FuriHalRtcFlagHandOrient) ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, hand_mode[value_index]);

    item = variable_item_list_add(
        app->var_item_list,
        "Units",
        COUNT_OF(mesurement_units_text),
        mesurement_units_changed,
        app);
    value_index = value_index_uint32(
        locale_get_measurement_unit(), mesurement_units_value, COUNT_OF(mesurement_units_value));
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, mesurement_units_text[value_index]);

    item = variable_item_list_add(
        app->var_item_list, "Time Format", COUNT_OF(time_format_text), time_format_changed, app);
    value_index = value_index_uint32(
        locale_get_time_format(), time_format_value, COUNT_OF(time_format_value));
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, time_format_text[value_index]);

    item = variable_item_list_add(
        app->var_item_list, "Date Format", COUNT_OF(date_format_text), date_format_changed, app);
    value_index = value_index_uint32(
        locale_get_date_format(), date_format_value, COUNT_OF(date_format_value));
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, date_format_text[value_index]);

    item = variable_item_list_add(
        app->var_item_list, "Log Level", COUNT_OF(log_level_text), log_level_changed, app);
    value_index = value_index_uint32(
        furi_hal_rtc_get_log_level(), log_level_value, COUNT_OF(log_level_text));
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, log_level_text[value_index]);

    item = variable_item_list_add(
        app->var_item_list, "Debug", COUNT_OF(debug_text), debug_changed, app);
    value_index = furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug) ? 1 : 0;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, debug_text[value_index]);

    item = variable_item_list_add(
        app->var_item_list,
        "Heap Trace",
        COUNT_OF(heap_trace_mode_text),
        heap_trace_mode_changed,
        app);
    value_index = value_index_uint32(
        furi_hal_rtc_get_heap_track_mode(), heap_trace_mode_value, COUNT_OF(heap_trace_mode_text));
    furi_hal_rtc_set_heap_track_mode(heap_trace_mode_value[value_index]);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, heap_trace_mode_text[value_index]);

    view_set_previous_callback(
        variable_item_list_get_view(app->var_item_list), system_settings_exit);
    view_dispatcher_add_view(
        app->view_dispatcher,
        SystemSettingsViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    view_dispatcher_switch_to_view(app->view_dispatcher, SystemSettingsViewVarItemList);

    return app;
}

void system_settings_free(SystemSettings* app) {
    furi_assert(app);
    // Variable item list
    view_dispatcher_remove_view(app->view_dispatcher, SystemSettingsViewVarItemList);
    variable_item_list_free(app->var_item_list);
    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    // Records
    furi_record_close(RECORD_GUI);
    free(app);
}

int32_t system_settings_app(void* p) {
    UNUSED(p);
    SystemSettings* app = system_settings_alloc();
    view_dispatcher_run(app->view_dispatcher);
    system_settings_free(app);
    return 0;
}
