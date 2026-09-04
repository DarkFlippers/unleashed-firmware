#include "menu_style_helpers.h"

#define MENU_STYLE_C64_ROWS 5 // Two columns of five, so a page of ten

static void menu_style_c64_draw(Canvas* canvas, MenuModel* model) {
    size_t position = model->position;
    size_t count = model->count;
    char line[48];

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 0, AlignCenter, AlignTop, "* FLIPPADORE 64 BASIC *");
    snprintf(line, sizeof(line), "%zu BASIC BYTES FREE", memmgr_get_free_heap());
    canvas_draw_str_aligned(canvas, 64, 9, AlignCenter, AlignTop, line);

    canvas_set_font(canvas, FontKeyboard);
    for(size_t i = 0; i < 2; i++) {
        for(size_t j = 0; j < MENU_STYLE_C64_ROWS; j++) {
            size_t index =
                i * MENU_STYLE_C64_ROWS + j + (position - (position % (MENU_STYLE_C64_ROWS * 2)));
            if(index >= count) continue;
            int32_t y = 9 * j + 13;
            int32_t x = 64 * i;
            bool selected = index == position;
            if(selected) {
                canvas_draw_box(canvas, x, y + 4, 64, 9);
                canvas_set_color(canvas, ColorWhite);
            }
            snprintf(
                line, sizeof(line), "%zu.%s", index, menu_style_label(&model->items[index], true));
            elements_scrollable_text_line_str(
                canvas, x + 2, y + 12, 60, line, menu_style_scroll(model, selected), false, false);
            if(selected) {
                canvas_set_color(canvas, ColorBlack);
            }
        }
    }
}

static size_t menu_style_c64_navigate(MenuModel* model, InputKey key) {
    switch(key) {
    case InputKeyLeft:
    case InputKeyRight:
        return menu_style_navigate_two_columns(model, MENU_STYLE_C64_ROWS);
    default:
        return menu_style_navigate_list(model, key);
    }
}

static const MenuStyle menu_style_c64 = {
    .draw = menu_style_c64_draw,
    .navigate = menu_style_c64_navigate,
};

MENU_STYLE_PLUGIN(menu_style_c64, menu_style_c64_ep)
