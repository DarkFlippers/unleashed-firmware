#include "menu_style_helpers.h"

static const uint8_t menu_style_coverflow_lines[][4] = {
    {5, 36, 1, 37},   {4, 9, 1, 8},       {6, 41, 17, 36},   {19, 41, 30, 36},   {32, 41, 43, 36},
    {6, 4, 17, 9},    {19, 4, 30, 9},     {32, 4, 43, 9},    {5, 5, 5, 40},      {18, 5, 18, 40},
    {31, 5, 31, 40},  {95, 41, 84, 36},   {108, 41, 97, 36}, {121, 41, 110, 36}, {84, 9, 95, 4},
    {97, 9, 108, 4},  {110, 9, 121, 4},   {96, 5, 96, 40},   {109, 5, 109, 40},  {122, 5, 122, 40},
    {123, 9, 126, 8}, {123, 36, 126, 37},
};

// canvas_set_orientation() only changes how u8g2 transforms draw calls, never the tile buffer, so
// the buffer is always device-native and a flipped canvas has to be read back flipped. Only the
// two horizontal orientations can reach here: nothing sets a vertical orientation on the menu's
// View, and the hand orientation flag only swaps Horizontal for HorizontalFlip.
static bool menu_style_coverflow_pixel(const uint8_t* buffer, bool flipped, int32_t x, int32_t y) {
    if(flipped) {
        x = 127 - x;
        y = 63 - y;
    }
    return buffer[(y >> 3) * 128 + x] & (1 << (y & 7));
}

// There is no scaled icon draw in this firmware, so the half-width icons are made by hand, via a
// scratch area at the bottom left. That area is clear of everything drawn before it, and is
// erased again before the label and the scrollbar are drawn over it. Assumes an icon of at most
// 20x20: taller than that and the scratch runs up into the frame lines at y=41, which the
// read-back would then pick up as icon pixels.
static void menu_style_coverflow_draw_icon_narrow(
    Canvas* canvas,
    IconAnimation* icon,
    int32_t x,
    int32_t y) {
    int32_t width = icon_animation_get_width(icon);
    int32_t height = icon_animation_get_height(icon);
    int32_t scratch_x = 2;
    int32_t scratch_y = 62 - height;
    bool flipped = canvas_get_orientation(canvas) == CanvasOrientationHorizontalFlip;
    const uint8_t* buffer = canvas_get_buffer(canvas);

    canvas_draw_icon_animation(canvas, scratch_x, scratch_y, icon);
    for(int32_t py = 0; py < height; py++) {
        for(int32_t px = 0; px < width; px += 2) {
            bool set = menu_style_coverflow_pixel(buffer, flipped, scratch_x + px, scratch_y + py);
            if(px + 1 < width) {
                set |= menu_style_coverflow_pixel(
                    buffer, flipped, scratch_x + px + 1, scratch_y + py);
            }
            if(set) canvas_draw_dot(canvas, x + px / 2, y + py);
        }
    }
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, scratch_x, scratch_y, width, height);
    canvas_set_color(canvas, ColorBlack);
}

static void menu_style_coverflow_draw(Canvas* canvas, MenuModel* model) {
    size_t position = model->position;
    size_t count = model->count;

    canvas_set_font(canvas, FontPrimary);
    canvas_set_bitmap_mode(canvas, true);
    canvas_draw_frame(canvas, 0, 0, 128, 64);
    canvas_draw_rframe(canvas, 45, 4, 38, 38, 3);
    for(size_t i = 0; i < COUNT_OF(menu_style_coverflow_lines); i++) {
        const uint8_t* l = menu_style_coverflow_lines[i];
        canvas_draw_line(canvas, l[0], l[1], l[2], l[3]);
    }

    for(int32_t i = -3; i <= 3; i++) {
        const MenuItem* item = &model->items[(position + count + i) % count];
        int32_t x;
        if(i < 0) {
            x = 48 + 13 * i;
        } else if(i > 0) {
            x = 68 + 13 * i;
        } else {
            x = 54;
        }
        int32_t icon_x = x + (20 - icon_animation_get_width(item->icon)) / 2;
        int32_t icon_y = 13 + (20 - icon_animation_get_height(item->icon)) / 2;
        if(i) {
            menu_style_coverflow_draw_icon_narrow(canvas, item->icon, icon_x, icon_y);
        } else {
            canvas_draw_icon_animation(canvas, icon_x, icon_y, item->icon);
        }
    }

    elements_scrollable_text_line_str(
        canvas,
        64,
        54,
        124,
        menu_style_label(&model->items[position], false),
        menu_style_scroll(model, true),
        false,
        true);
    menu_style_scrollbar_horizontal(canvas, 0, 60, 128, position, count);
    canvas_set_bitmap_mode(canvas, false);
}

static size_t menu_style_coverflow_navigate(MenuModel* model, InputKey key) {
    return menu_style_navigate_wrap(model, key);
}

static const MenuStyle menu_style_coverflow = {
    .draw = menu_style_coverflow_draw,
    .navigate = menu_style_coverflow_navigate,
};

MENU_STYLE_PLUGIN(menu_style_coverflow, menu_style_coverflow_ep)
