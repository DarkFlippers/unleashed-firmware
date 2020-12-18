#include "dolphin_i.h"

void dolphin_draw_callback(Canvas* canvas, void* context) {
    Dolphin* dolphin = context;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    if(dolphin->screen == DolphinScreenIdle) {
        dolphin_draw_idle(canvas, dolphin);
    } else if(dolphin->screen == DolphinScreenDebug) {
        dolphin_draw_debug(canvas, dolphin);
    } else if(dolphin->screen == DolphinScreenStats) {
        dolphin_draw_stats(canvas, dolphin);
    }
}

void dolphin_draw_idle(Canvas* canvas, Dolphin* dolphin) {
    canvas_draw_icon(canvas, 128 - 80, 0, dolphin->icon);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 10, "/\\: Stats");
    canvas_draw_str(canvas, 5, 32, "OK: Menu");
    canvas_draw_str(canvas, 2, 52, "\\/: Version");
}

void dolphin_draw_debug(Canvas* canvas, Dolphin* dolphin) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Version info:");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 22, TARGET " " BUILD_DATE);
    canvas_draw_str(canvas, 5, 32, GIT_BRANCH);
    canvas_draw_str(canvas, 5, 42, GIT_BRANCH_NUM);
    canvas_draw_str(canvas, 5, 52, GIT_COMMIT);
}

void dolphin_draw_stats(Canvas* canvas, Dolphin* dolphin) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Dolphin stats:");

    char buffer[64];
    canvas_set_font(canvas, FontSecondary);
    snprintf(buffer, 64, "Icounter: %ld", dolphin_state_get_icounter(dolphin->state));
    canvas_draw_str(canvas, 5, 22, buffer);
    snprintf(buffer, 64, "Butthurt: %ld", dolphin_state_get_butthurt(dolphin->state));
    canvas_draw_str(canvas, 5, 32, buffer);
    canvas_draw_str(canvas, 5, 40, "< > change icounter");
}

void dolphin_input_callback(InputEvent* event, void* context) {
    Dolphin* dolphin = context;

    if(!event->state) return;

    if(event->input == InputOk) {
        with_value_mutex(
            dolphin->menu_vm, (Menu * menu) { menu_ok(menu); });
    } else if(event->input == InputUp) {
        if(dolphin->screen != DolphinScreenStats) {
            dolphin->screen++;
        }
    } else if(event->input == InputDown) {
        if(dolphin->screen != DolphinScreenDebug) {
            dolphin->screen--;
        }
    } else if(event->input == InputBack) {
        dolphin->screen = DolphinScreenIdle;
    } else if(event->input == InputLeft) {
        dolphin_deed(dolphin, DolphinDeedIButtonEmulate);
    } else if(event->input == InputRight) {
        dolphin_deed(dolphin, DolphinDeedWrong);
    }

    widget_update(dolphin->widget);
}

Dolphin* dolphin_alloc() {
    Dolphin* dolphin = furi_alloc(sizeof(Dolphin));

    dolphin->icon = assets_icons_get(I_Flipper_young_80x60);
    icon_start_animation(dolphin->icon);

    dolphin->widget = widget_alloc();
    widget_draw_callback_set(dolphin->widget, dolphin_draw_callback, dolphin);
    widget_input_callback_set(dolphin->widget, dolphin_input_callback, dolphin);

    dolphin->menu_vm = furi_open("menu");
    furi_check(dolphin->menu_vm);

    dolphin->state = dolphin_state_alloc();

    dolphin->screen = DolphinScreenIdle;

    dolphin->event_queue = osMessageQueueNew(8, sizeof(DolphinEvent), NULL);
    furi_check(dolphin->event_queue);
    return dolphin;
}

void dolphin_deed(Dolphin* dolphin, DolphinDeed deed) {
    DolphinEvent event;
    event.type = DolphinEventTypeDeed;
    event.deed = deed;
    furi_check(osMessageQueuePut(dolphin->event_queue, &event, 0, osWaitForever) == osOK);
}

void dolphin_task() {
    Dolphin* dolphin = dolphin_alloc();

    Gui* gui = furi_open("gui");
    gui_add_widget(gui, dolphin->widget, GuiLayerNone);

    if(!furi_create("dolphin", dolphin)) {
        printf("[dolphin_task] cannot create the dolphin record\n");
        furiac_exit(NULL);
    }

    furiac_ready();

    DolphinEvent event;
    while(1) {
        furi_check(osMessageQueueGet(dolphin->event_queue, &event, NULL, osWaitForever) == osOK);
        if(event.type == DolphinEventTypeDeed) {
            dolphin_state_on_deed(dolphin->state, event.deed);
        }
    }
}
