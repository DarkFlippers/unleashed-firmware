#include "menu_style_helpers.h"

static void menu_style_wii_draw(Canvas* canvas, MenuModel* model) {
    size_t position = model->position;
    size_t count = model->count;
    size_t shift = 0;
    if(count > 6 && position >= 4) {
        shift = position - (position % 2) - ((position >= count - 2 + (count % 2)) ? 4 : 2);
    }

    canvas_set_font(canvas, FontSecondary);
    for(size_t i = 0; i < 6; i++) {
        size_t item_i = shift + i;
        if(item_i >= count) continue;
        int32_t x = (i / 2) * 43 + 1;
        int32_t y = (i % 2) * 32;
        bool selected = item_i == position;
        if(selected) {
            elements_slightly_rounded_box(canvas, x, y, 40, 30);
            canvas_set_color(canvas, ColorWhite);
        }
        const MenuItem* item = &model->items[item_i];
        menu_style_icon_centered(canvas, item->icon, x, y, 40, 20);
        elements_scrollable_text_line_str(
            canvas,
            x + 20,
            y + 26,
            36,
            menu_style_label(item, true),
            menu_style_scroll(model, selected),
            false,
            true);
        if(selected) {
            canvas_set_color(canvas, ColorBlack);
        } else {
            elements_frame(canvas, x, y, 40, 30);
        }
    }
}

static size_t menu_style_wii_navigate(MenuModel* model, InputKey key) {
    size_t position = model->position;
    size_t count = model->count;
    switch(key) {
    case InputKeyUp:
    case InputKeyDown:
        if(position % 2 || (position == count - 1 && count % 2)) {
            position--;
        } else {
            position++;
        }
        break;
    case InputKeyLeft:
        if(position < 2) {
            position = (count % 2) ? count - 1 : count - 2 + position % 2;
        } else {
            position -= 2;
        }
        break;
    case InputKeyRight:
        if(count % 2) {
            if(position == count - 1) {
                position = 0;
            } else if(position == count - 2) {
                position = count - 1;
            } else {
                position += 2;
            }
        } else {
            position += 2;
            if(position >= count) position %= 2;
        }
        break;
    default:
        break;
    }
    return position;
}

static const MenuStyle menu_style_wii = {
    .draw = menu_style_wii_draw,
    .navigate = menu_style_wii_navigate,
};

MENU_STYLE_PLUGIN(menu_style_wii, menu_style_wii_ep)
