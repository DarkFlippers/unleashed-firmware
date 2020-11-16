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

    ValueMutex* menu_vm;
    Cli* cli;
    MenuItem* menu;

    uint8_t charge;
};

void power_draw_usb_callback(CanvasApi* canvas, void* context) {
    assert(context);
    Power* power = context;
    canvas->draw_icon(canvas, 0, 0, power->usb_icon);
}

void power_draw_battery_callback(CanvasApi* canvas, void* context) {
    assert(context);
    Power* power = context;

    canvas->draw_icon(canvas, 0, 0, power->battery_icon);
    canvas->draw_box(canvas, 2, 2, (float)power->charge / 100 * 14, 4);
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
    cli_print("Poweroff in 5 seconds");
    osDelay(5000);
    api_hal_power_off();
}

void power_task(void* p) {
    (void)p;
    Power* power = power_alloc();

    if(power->cli) {
        cli_add_command(power->cli, "poweroff", power_cli_poweroff, power);
    }

    FuriRecordSubscriber* gui_record = furi_open_deprecated("gui", false, false, NULL, NULL, NULL);
    assert(gui_record);
    GuiApi* gui = furi_take(gui_record);
    assert(gui);
    gui->add_widget(gui, power->usb_widget, GuiLayerStatusBarLeft);
    gui->add_widget(gui, power->battery_widget, GuiLayerStatusBarRight);
    furi_commit(gui_record);

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
        widget_enabled_set(power->usb_widget, api_hal_power_is_charging());
        osDelay(1000);
    }
}
