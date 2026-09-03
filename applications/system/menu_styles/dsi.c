#include "menu_style_helpers.h"

static void menu_style_dsi_draw(Canvas* canvas, MenuModel* model) {
    size_t position = model->position;
    size_t count = model->count;

    for(int32_t i = -2; i <= 2; i++) {
        const MenuItem* item = &model->items[(position + count + i) % count];
        int32_t width = 24;
        int32_t height = 26;
        int32_t x = 64;
        int32_t y = 36;
        if(i == 0) {
            width += 6;
            height += 4;
            elements_bold_rounded_frame(canvas, x - width / 2, y - height / 2, width, height + 5);
            canvas_set_font(canvas, FontBatteryPercent);
            canvas_draw_str_aligned(
                canvas, x - 9, y + height / 2 + 1, AlignCenter, AlignBottom, "S");
            canvas_draw_str_aligned(
                canvas, x, y + height / 2 + 1, AlignCenter, AlignBottom, "TAR");
            canvas_draw_str_aligned(
                canvas, x + 9, y + height / 2 + 1, AlignCenter, AlignBottom, "T");

            canvas_draw_rframe(canvas, 0, 0, 128, 18, 3);
            canvas_draw_line(canvas, 60, 18, 64, 26);
            canvas_draw_line(canvas, 64, 26, 68, 18);
            canvas_set_color(canvas, ColorWhite);
            canvas_draw_line(canvas, 60, 17, 68, 17);
            canvas_draw_box(canvas, 62, 21, 5, 2);
            canvas_set_color(canvas, ColorBlack);

            canvas_set_font(canvas, FontPrimary);
            elements_scrollable_text_line_str(
                canvas,
                x,
                y - height / 2 - 8,
                124,
                menu_style_label(item, false),
                menu_style_scroll(model, true),
                false,
                true);
        } else {
            x += (width + 6) * i;
            y += 2;
            elements_slightly_rounded_frame(canvas, x - width / 2, y - height / 2, width, height);
        }
        menu_style_icon_centered(canvas, item->icon, x - 7, y - 7, 14, 14);
    }

    menu_style_scrollbar_horizontal(canvas, 0, 64, 128, position, count);
}

static size_t menu_style_dsi_navigate(MenuModel* model, InputKey key) {
    return menu_style_navigate_wrap(model, key);
}

static const MenuStyle menu_style_dsi = {
    .draw = menu_style_dsi_draw,
    .navigate = menu_style_dsi_navigate,
};

MENU_STYLE_PLUGIN(menu_style_dsi, menu_style_dsi_ep)
