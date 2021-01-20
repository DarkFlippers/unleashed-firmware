#include "power.h"
#include "power_views.h"

#include <furi.h>

#include <menu/menu.h>
#include <menu/menu_item.h>

#include <gui/gui.h>
#include <gui/widget.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>

#include <assets_icons.h>
#include <cli/cli.h>
#include <stm32wbxx.h>

struct Power {
    ViewDispatcher* view_dispatcher;
    View* info_view;

    Icon* usb_icon;
    Widget* usb_widget;

    Icon* battery_icon;
    Widget* battery_widget;

    ValueMutex* menu_vm;
    Cli* cli;
    MenuItem* menu;
};

void power_draw_usb_callback(Canvas* canvas, void* context) {
    assert(context);
    Power* power = context;
    canvas_draw_icon(canvas, 0, 0, power->usb_icon);
}

void power_draw_battery_callback(Canvas* canvas, void* context) {
    assert(context);
    Power* power = context;

    canvas_draw_icon(canvas, 0, 0, power->battery_icon);
    with_view_model(
        power->info_view, (PowerInfoModel * model) {
            canvas_draw_box(canvas, 2, 2, (float)model->charge / 100 * 14, 4);
        });
}

uint32_t power_info_back_callback(void* context) {
    return VIEW_NONE;
}

void power_menu_off_callback(void* context) {
    api_hal_power_off();
}

void power_menu_reset_callback(void* context) {
    NVIC_SystemReset();
}

void power_menu_enable_otg_callback(void* context) {
    api_hal_power_enable_otg();
}

void power_menu_disable_otg_callback(void* context) {
    api_hal_power_disable_otg();
}

void power_menu_info_callback(void* context) {
    Power* power = context;
    view_dispatcher_switch_to_view(power->view_dispatcher, PowerViewInfo);
}

Power* power_alloc() {
    Power* power = furi_alloc(sizeof(Power));

    power->menu_vm = furi_record_open("menu");

    power->cli = furi_record_open("cli");

    power->menu = menu_item_alloc_menu("Power", NULL);
    menu_item_subitem_add(
        power->menu, menu_item_alloc_function("Off", NULL, power_menu_off_callback, power));
    menu_item_subitem_add(
        power->menu, menu_item_alloc_function("Reset", NULL, power_menu_reset_callback, power));
    menu_item_subitem_add(
        power->menu,
        menu_item_alloc_function("Enable OTG", NULL, power_menu_enable_otg_callback, power));
    menu_item_subitem_add(
        power->menu,
        menu_item_alloc_function("Disable OTG", NULL, power_menu_disable_otg_callback, power));
    menu_item_subitem_add(
        power->menu, menu_item_alloc_function("Info", NULL, power_menu_info_callback, power));

    power->view_dispatcher = view_dispatcher_alloc();
    power->info_view = view_alloc();
    view_allocate_model(power->info_view, ViewModelTypeLockFree, sizeof(PowerInfoModel));
    view_set_draw_callback(power->info_view, power_info_draw_callback);
    view_set_previous_callback(power->info_view, power_info_back_callback);
    view_dispatcher_add_view(power->view_dispatcher, PowerViewInfo, power->info_view);

    power->usb_icon = assets_icons_get(I_USBConnected_15x8);
    power->usb_widget = widget_alloc();
    widget_set_width(power->usb_widget, icon_get_width(power->usb_icon));
    widget_draw_callback_set(power->usb_widget, power_draw_usb_callback, power);

    power->battery_icon = assets_icons_get(I_Battery_19x8);
    power->battery_widget = widget_alloc();
    widget_set_width(power->battery_widget, icon_get_width(power->battery_icon));
    widget_draw_callback_set(power->battery_widget, power_draw_battery_callback, power);

    return power;
}

void power_free(Power* power) {
    assert(power);
    free(power);
}

void power_cli_poweroff(string_t args, void* context) {
    cli_print("Poweroff in 3 seconds");
    osDelay(3000);
    api_hal_power_off();
}

void power_cli_reset(string_t args, void* context) {
    cli_print("NVIC System Reset in 3 seconds");
    osDelay(3000);
    NVIC_SystemReset();
}

void power_cli_dfu(string_t args, void* context) {
    cli_print("NVIC System Reset to DFU mode in 3 seconds");
    api_hal_boot_set_mode(ApiHalBootModeDFU);
    osDelay(3000);
    NVIC_SystemReset();
}

void power_cli_test(string_t args, void* context) {
    string_t buffer;
    string_init(buffer);
    api_hal_power_dump_state(buffer);
    cli_print(string_get_cstr(buffer));
    string_clear(buffer);
}

void power_cli_otg_on(string_t args, void* context) {
    api_hal_power_enable_otg();
}

void power_cli_otg_off(string_t args, void* context) {
    api_hal_power_disable_otg();
}

void power_task(void* p) {
    (void)p;
    Power* power = power_alloc();

    if(power->cli) {
        cli_add_command(power->cli, "poweroff", power_cli_poweroff, power);
        cli_add_command(power->cli, "reset", power_cli_reset, power);
        cli_add_command(power->cli, "dfu", power_cli_dfu, power);
        cli_add_command(power->cli, "power_test", power_cli_test, power);
        cli_add_command(power->cli, "power_otg_on", power_cli_otg_on, power);
        cli_add_command(power->cli, "power_otg_off", power_cli_otg_off, power);
    }

    Gui* gui = furi_record_open("gui");
    gui_add_widget(gui, power->usb_widget, GuiLayerStatusBarLeft);
    gui_add_widget(gui, power->battery_widget, GuiLayerStatusBarRight);
    view_dispatcher_attach_to_gui(power->view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    with_value_mutex(
        power->menu_vm, (Menu * menu) { menu_item_add(menu, power->menu); });

    api_hal_power_init();

    furi_record_create("power", power);

    while(1) {
        with_view_model(
            power->info_view, (PowerInfoModel * model) {
                model->charge = api_hal_power_get_pct();
                model->capacity_remaining = api_hal_power_get_battery_remaining_capacity();
                model->capacity_full = api_hal_power_get_battery_full_capacity();
                model->current_charger = api_hal_power_get_battery_current(ApiHalPowerICCharger);
                model->current_gauge = api_hal_power_get_battery_current(ApiHalPowerICFuelGauge);
                model->voltage_charger = api_hal_power_get_battery_voltage(ApiHalPowerICCharger);
                model->voltage_gauge = api_hal_power_get_battery_voltage(ApiHalPowerICFuelGauge);
                model->temperature_charger =
                    api_hal_power_get_battery_temperature(ApiHalPowerICCharger);
                model->temperature_gauge =
                    api_hal_power_get_battery_temperature(ApiHalPowerICFuelGauge);
            });

        widget_update(power->battery_widget);
        widget_enabled_set(power->usb_widget, api_hal_power_is_charging());
        osDelay(1000);
    }
}
