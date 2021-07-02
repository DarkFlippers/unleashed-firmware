#include "elements.h"
#include "gui/canvas.h"
#include <assets_icons.h>
#include <gui/icon_i.h>
#include <m-string.h>
#include <furi.h>
#include "canvas_i.h"
#include <string.h>
#include <stdint.h>

void elements_progress_bar(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t progress,
    uint8_t total) {
    furi_assert(canvas);
    furi_assert(total > 0);
    uint8_t height = 9;
    uint8_t marker_width = 7;
    furi_assert(width > marker_width);

    uint8_t progress_length = ((float)progress / total) * (width - marker_width - 2);

    // rframe doesnt work if (radius * 2) > any rect side, so write manually
    uint8_t x_max = x + width - 1;
    uint8_t y_max = y + height - 1;
    canvas_draw_line(canvas, x + 3, y, x_max - 3, y);
    canvas_draw_line(canvas, x_max - 3, y, x_max, y + 3);
    canvas_draw_line(canvas, x_max, y + 3, x_max, y_max - 3);
    canvas_draw_line(canvas, x_max, y_max - 3, x_max - 3, y_max);
    canvas_draw_line(canvas, x_max - 3, y_max, x + 3, y_max);
    canvas_draw_line(canvas, x + 3, y_max, x, y_max - 3);
    canvas_draw_line(canvas, x, y_max - 3, x, y + 3);
    canvas_draw_line(canvas, x, y + 3, x + 3, y);

    canvas_draw_rbox(canvas, x + 1, y + 1, marker_width + progress_length, height - 2, 3);
    canvas_invert_color(canvas);
    canvas_draw_dot(canvas, x + progress_length + 3, y + 2);
    canvas_draw_dot(canvas, x + progress_length + 4, y + 2);
    canvas_draw_dot(canvas, x + progress_length + 5, y + 3);
    canvas_draw_dot(canvas, x + progress_length + 6, y + 4);
    canvas_invert_color(canvas);
}

void elements_scrollbar_pos(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t height,
    uint16_t pos,
    uint16_t total) {
    furi_assert(canvas);
    // prevent overflows
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x - 3, y, 3, height);
    // dot line
    canvas_set_color(canvas, ColorBlack);
    for(uint8_t i = y; i < height + y; i += 2) {
        canvas_draw_dot(canvas, x - 2, i);
    }
    // Position block
    if(total) {
        float block_h = ((float)height) / total;
        canvas_draw_box(canvas, x - 3, y + (block_h * pos), 3, MAX(block_h, 1));
    }
}

void elements_scrollbar(Canvas* canvas, uint16_t pos, uint16_t total) {
    furi_assert(canvas);

    uint8_t width = canvas_width(canvas);
    uint8_t height = canvas_height(canvas);
    // prevent overflows
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, width - 3, 0, 3, height);
    // dot line
    canvas_set_color(canvas, ColorBlack);
    for(uint8_t i = 0; i < height; i += 2) {
        canvas_draw_dot(canvas, width - 2, i);
    }
    // Position block
    if(total) {
        float block_h = ((float)height) / total;
        canvas_draw_box(canvas, width - 3, block_h * pos, 3, MAX(block_h, 1));
    }
}

void elements_frame(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    furi_assert(canvas);

    canvas_draw_line(canvas, x + 2, y, x + width - 2, y);
    canvas_draw_line(canvas, x + 1, y + height - 1, x + width, y + height - 1);
    canvas_draw_line(canvas, x + 2, y + height, x + width - 1, y + height);

    canvas_draw_line(canvas, x, y + 2, x, y + height - 2);
    canvas_draw_line(canvas, x + width - 1, y + 1, x + width - 1, y + height - 2);
    canvas_draw_line(canvas, x + width, y + 2, x + width, y + height - 2);

    canvas_draw_dot(canvas, x + 1, y + 1);
}

void elements_button_left(Canvas* canvas, const char* str) {
    const uint8_t button_height = 13;
    const uint8_t vertical_offset = 3;
    const uint8_t horizontal_offset = 3;
    const uint8_t string_width = canvas_string_width(canvas, str);
    const IconData* icon = assets_icons_get_data(I_ButtonLeft_4x7);
    const uint8_t icon_offset = 3;
    const uint8_t icon_width_with_offset = icon->width + icon_offset;
    const uint8_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const uint8_t x = 0;
    const uint8_t y = canvas_height(canvas);

    canvas_draw_box(canvas, x, y - button_height, button_width, button_height);
    canvas_draw_line(canvas, x + button_width + 0, y, x + button_width + 0, y - button_height + 0);
    canvas_draw_line(canvas, x + button_width + 1, y, x + button_width + 1, y - button_height + 1);
    canvas_draw_line(canvas, x + button_width + 2, y, x + button_width + 2, y - button_height + 2);

    canvas_invert_color(canvas);
    canvas_draw_icon_name(
        canvas, x + horizontal_offset, y - button_height + vertical_offset, I_ButtonLeft_4x7);
    canvas_draw_str(
        canvas, x + horizontal_offset + icon_width_with_offset, y - vertical_offset, str);
    canvas_invert_color(canvas);
}

void elements_button_right(Canvas* canvas, const char* str) {
    const uint8_t button_height = 13;
    const uint8_t vertical_offset = 3;
    const uint8_t horizontal_offset = 3;
    const uint8_t string_width = canvas_string_width(canvas, str);
    const IconData* icon = assets_icons_get_data(I_ButtonRight_4x7);
    const uint8_t icon_offset = 3;
    const uint8_t icon_width_with_offset = icon->width + icon_offset;
    const uint8_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const uint8_t x = canvas_width(canvas);
    const uint8_t y = canvas_height(canvas);

    canvas_draw_box(canvas, x - button_width, y - button_height, button_width, button_height);
    canvas_draw_line(canvas, x - button_width - 1, y, x - button_width - 1, y - button_height + 0);
    canvas_draw_line(canvas, x - button_width - 2, y, x - button_width - 2, y - button_height + 1);
    canvas_draw_line(canvas, x - button_width - 3, y, x - button_width - 3, y - button_height + 2);

    canvas_invert_color(canvas);
    canvas_draw_str(canvas, x - button_width + horizontal_offset, y - vertical_offset, str);
    canvas_draw_icon_name(
        canvas,
        x - horizontal_offset - icon->width,
        y - button_height + vertical_offset,
        I_ButtonRight_4x7);
    canvas_invert_color(canvas);
}

void elements_button_center(Canvas* canvas, const char* str) {
    const uint8_t button_height = 13;
    const uint8_t vertical_offset = 3;
    const uint8_t horizontal_offset = 1;
    const uint8_t string_width = canvas_string_width(canvas, str);
    const IconData* icon = assets_icons_get_data(I_ButtonCenter_7x7);
    const uint8_t icon_offset = 3;
    const uint8_t icon_width_with_offset = icon->width + icon_offset;
    const uint8_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const uint8_t x = (canvas_width(canvas) - button_width) / 2;
    const uint8_t y = canvas_height(canvas);

    canvas_draw_box(canvas, x, y - button_height, button_width, button_height);

    canvas_draw_line(canvas, x - 1, y, x - 1, y - button_height + 0);
    canvas_draw_line(canvas, x - 2, y, x - 2, y - button_height + 1);
    canvas_draw_line(canvas, x - 3, y, x - 3, y - button_height + 2);

    canvas_draw_line(canvas, x + button_width + 0, y, x + button_width + 0, y - button_height + 0);
    canvas_draw_line(canvas, x + button_width + 1, y, x + button_width + 1, y - button_height + 1);
    canvas_draw_line(canvas, x + button_width + 2, y, x + button_width + 2, y - button_height + 2);

    canvas_invert_color(canvas);
    canvas_draw_icon_name(
        canvas, x + horizontal_offset, y - button_height + vertical_offset, I_ButtonCenter_7x7);
    canvas_draw_str(
        canvas, x + horizontal_offset + icon_width_with_offset, y - vertical_offset, str);
    canvas_invert_color(canvas);
}

void elements_multiline_text_aligned(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical,
    const char* text) {
    furi_assert(canvas);
    furi_assert(text);

    uint8_t font_height = canvas_current_font_height(canvas) + 2;
    string_t str;
    string_init(str);
    const char* start = text;
    char* end;

    // get lines count
    uint8_t i, lines_count;
    for(i = 0, lines_count = 0; text[i]; i++) lines_count += (text[i] == '\n');

    switch(vertical) {
    case AlignBottom:
        y -= font_height * lines_count;
        break;
    case AlignCenter:
        y -= (font_height * lines_count) / 2;
        break;
    case AlignTop:
    default:
        break;
    }

    do {
        end = strchr(start, '\n');

        if(end) {
            string_set_strn(str, start, end - start);
        } else {
            string_set_str(str, start);
        }

        uint16_t len_px = canvas_string_width(canvas, string_get_cstr(str));
        uint8_t px_left =
            canvas_width(canvas) - (x - (horizontal == AlignCenter ? len_px / 2 : 0));

        // hacky
        if(len_px > px_left) {
            string_t buff;
            string_init_set(buff, str);
            size_t s_len = string_size(str);
            uint8_t end_pos = s_len - ((len_px - px_left) / (len_px / s_len) + 5);

            string_left(buff, end_pos);
            string_cat(buff, "-");
            string_right(str, end_pos);

            canvas_draw_str_aligned(canvas, x, y, horizontal, vertical, string_get_cstr(buff));
            string_clear(buff);

            start = end + 1;
            y += font_height;
        }

        canvas_draw_str_aligned(canvas, x, y, horizontal, vertical, string_get_cstr(str));
        start = end + 1;
        y += font_height;
    } while(end);
    string_clear(str);
}

void elements_multiline_text(Canvas* canvas, uint8_t x, uint8_t y, const char* text) {
    furi_assert(canvas);
    furi_assert(text);

    uint8_t font_height = canvas_current_font_height(canvas);
    string_t str;
    string_init(str);
    const char* start = text;
    char* end;
    do {
        end = strchr(start, '\n');
        if(end) {
            string_set_strn(str, start, end - start);
        } else {
            string_set_str(str, start);
        }
        canvas_draw_str(canvas, x, y, string_get_cstr(str));
        start = end + 1;
        y += font_height;
    } while(end);
    string_clear(str);
}

void elements_multiline_text_framed(Canvas* canvas, uint8_t x, uint8_t y, const char* text) {
    furi_assert(canvas);
    furi_assert(text);

    uint8_t font_y = canvas_current_font_height(canvas);
    uint16_t str_width = canvas_string_width(canvas, text);
    // count \n's
    uint8_t lines = 1;
    const char* t = text;
    while(*t != '\0') {
        if(*t == '\n') {
            lines++;
            uint16_t temp_width = canvas_string_width(canvas, t + 1);
            str_width = temp_width > str_width ? temp_width : str_width;
        }
        t++;
    }

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x, y - font_y, str_width + 8, font_y * lines + 6);
    canvas_set_color(canvas, ColorBlack);
    elements_multiline_text(canvas, x + 4, y + 1, text);
    elements_frame(canvas, x, y - font_y, str_width + 8, font_y * lines + 6);
}

void elements_slightly_rounded_frame(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height) {
    furi_assert(canvas);
    canvas_draw_rframe(canvas, x, y, width, height, 1);
}

void elements_slightly_rounded_box(
    Canvas* canvas,
    uint8_t x,
    uint8_t y,
    uint8_t width,
    uint8_t height) {
    furi_assert(canvas);
    canvas_draw_rbox(canvas, x, y, width, height, 1);
}

void elements_string_fit_width(Canvas* canvas, string_t string, uint8_t width) {
    furi_assert(canvas);
    furi_assert(string);

    uint16_t len_px = canvas_string_width(canvas, string_get_cstr(string));

    if(len_px > width) {
        size_t s_len = string_size(string);
        uint8_t end_pos = s_len - ((len_px - width) / ((len_px / s_len) + 2) + 2);

        string_mid(string, 0, end_pos);
        string_cat(string, "...");
    }
}
