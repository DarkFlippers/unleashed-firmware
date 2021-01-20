#include "bt_i.h"

Bt* bt_alloc() {
    Bt* bt = furi_alloc(sizeof(Bt));

    bt->cli = furi_record_open("cli");
    cli_add_command(bt->cli, "bt_info", bt_cli_info, bt);
    bt->gui = furi_record_open("gui");
    bt->menu = furi_record_open("menu");

    bt->statusbar_icon = assets_icons_get(I_Bluetooth_5x8);
    bt->statusbar_widget = widget_alloc();
    widget_set_width(bt->statusbar_widget, icon_get_width(bt->statusbar_icon));
    widget_draw_callback_set(bt->statusbar_widget, bt_draw_statusbar_callback, bt);
    widget_enabled_set(bt->statusbar_widget, false);
    gui_add_widget(bt->gui, bt->statusbar_widget, GuiLayerStatusBarLeft);

    bt->menu_icon = assets_icons_get(A_Bluetooth_14);
    bt->menu_item = menu_item_alloc_menu("Bluetooth", bt->menu_icon);
    with_value_mutex(
        bt->menu, (Menu * menu) { menu_item_add(menu, bt->menu_item); });

    return bt;
}

void bt_draw_statusbar_callback(Canvas* canvas, void* context) {
    assert(context);
    Bt* bt = context;
    canvas_draw_icon(canvas, 0, 0, bt->statusbar_icon);
}

void bt_cli_info(string_t args, void* context) {
    string_t buffer;
    string_init(buffer);
    api_hal_bt_dump_state(buffer);
    cli_print(string_get_cstr(buffer));
    string_clear(buffer);
}

void bt_task() {
    Bt* bt = bt_alloc();

    furi_record_create("bt", bt);

    api_hal_bt_init();

    while(1) {
        widget_enabled_set(bt->statusbar_widget, api_hal_bt_is_alive());
        osDelay(1000);
    }
}
