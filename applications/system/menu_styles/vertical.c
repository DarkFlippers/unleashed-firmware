#include "menu_style_helpers.h"
#include <furi_hal_rtc.h>

static void menu_style_vertical_draw(Canvas* canvas, MenuModel* model) {
    size_t position = model->position;
    size_t count = model->count;

    // Left rotated on purpose - see MenuStyle::draw in menu.h. Restoring it here would stream a
    // rotated image tagged horizontal, which qFlipper and the mobile app then draw sideways.
    canvas_set_orientation(canvas, CanvasOrientationVertical);

    size_t shift = model->offset;
    if(shift >= position || shift + 7 <= position) {
        int32_t max_shift = (int32_t)count - 8;
        if(max_shift < 0) max_shift = 0;
        int32_t centered = (int32_t)position - 4;
        if(centered < 0) centered = 0;
        if(centered > max_shift) centered = max_shift;
        shift = centered;
        model->offset = shift;
    }

    canvas_set_font(canvas, FontSecondary);
    for(size_t i = 0; i < 8; i++) {
        size_t item_i = shift + i;
        if(item_i >= count) continue;
        int32_t y = 16 * i;
        bool selected = item_i == position;
        if(selected) {
            elements_slightly_rounded_box(canvas, 0, y, 64, 16);
            canvas_set_color(canvas, ColorWhite);
        }
        const MenuItem* item = &model->items[item_i];
        menu_style_icon_centered(canvas, item->icon, 0, y, 16, 16);
        elements_scrollable_text_line_str(
            canvas,
            17,
            y + 12,
            46,
            menu_style_label(item, true),
            menu_style_scroll(model, selected),
            false,
            false);
        if(selected) {
            canvas_set_color(canvas, ColorBlack);
        }
    }
}

static size_t menu_style_vertical_navigate(MenuModel* model, InputKey key) {
    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagHandOrient)) {
        if(key == InputKeyLeft) {
            key = InputKeyRight;
        } else if(key == InputKeyRight) {
            key = InputKeyLeft;
        }
    }

    size_t position = model->position;
    size_t count = model->count;
    size_t max_offset = count > 8 ? count - 8 : 0;

    if(key == InputKeyLeft) {
        if(position > 0) {
            position--;
            if(model->offset && model->offset == position) model->offset--;
        } else {
            position = count - 1;
            model->offset = max_offset;
        }
    } else if(key == InputKeyRight) {
        if(position < count - 1) {
            position++;
            if(model->offset < max_offset && model->offset + 7 == position) model->offset++;
        } else {
            position = 0;
            model->offset = 0;
        }
    }
    return position;
}

static const MenuStyle menu_style_vertical = {
    .draw = menu_style_vertical_draw,
    .navigate = menu_style_vertical_navigate,
};

MENU_STYLE_PLUGIN(menu_style_vertical, menu_style_vertical_ep)
