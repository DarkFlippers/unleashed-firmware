#include "menu_style_helpers.h"

static void menu_style_compact_draw(Canvas* canvas, MenuModel* model) {
    size_t position = model->position;
    size_t count = model->count;

    canvas_set_font(canvas, FontBatteryPercent);
    for(size_t i = 0; i < 2; i++) {
        for(size_t j = 0; j < 8; j++) {
            size_t index = i * 8 + j + (position - (position % 16));
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
    size_t position = model->position;
    switch(key) {
    case InputKeyLeft:
    case InputKeyRight:
        return (position % 16) < 8 ? position + 8 : position - 8;
    default:
        return menu_style_navigate_list(model, key);
    }
}

static const MenuStyle menu_style_compact = {
    .draw = menu_style_compact_draw,
    .navigate = menu_style_compact_navigate,
};

MENU_STYLE_PLUGIN(menu_style_compact, menu_style_compact_ep)
