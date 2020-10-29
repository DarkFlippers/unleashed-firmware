#include "irukagotchi.h"

#include <flipper_v2.h>

#include <gui/gui.h>
#include <gui/widget.h>
#include <gui/canvas.h>
#include <menu/menu.h>

#include <assets_icons.h>

struct Irukagotchi {
    Icon* icon;
    Widget* widget;
    ValueMutex* menu_vm;
};

void irukagotchi_draw_callback(CanvasApi* canvas, void* context) {
    Irukagotchi* irukagotchi = context;

    canvas->clear(canvas);
    canvas->set_color(canvas, ColorBlack);
    canvas->set_font(canvas, FontPrimary);
    canvas->draw_icon(canvas, 10, 20, irukagotchi->icon);
    canvas->draw_str(canvas, 30, 32, "Irukagotchi");
}

void irukagotchi_input_callback(InputEvent* event, void* context) {
    Irukagotchi* irukagotchi = context;

    if(!event->state || event->input != InputOk) return;

    with_value_mutex(
        irukagotchi->menu_vm, (Menu * menu) { menu_ok(menu); });
}

Irukagotchi* irukagotchi_alloc() {
    Irukagotchi* irukagotchi = furi_alloc(sizeof(Irukagotchi));

    irukagotchi->icon = assets_icons_get(A_Tamagotchi_14);
    icon_start_animation(irukagotchi->icon);

    irukagotchi->widget = widget_alloc();
    widget_draw_callback_set(irukagotchi->widget, irukagotchi_draw_callback, irukagotchi);
    widget_input_callback_set(irukagotchi->widget, irukagotchi_input_callback, irukagotchi);

    irukagotchi->menu_vm = furi_open("menu");
    furi_check(irukagotchi->menu_vm);

    return irukagotchi;
}

void irukagotchi_task() {
    Irukagotchi* irukagotchi = irukagotchi_alloc();

    FuriRecordSubscriber* gui_record = furi_open_deprecated("gui", false, false, NULL, NULL, NULL);
    furi_check(gui_record);
    GuiApi* gui = furi_take(gui_record);
    furi_check(gui);
    gui->add_widget(gui, irukagotchi->widget, GuiLayerNone);
    furi_commit(gui_record);

    furiac_ready();

    while(1) {
        osDelay(osWaitForever);
    }
}