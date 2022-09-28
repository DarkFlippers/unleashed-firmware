#include "../signal_gen_app_i.h"

typedef enum {
    LineIndexSource,
    LineIndexDivision,
} LineIndex;

static const char* const mco_source_names[] = {
    "32768",
    "64MHz",
    "~100K",
    "~200K",
    "~400K",
    "~800K",
    "~1MHz",
    "~2MHz",
    "~4MHz",
    "~8MHz",
    "~16MHz",
    "~24MHz",
    "~32MHz",
    "~48MHz",
};

static const FuriHalClockMcoSourceId mco_sources[] = {
    FuriHalClockMcoLse,
    FuriHalClockMcoSysclk,
    FuriHalClockMcoMsi100k,
    FuriHalClockMcoMsi200k,
    FuriHalClockMcoMsi400k,
    FuriHalClockMcoMsi800k,
    FuriHalClockMcoMsi1m,
    FuriHalClockMcoMsi2m,
    FuriHalClockMcoMsi4m,
    FuriHalClockMcoMsi8m,
    FuriHalClockMcoMsi16m,
    FuriHalClockMcoMsi24m,
    FuriHalClockMcoMsi32m,
    FuriHalClockMcoMsi48m,
};

static const char* const mco_divisor_names[] = {
    "1",
    "2",
    "4",
    "8",
    "16",
};

static const FuriHalClockMcoDivisorId mco_divisors[] = {
    FuriHalClockMcoDiv1,
    FuriHalClockMcoDiv2,
    FuriHalClockMcoDiv4,
    FuriHalClockMcoDiv8,
    FuriHalClockMcoDiv16,
};

static void mco_source_list_change_callback(VariableItem* item) {
    SignalGenApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, mco_source_names[index]);

    app->mco_src = mco_sources[index];

    view_dispatcher_send_custom_event(app->view_dispatcher, SignalGenMcoEventUpdate);
}

static void mco_divisor_list_change_callback(VariableItem* item) {
    SignalGenApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, mco_divisor_names[index]);

    app->mco_div = mco_divisors[index];

    view_dispatcher_send_custom_event(app->view_dispatcher, SignalGenMcoEventUpdate);
}

void signal_gen_scene_mco_on_enter(void* context) {
    SignalGenApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    VariableItem* item;

    item = variable_item_list_add(
        var_item_list, "Source", COUNT_OF(mco_source_names), mco_source_list_change_callback, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, mco_source_names[0]);

    item = variable_item_list_add(
        var_item_list,
        "Division",
        COUNT_OF(mco_divisor_names),
        mco_divisor_list_change_callback,
        app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, mco_divisor_names[0]);

    variable_item_list_set_selected_item(var_item_list, LineIndexSource);

    view_dispatcher_switch_to_view(app->view_dispatcher, SignalGenViewVarItemList);

    app->mco_src = FuriHalClockMcoLse;
    app->mco_div = FuriHalClockMcoDiv1;
    furi_hal_clock_mco_enable(app->mco_src, app->mco_div);
    furi_hal_gpio_init_ex(
        &gpio_usart_tx, GpioModeAltFunctionPushPull, GpioPullUp, GpioSpeedVeryHigh, GpioAltFn0MCO);
}

bool signal_gen_scene_mco_on_event(void* context, SceneManagerEvent event) {
    SignalGenApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SignalGenMcoEventUpdate) {
            consumed = true;
            furi_hal_clock_mco_enable(app->mco_src, app->mco_div);
        }
    }
    return consumed;
}

void signal_gen_scene_mco_on_exit(void* context) {
    SignalGenApp* app = context;
    variable_item_list_reset(app->var_item_list);
    furi_hal_gpio_init_ex(
        &gpio_usart_tx,
        GpioModeAltFunctionPushPull,
        GpioPullUp,
        GpioSpeedVeryHigh,
        GpioAltFn7USART1);
    furi_hal_clock_mco_disable();
}
