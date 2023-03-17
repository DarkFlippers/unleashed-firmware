#include "menu.h"

void add_menu(Menu* menu, const char* name, void (*callback)(void*)) {
    MenuItem* items = menu->items;

    menu->items = malloc(sizeof(MenuItem) * (menu->menu_count + 1));
    for(uint8_t i = 0; i < menu->menu_count; i++) {
        menu->items[i] = items[i];
    }
    free(items);

    menu->items[menu->menu_count] = (MenuItem){name, true, callback};
    menu->menu_count++;
}

void free_menu(Menu* menu) {
    free(menu->items);
    free(menu);
}

void set_menu_state(Menu* menu, uint8_t index, bool state) {
    if(menu->menu_count > index) {
        menu->items[index].enabled = state;
    }
    if(!state && menu->current_menu == index) move_menu(menu, 1);
}

void move_menu(Menu* menu, int8_t direction) {
    if(!menu->enabled) return;
    int max = menu->menu_count;
    for(int8_t i = 0; i < max; i++) {
        FURI_LOG_D(
            "MENU",
            "Iteration %i, current %i, direction %i, state %i",
            i,
            menu->current_menu,
            direction,
            menu->items[menu->current_menu].enabled ? 1 : 0);
        if(direction < 0 && menu->current_menu == 0) {
            menu->current_menu = menu->menu_count - 1;
        } else {
            menu->current_menu = (menu->current_menu + direction) % menu->menu_count;
        }
        FURI_LOG_D(
            "MENU",
            "After process current %i, direction %i, state %i",
            menu->current_menu,
            direction,
            menu->items[menu->current_menu].enabled ? 1 : 0);
        if(menu->items[menu->current_menu].enabled) {
            FURI_LOG_D("MENU", "Next menu %i", menu->current_menu);
            return;
        }
    }
    FURI_LOG_D("MENU", "Not found, setting false");
    menu->enabled = false;
}

void activate_menu(Menu* menu, void* state) {
    if(!menu->enabled) return;
    menu->items[menu->current_menu].callback(state);
}

void render_menu(Menu* menu, Canvas* canvas, uint8_t pos_x, uint8_t pos_y) {
    if(!menu->enabled) return;
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_rbox(canvas, pos_x, pos_y, menu->menu_width + 2, 10, 2);

    uint8_t w = pos_x + menu->menu_width;
    uint8_t h = pos_y + 10;
    uint8_t p1x = pos_x + 2;
    uint8_t p2x = pos_x + menu->menu_width - 2;
    uint8_t p1y = pos_y + 2;
    uint8_t p2y = pos_y + 8;

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, p1x, pos_y, p2x, pos_y);
    canvas_draw_line(canvas, p1x, h, p2x, h);
    canvas_draw_line(canvas, pos_x, p1y, pos_x, p2y);
    canvas_draw_line(canvas, w, p1y, w, p2y);
    canvas_draw_dot(canvas, pos_x + 1, pos_y + 1);
    canvas_draw_dot(canvas, w - 1, pos_y + 1);
    canvas_draw_dot(canvas, w - 1, h - 1);
    canvas_draw_dot(canvas, pos_x + 1, h - 1);

    //    canvas_draw_rbox(canvas, pos_x, pos_y, menu->menu_width + 2, 10, 2);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(
        canvas,
        pos_x + menu->menu_width / 2,
        pos_y + 6,
        AlignCenter,
        AlignCenter,
        menu->items[menu->current_menu].name);
    //9*5
    int center = pos_x + menu->menu_width / 2;
    for(uint8_t i = 0; i < 4; i++) {
        for(int8_t j = -i; j <= i; j++) {
            canvas_draw_dot(canvas, center + j, pos_y - 4 + i);
            canvas_draw_dot(canvas, center + j, pos_y + 14 - i);
        }
    }
}