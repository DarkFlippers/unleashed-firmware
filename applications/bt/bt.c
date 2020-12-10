#include "bt_i.h"

Bt* bt_alloc() {
    Bt* bt = furi_alloc(sizeof(Bt));
    bt->cli = furi_open("cli");

    bt->statusbar_icon = assets_icons_get(I_Bluetooth_5x8);
    bt->statusbar_widget = widget_alloc();
    widget_set_width(bt->statusbar_widget, icon_get_width(bt->statusbar_icon) + 2);
    widget_draw_callback_set(bt->statusbar_widget, bt_draw_statusbar_callback, bt);
    widget_enabled_set(bt->statusbar_widget, false);

    return bt;
}

void bt_draw_statusbar_callback(CanvasApi* canvas, void* context) {
    assert(context);
    Bt* bt = context;
    canvas->draw_icon(canvas, 0, 0, bt->statusbar_icon);
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

    if(bt->cli) {
        cli_add_command(bt->cli, "bt_info", bt_cli_info, bt);
    }

    // TODO: add ValueMutex(bt) to "bt" record
    if(!furi_create("bt", bt)) {
        printf("[bt_task] unable to create bt record\n");
        furiac_exit(NULL);
    }

    FuriRecordSubscriber* gui_record = furi_open_deprecated("gui", false, false, NULL, NULL, NULL);
    furi_assert(gui_record);
    GuiApi* gui = furi_take(gui_record);
    furi_assert(gui);
    gui->add_widget(gui, bt->statusbar_widget, GuiLayerStatusBarLeft);
    furi_commit(gui_record);

    furiac_ready();

    api_hal_bt_init();

    while(1) {
        widget_enabled_set(bt->statusbar_widget, api_hal_bt_is_alive());
        osDelay(1000);
    }
}
