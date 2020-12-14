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

void irukagotchi_draw_callback(Canvas* canvas, void* context) {
    Irukagotchi* irukagotchi = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon(canvas, 128 - 80, 0, irukagotchi->icon);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 10, TARGET " " BUILD_DATE);
    canvas_draw_str(canvas, 2, 22, GIT_BRANCH);
    canvas_draw_str(canvas, 2, 34, GIT_BRANCH_NUM);
    canvas_draw_str(canvas, 2, 46, GIT_COMMIT);
}

void irukagotchi_input_callback(InputEvent* event, void* context) {
    Irukagotchi* irukagotchi = context;

    if(!event->state || event->input != InputOk) return;

    with_value_mutex(
        irukagotchi->menu_vm, (Menu * menu) { menu_ok(menu); });
}

Irukagotchi* irukagotchi_alloc() {
    Irukagotchi* irukagotchi = furi_alloc(sizeof(Irukagotchi));

    irukagotchi->icon = assets_icons_get(I_Flipper_young_80x60);
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

    Gui* gui = furi_open("gui");
    gui_add_widget(gui, irukagotchi->widget, GuiLayerNone);

    furiac_ready();

    while(1) {
        osDelay(osWaitForever);
    }
}