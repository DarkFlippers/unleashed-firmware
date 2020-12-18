#include "power.h"

#include <flipper_v2.h>

#include <menu/menu.h>
#include <menu/menu_item.h>
#include <gui/gui.h>
#include <gui/widget.h>
#include <assets_icons.h>
#include <api-hal-power.h>
#include <cli/cli.h>

struct Power {
    Icon* usb_icon;
    Widget* usb_widget;

    Icon* battery_icon;
    Widget* battery_widget;

    Widget* widget;

    ValueMutex* menu_vm;
    Cli* cli;
    MenuItem* menu;

    float current;
    float voltage;
    float temperature;

    uint8_t charge;
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
    canvas_draw_box(canvas, 2, 2, (float)power->charge / 100 * 14, 4);
}

void power_off_callback(void* context) {
    api_hal_power_off();
}

void power_enable_otg_callback(void* context) {
    api_hal_power_enable_otg();
}

void power_disable_otg_callback(void* context) {
    api_hal_power_disable_otg();
}

void power_info_callback(void* context) {
    Power* power = context;
    widget_enabled_set(power->widget, true);
}

void power_draw_callback(Canvas* canvas, void* context) {
    Power* power = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Power state:");

    char buffer[64];
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, 64, "Current: %ldmA", (int32_t)(power->current * 1000));
    canvas_draw_str(canvas, 5, 22, buffer);
    snprintf(buffer, 64, "Voltage: %ldmV", (uint32_t)(power->voltage * 1000));
    canvas_draw_str(canvas, 5, 32, buffer);
    snprintf(buffer, 64, "Charge: %ld%%", (uint32_t)(power->charge));
    canvas_draw_str(canvas, 5, 42, buffer);
    snprintf(buffer, 64, "Temperature: %ldC", (uint32_t)(power->temperature));
    canvas_draw_str(canvas, 5, 52, buffer);
}

void power_input_callback(InputEvent* event, void* context) {
    Power* power = context;

    if(!event->state) return;

    widget_enabled_set(power->widget, false);
}

Power* power_alloc() {
    Power* power = furi_alloc(sizeof(Power));

    power->menu_vm = furi_open("menu");
    furi_check(power->menu_vm);

    power->cli = furi_open("cli");

    power->menu = menu_item_alloc_menu("Power", NULL);
    menu_item_subitem_add(
        power->menu, menu_item_alloc_function("Poweroff", NULL, power_off_callback, power));
    menu_item_subitem_add(
        power->menu,
        menu_item_alloc_function("Enable OTG", NULL, power_enable_otg_callback, power));
    menu_item_subitem_add(
        power->menu,
        menu_item_alloc_function("Disable OTG", NULL, power_disable_otg_callback, power));
    menu_item_subitem_add(
        power->menu, menu_item_alloc_function("Info", NULL, power_info_callback, power));

    power->usb_icon = assets_icons_get(I_USBConnected_15x8);
    power->usb_widget = widget_alloc();
    widget_set_width(power->usb_widget, icon_get_width(power->usb_icon));
    widget_draw_callback_set(power->usb_widget, power_draw_usb_callback, power);

    power->widget = widget_alloc();
    widget_draw_callback_set(power->widget, power_draw_callback, power);
    widget_input_callback_set(power->widget, power_input_callback, power);
    widget_enabled_set(power->widget, false);

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

    Gui* gui = furi_open("gui");
    gui_add_widget(gui, power->widget, GuiLayerFullscreen);
    gui_add_widget(gui, power->usb_widget, GuiLayerStatusBarLeft);
    gui_add_widget(gui, power->battery_widget, GuiLayerStatusBarRight);

    with_value_mutex(
        power->menu_vm, (Menu * menu) { menu_item_add(menu, power->menu); });

    if(!furi_create("power", power)) {
        printf("[power_task] unable to create power record\n");
        furiac_exit(NULL);
    }

    api_hal_power_init();

    furiac_ready();

    while(1) {
        power->charge = api_hal_power_get_pct();
        power->current = api_hal_power_get_battery_current();
        power->voltage = api_hal_power_get_battery_voltage();
        power->temperature = api_hal_power_get_battery_temperature();
        widget_update(power->widget);
        widget_enabled_set(power->usb_widget, api_hal_power_is_charging());
        osDelay(1000);
    }
}
