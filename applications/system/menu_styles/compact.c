/**
 * @file compact.c
 * Compact main menu style.
 *
 * Ported from Momentum Firmware (GPL-3.0), originally by @MatthewKuKanich:
 * https://github.com/Next-Flip/Momentum-Firmware/commit/09986b33a7ff1912a1167f2427eddc8adfe82e6f
 */
#include "menu_style_helpers.h"

#define MENU_STYLE_COMPACT_ROWS 8 // Two columns of eight, so a page of sixteen

static void menu_style_compact_draw(Canvas* canvas, MenuModel* model) {
    size_t position = model->position;
    size_t count = model->count;

    canvas_set_font(canvas, FontBatteryPercent);
    for(size_t i = 0; i < 2; i++) {
        for(size_t j = 0; j < MENU_STYLE_COMPACT_ROWS; j++) {
            size_t index = i * MENU_STYLE_COMPACT_ROWS + j +
                           (position - (position % (MENU_STYLE_COMPACT_ROWS * 2)));
            if(index >= count) continue;
            int32_t y = 8 * j;
            int32_t x = 64 * i;
            bool selected = index == position;
            if(selected) {
                canvas_draw_box(canvas, x, y, 64, 8);
                canvas_set_color(canvas, ColorWhite);
            }
            elements_scrollable_text_line_str(
                canvas,
                x + 1,
                y + 7,
                62,
                menu_style_label(&model->items[index], true),
                menu_style_scroll(model, selected),
                false,
                false);
            if(selected) {
                canvas_set_color(canvas, ColorBlack);
            }
        }
    }
}

static size_t menu_style_compact_navigate(MenuModel* model, InputKey key) {
    switch(key) {
    case InputKeyLeft:
    case InputKeyRight:
        return menu_style_navigate_two_columns(model, MENU_STYLE_COMPACT_ROWS);
    default:
        return menu_style_navigate_list(model, key);
    }
}

static const MenuStyle menu_style_compact = {
    .draw = menu_style_compact_draw,
    .navigate = menu_style_compact_navigate,
};

MENU_STYLE_PLUGIN(menu_style_compact, menu_style_compact_ep)
