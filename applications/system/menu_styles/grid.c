/**
 * @file grid.c
 * Grid main menu style.
 *
 * By @apfxtech.
 */
#include "menu_style_helpers.h"

#define MENU_STYLE_GRID_COLS 5
#define MENU_STYLE_GRID_ROWS 3
#define MENU_STYLE_GRID_PAGE (MENU_STYLE_GRID_COLS * MENU_STYLE_GRID_ROWS)

static void menu_style_grid_draw(Canvas* canvas, MenuModel* model) {
    size_t position = model->position;
    size_t count = model->count;
    size_t page_start = position - position % MENU_STYLE_GRID_PAGE;

    canvas_set_font(canvas, FontPrimary);
    elements_scrollable_text_line_str(
        canvas,
        2,
        9,
        92,
        menu_style_label(&model->items[position], false),
        menu_style_scroll(model, true),
        false,
        false);
    canvas_set_font(canvas, FontSecondary);
    char counter[16];
    snprintf(counter, sizeof(counter), "%zu/%zu", position + 1, count);
    canvas_draw_str_aligned(canvas, 126, 2, AlignRight, AlignTop, counter);
    canvas_draw_line(canvas, 0, 12, 127, 12);

    for(size_t i = 0; i < MENU_STYLE_GRID_PAGE; i++) {
        size_t item_i = page_start + i;
        if(item_i >= count) break;
        int32_t x = 2 + (i % MENU_STYLE_GRID_COLS) * 25;
        int32_t y = 14 + (i / MENU_STYLE_GRID_COLS) * 17;
        bool selected = item_i == position;
        if(selected) {
            canvas_draw_rbox(canvas, x, y, 24, 16, 2);
            canvas_set_color(canvas, ColorWhite);
        }
        menu_style_icon_centered(canvas, model->items[item_i].icon, x, y, 24, 16);
        if(selected) canvas_set_color(canvas, ColorBlack);
    }
}

static size_t menu_style_grid_navigate(MenuModel* model, InputKey key) {
    size_t position = model->position;
    size_t count = model->count;
    size_t page_start = position - position % MENU_STYLE_GRID_PAGE;
    size_t page_end = MIN(page_start + MENU_STYLE_GRID_PAGE, count);
    size_t column = (position - page_start) % MENU_STYLE_GRID_COLS;
    switch(key) {
    case InputKeyDown: {
        size_t next = position + MENU_STYLE_GRID_COLS;
        return next < page_end ? next : page_start + column;
    }
    case InputKeyUp: {
        if(position - page_start >= MENU_STYLE_GRID_COLS) return position - MENU_STYLE_GRID_COLS;
        size_t bottom = position;
        while(bottom + MENU_STYLE_GRID_COLS < page_end)
            bottom += MENU_STYLE_GRID_COLS;
        return bottom;
    }
    default:
        return menu_style_navigate_wrap(model, key);
    }
}

static const MenuStyle menu_style_grid = {
    .draw = menu_style_grid_draw,
    .navigate = menu_style_grid_navigate,
};

MENU_STYLE_PLUGIN(menu_style_grid, menu_style_grid_ep)
