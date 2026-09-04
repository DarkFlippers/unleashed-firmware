#include "menu_style_helpers.h"
#include <dolphin/dolphin.h>
#include <furi_hal_version.h>

// dolphin_stats() is a blocking round trip to another service, and this runs in the draw
// callback, on the GUI thread, under the model lock - cache it so it cannot stall every frame
static uint8_t menu_style_ps4_level(void) {
    static uint32_t next_refresh;
    static uint8_t level;
    static bool known;
    if(!known || (int32_t)(furi_get_tick() - next_refresh) >= 0) {
        Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
        level = dolphin_stats(dolphin).level;
        furi_record_close(RECORD_DOLPHIN);
        next_refresh = furi_get_tick() + furi_ms_to_ticks(5000);
        known = true;
    }
    return level;
}

static void menu_style_ps4_draw(Canvas* canvas, MenuModel* model) {
    size_t position = model->position;
    size_t count = model->count;

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 1, 1, AlignLeft, AlignTop, furi_hal_version_get_name_ptr());
    char level[12];
    snprintf(level, sizeof(level), "Level %u", menu_style_ps4_level());
    canvas_draw_str_aligned(canvas, 127, 1, AlignRight, AlignTop, level);

    for(int32_t i = -1; i <= 4; i++) {
        size_t item_i = position + i;
        if(item_i >= count) continue;
        const MenuItem* item = &model->items[item_i];
        int32_t width = 20;
        int32_t height = 20;
        int32_t x = 36;
        int32_t y = 27;
        if(i == 0) {
            width += 10;
            height += 10;
            y += 2;
            canvas_draw_box(canvas, x - width / 2, y + height / 2, width, 9);
            canvas_set_color(canvas, ColorWhite);
            canvas_set_font(canvas, FontBatteryPercent);
            canvas_draw_str_aligned(canvas, x, y + height / 2 + 1, AlignCenter, AlignTop, "Start");

            canvas_set_color(canvas, ColorBlack);
            canvas_set_font(canvas, FontSecondary);
            elements_scrollable_text_line_str(
                canvas,
                x + width / 2 + 2,
                y + height / 2 + 7,
                74,
                menu_style_label(item, true),
                menu_style_scroll(model, true),
                false,
                false);
        } else {
            x += (width + 1) * i + (i < 0 ? -6 : 6);
        }
        canvas_draw_frame(canvas, x - width / 2, y - height / 2, width, height);
        menu_style_icon_centered(canvas, item->icon, x - 7, y - 7, 14, 14);
    }

    menu_style_scrollbar_horizontal(canvas, 0, 64, 128, position, count);
}

static size_t menu_style_ps4_navigate(MenuModel* model, InputKey key) {
    return menu_style_navigate_wrap(model, key);
}

static const MenuStyle menu_style_ps4 = {
    .draw = menu_style_ps4_draw,
    .navigate = menu_style_ps4_navigate,
};

MENU_STYLE_PLUGIN(menu_style_ps4, menu_style_ps4_ep)
