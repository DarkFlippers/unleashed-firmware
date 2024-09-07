#include "elements.h"
#include <m-core.h>
#include <assets_icons.h>
#include <furi_hal_resources.h>
#include <furi_hal.h>

#include <gui/canvas.h>
#include <gui/icon_i.h>
#include <gui/icon_animation_i.h>

#include <furi.h>

#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int32_t x;
    int32_t y;
    int32_t leading_min;
    int32_t leading_default;
    size_t height;
    size_t descender;
    size_t len;
    const char* text;
} ElementTextBoxLine;

void elements_progress_bar(Canvas* canvas, int32_t x, int32_t y, size_t width, float progress) {
    furi_check(canvas);
    furi_check((progress >= 0.0f) && (progress <= 1.0f));
    size_t height = 9;

    float progress_width = roundf(progress * (width - 2));

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x + 1, y + 1, width - 2, height - 2);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_rframe(canvas, x, y, width, height, 3);

    canvas_draw_box(canvas, x + 1, y + 1, progress_width, height - 2);
}

void elements_progress_bar_with_text(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    float progress,
    const char* text) {
    furi_check(canvas);
    furi_check((progress >= 0.0f) && (progress <= 1.0f));
    size_t height = 11;

    float progress_width = roundf(progress * (width - 2));

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x + 1, y + 1, width - 2, height - 2);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_rframe(canvas, x, y, width, height, 3);

    canvas_draw_box(canvas, x + 1, y + 1, progress_width, height - 2);

    canvas_set_color(canvas, ColorXOR);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, x + width / 2, y + 2, AlignCenter, AlignTop, text);
}

void elements_scrollbar_pos(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t height,
    size_t pos,
    size_t total) {
    furi_check(canvas);

    // prevent overflows
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x - 3, y, 3, height);

    // dot line
    canvas_set_color(canvas, ColorBlack);
    for(int32_t i = y; i < (int32_t)height + y; i += 2) {
        canvas_draw_dot(canvas, x - 2, i);
    }

    // Position block
    if(total) {
        float block_h = ((float)height) / total;
        canvas_draw_box(canvas, x - 3, y + (block_h * pos), 3, MAX(block_h, 1));
    }
}

void elements_scrollbar(Canvas* canvas, size_t pos, size_t total) {
    furi_check(canvas);

    size_t width = canvas_width(canvas);
    size_t height = canvas_height(canvas);

    // prevent overflows
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, width - 3, 0, 3, height);

    // dot line
    canvas_set_color(canvas, ColorBlack);
    for(size_t i = 0; i < height; i += 2) {
        canvas_draw_dot(canvas, width - 2, i);
    }

    // Position block
    if(total) {
        float block_h = ((float)height) / total;
        canvas_draw_box(canvas, width - 3, block_h * pos, 3, MAX(block_h, 1));
    }
}

void elements_frame(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height) {
    furi_check(canvas);

    canvas_draw_line(canvas, x + 2, y, x + width - 2, y);
    canvas_draw_line(canvas, x + 1, y + height - 1, x + width, y + height - 1);
    canvas_draw_line(canvas, x + 2, y + height, x + width - 1, y + height);

    canvas_draw_line(canvas, x, y + 2, x, y + height - 2);
    canvas_draw_line(canvas, x + width - 1, y + 1, x + width - 1, y + height - 2);
    canvas_draw_line(canvas, x + width, y + 2, x + width, y + height - 2);

    canvas_draw_dot(canvas, x + 1, y + 1);
}

void elements_button_left(Canvas* canvas, const char* str) {
    furi_check(canvas);

    const size_t button_height = 12;
    const size_t vertical_offset = 3;
    const size_t horizontal_offset = 3;
    const size_t string_width = canvas_string_width(canvas, str);
    const Icon* icon = &I_ButtonLeft_4x7;
    const int32_t icon_h_offset = 3;
    const int32_t icon_width_with_offset = icon->width + icon_h_offset;
    const int32_t icon_v_offset = icon->height + vertical_offset;
    const size_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const int32_t x = 0;
    const int32_t y = canvas_height(canvas);

    canvas_draw_box(canvas, x, y - button_height, button_width, button_height);
    canvas_draw_line(canvas, x + button_width + 0, y, x + button_width + 0, y - button_height + 0);
    canvas_draw_line(canvas, x + button_width + 1, y, x + button_width + 1, y - button_height + 1);
    canvas_draw_line(canvas, x + button_width + 2, y, x + button_width + 2, y - button_height + 2);

    canvas_invert_color(canvas);
    canvas_draw_icon(canvas, x + horizontal_offset, y - icon_v_offset, &I_ButtonLeft_4x7);
    canvas_draw_str(
        canvas, x + horizontal_offset + icon_width_with_offset, y - vertical_offset, str);
    canvas_invert_color(canvas);
}

void elements_button_right(Canvas* canvas, const char* str) {
    furi_check(canvas);

    const size_t button_height = 12;
    const size_t vertical_offset = 3;
    const size_t horizontal_offset = 3;
    const size_t string_width = canvas_string_width(canvas, str);
    const Icon* icon = &I_ButtonRight_4x7;
    const int32_t icon_h_offset = 3;
    const int32_t icon_width_with_offset = icon->width + icon_h_offset;
    const int32_t icon_v_offset = icon->height + vertical_offset;
    const size_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const int32_t x = canvas_width(canvas);
    const int32_t y = canvas_height(canvas);

    canvas_draw_box(canvas, x - button_width, y - button_height, button_width, button_height);
    canvas_draw_line(canvas, x - button_width - 1, y, x - button_width - 1, y - button_height + 0);
    canvas_draw_line(canvas, x - button_width - 2, y, x - button_width - 2, y - button_height + 1);
    canvas_draw_line(canvas, x - button_width - 3, y, x - button_width - 3, y - button_height + 2);

    canvas_invert_color(canvas);
    canvas_draw_str(canvas, x - button_width + horizontal_offset, y - vertical_offset, str);
    canvas_draw_icon(
        canvas, x - horizontal_offset - icon->width, y - icon_v_offset, &I_ButtonRight_4x7);
    canvas_invert_color(canvas);
}

void elements_button_up(Canvas* canvas, const char* str) {
    furi_check(canvas);

    const Icon* icon = &I_ButtonUp_7x4;

    const size_t button_height = 12;
    const size_t vertical_offset = 3;
    const size_t horizontal_offset = 3;
    const size_t string_width = canvas_string_width(canvas, str);
    const int32_t icon_h_offset = 3;
    const int32_t icon_width_with_offset = icon_get_width(icon) + icon_h_offset;
    const int32_t icon_v_offset = icon_get_height(icon) + (int32_t)vertical_offset;
    const size_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const int32_t x = 0;
    const int32_t y = 0 + button_height;

    int32_t line_x = x + button_width;
    int32_t line_y = y - button_height;

    canvas_draw_box(canvas, x, line_y, button_width, button_height);
    canvas_draw_line(canvas, line_x + 0, line_y, line_x + 0, y - 1);
    canvas_draw_line(canvas, line_x + 1, line_y, line_x + 1, y - 2);
    canvas_draw_line(canvas, line_x + 2, line_y, line_x + 2, y - 3);

    canvas_invert_color(canvas);
    canvas_draw_icon(canvas, x + horizontal_offset, y - icon_v_offset, icon);
    canvas_draw_str(
        canvas, x + horizontal_offset + icon_width_with_offset, y - vertical_offset, str);
    canvas_invert_color(canvas);
}

void elements_button_down(Canvas* canvas, const char* str) {
    furi_check(canvas);

    const Icon* icon = &I_ButtonDown_7x4;

    const size_t button_height = 12;
    const size_t vertical_offset = 3;
    const size_t horizontal_offset = 3;
    const size_t string_width = canvas_string_width(canvas, str);
    const int32_t icon_h_offset = 3;
    const int32_t icon_width_with_offset = icon_get_width(icon) + icon_h_offset;
    const int32_t icon_v_offset = icon_get_height(icon) + vertical_offset + 1;
    const size_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const int32_t x = canvas_width(canvas);
    const int32_t y = button_height;

    int32_t line_x = x - button_width;
    int32_t line_y = y - button_height;

    canvas_draw_box(canvas, line_x, line_y, button_width, button_height);
    canvas_draw_line(canvas, line_x - 1, line_y, line_x - 1, y - 1);
    canvas_draw_line(canvas, line_x - 2, line_y, line_x - 2, y - 2);
    canvas_draw_line(canvas, line_x - 3, line_y, line_x - 3, y - 3);

    canvas_invert_color(canvas);
    canvas_draw_str(canvas, x - button_width + horizontal_offset, y - vertical_offset, str);
    canvas_draw_icon(
        canvas, x - horizontal_offset - icon_get_width(icon), y - icon_v_offset, icon);
    canvas_invert_color(canvas);
}

void elements_button_center(Canvas* canvas, const char* str) {
    furi_check(canvas);

    const size_t button_height = 12;
    const size_t vertical_offset = 3;
    const size_t horizontal_offset = 1;
    const size_t string_width = canvas_string_width(canvas, str);
    const Icon* icon = &I_ButtonCenter_7x7;
    const int32_t icon_h_offset = 3;
    const int32_t icon_width_with_offset = icon->width + icon_h_offset;
    const int32_t icon_v_offset = icon->height + vertical_offset;
    const size_t button_width = string_width + horizontal_offset * 2 + icon_width_with_offset;

    const int32_t x = (canvas_width(canvas) - button_width) / 2;
    const int32_t y = canvas_height(canvas);

    canvas_draw_box(canvas, x, y - button_height, button_width, button_height);

    canvas_draw_line(canvas, x - 1, y, x - 1, y - button_height + 0);
    canvas_draw_line(canvas, x - 2, y, x - 2, y - button_height + 1);
    canvas_draw_line(canvas, x - 3, y, x - 3, y - button_height + 2);

    canvas_draw_line(canvas, x + button_width + 0, y, x + button_width + 0, y - button_height + 0);
    canvas_draw_line(canvas, x + button_width + 1, y, x + button_width + 1, y - button_height + 1);
    canvas_draw_line(canvas, x + button_width + 2, y, x + button_width + 2, y - button_height + 2);

    canvas_invert_color(canvas);
    canvas_draw_icon(canvas, x + horizontal_offset, y - icon_v_offset, &I_ButtonCenter_7x7);
    canvas_draw_str(
        canvas, x + horizontal_offset + icon_width_with_offset, y - vertical_offset, str);
    canvas_invert_color(canvas);
}

static size_t
    elements_get_max_chars_to_fit(Canvas* canvas, Align horizontal, const char* text, int32_t x) {
    const char* end = strchr(text, '\n');
    if(end == NULL) {
        end = text + strlen(text);
    }
    size_t text_size = end - text;
    FuriString* str;
    str = furi_string_alloc_set(text);
    furi_string_left(str, text_size);
    size_t result = 0;

    size_t len_px = canvas_string_width(canvas, furi_string_get_cstr(str));
    size_t px_left = 0;
    if(horizontal == AlignCenter) {
        if(x > (int32_t)(canvas_width(canvas) / 2)) {
            px_left = (canvas_width(canvas) - x) * 2;
        } else {
            px_left = x * 2;
        }
    } else if(horizontal == AlignLeft) {
        px_left = canvas_width(canvas) - x;
    } else if(horizontal == AlignRight) {
        px_left = x;
    } else {
        furi_crash();
    }

    if(len_px > px_left) {
        size_t excess_symbols_approximately =
            ceilf((float)(len_px - px_left) / ((float)len_px / (float)text_size));
        // reduce to 5 to be sure dash fit, and next line will be at least 5 symbols long
        if(excess_symbols_approximately > 0) {
            excess_symbols_approximately = MAX(excess_symbols_approximately, 5u);
            result = text_size - excess_symbols_approximately - 1;
        } else {
            result = text_size;
        }
    } else {
        result = text_size;
    }

    furi_string_free(str);
    return result;
}

void elements_multiline_text_aligned(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    Align horizontal,
    Align vertical,
    const char* text) {
    furi_check(canvas);
    furi_check(text);

    size_t lines_count = 0;
    size_t font_height = canvas_current_font_height(canvas);
    FuriString* line;

    /* go through text line by line and count lines */
    for(const char* start = text; start[0];) {
        size_t chars_fit = elements_get_max_chars_to_fit(canvas, horizontal, start, x);
        ++lines_count;
        start += chars_fit;
        start += start[0] == '\n' ? 1 : 0;
    }

    if(vertical == AlignBottom) {
        y -= font_height * (lines_count - 1);
    } else if(vertical == AlignCenter) {
        y -= (font_height * (lines_count - 1)) / 2;
    }

    /* go through text line by line and print them */
    for(const char* start = text; start[0];) {
        size_t chars_fit = elements_get_max_chars_to_fit(canvas, horizontal, start, x);

        if((start[chars_fit] == '\n') || (start[chars_fit] == 0)) {
            line = furi_string_alloc_printf("%.*s", chars_fit, start);
        } else if((y + font_height) > canvas_height(canvas)) {
            line = furi_string_alloc_printf("%.*s...\n", chars_fit, start);
        } else {
            chars_fit -= 1; // account for the dash
            line = furi_string_alloc_printf("%.*s-\n", chars_fit, start);
        }
        canvas_draw_str_aligned(canvas, x, y, horizontal, vertical, furi_string_get_cstr(line));
        furi_string_free(line);
        y += font_height;
        if(y > (int32_t)canvas_height(canvas)) {
            break;
        }

        start += chars_fit;
        start += start[0] == '\n' ? 1 : 0;
    }
}

void elements_multiline_text(Canvas* canvas, int32_t x, int32_t y, const char* text) {
    furi_check(canvas);
    furi_check(text);

    size_t font_height = canvas_current_font_height(canvas);
    FuriString* str;
    str = furi_string_alloc();
    const char* start = text;
    char* end;
    do {
        end = strchr(start, '\n');
        if(end) {
            furi_string_set_strn(str, start, end - start);
            start = end + 1;
        } else {
            furi_string_set(str, start);
        }
        canvas_draw_str(canvas, x, y, furi_string_get_cstr(str));
        y += font_height;
    } while(end && y < 64);
    furi_string_free(str);
}

void elements_multiline_text_framed(Canvas* canvas, int32_t x, int32_t y, const char* text) {
    furi_check(canvas);
    furi_check(text);

    size_t font_height = canvas_current_font_height(canvas);
    size_t str_width = canvas_string_width(canvas, text);

    // count \n's
    size_t lines = 1;
    const char* t = text;
    while(*t != '\0') {
        if(*t == '\n') {
            lines++;
            size_t temp_width = canvas_string_width(canvas, t + 1);
            str_width = temp_width > str_width ? temp_width : str_width;
        }
        t++;
    }

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x, y - font_height, str_width + 8, font_height * lines + 4);
    canvas_set_color(canvas, ColorBlack);
    elements_multiline_text(canvas, x + 4, y - 1, text);
    elements_frame(canvas, x, y - font_height, str_width + 8, font_height * lines + 4);
}

void elements_slightly_rounded_frame(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height) {
    furi_check(canvas);
    canvas_draw_rframe(canvas, x, y, width, height, 1);
}

void elements_slightly_rounded_box(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height) {
    furi_check(canvas);
    canvas_draw_rbox(canvas, x, y, width, height, 1);
}

void elements_bold_rounded_frame(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height) {
    furi_check(canvas);

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, x + 2, y + 2, width - 3, height - 3);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_line(canvas, x + 3, y, x + width - 3, y);
    canvas_draw_line(canvas, x + 2, y + 1, x + width - 2, y + 1);

    canvas_draw_line(canvas, x, y + 3, x, y + height - 3);
    canvas_draw_line(canvas, x + 1, y + 2, x + 1, y + height - 2);

    canvas_draw_line(canvas, x + width, y + 3, x + width, y + height - 3);
    canvas_draw_line(canvas, x + width - 1, y + 2, x + width - 1, y + height - 2);

    canvas_draw_line(canvas, x + 3, y + height, x + width - 3, y + height);
    canvas_draw_line(canvas, x + 2, y + height - 1, x + width - 2, y + height - 1);

    canvas_draw_dot(canvas, x + 2, y + 2);
    canvas_draw_dot(canvas, x + 3, y + 2);
    canvas_draw_dot(canvas, x + 2, y + 3);

    canvas_draw_dot(canvas, x + width - 2, y + 2);
    canvas_draw_dot(canvas, x + width - 3, y + 2);
    canvas_draw_dot(canvas, x + width - 2, y + 3);

    canvas_draw_dot(canvas, x + 2, y + height - 2);
    canvas_draw_dot(canvas, x + 3, y + height - 2);
    canvas_draw_dot(canvas, x + 2, y + height - 3);

    canvas_draw_dot(canvas, x + width - 2, y + height - 2);
    canvas_draw_dot(canvas, x + width - 3, y + height - 2);
    canvas_draw_dot(canvas, x + width - 2, y + height - 3);
}

void elements_bubble(Canvas* canvas, int32_t x, int32_t y, size_t width, size_t height) {
    furi_check(canvas);
    canvas_draw_rframe(canvas, x + 4, y, width, height, 3);
    int32_t y_corner = y + height * 2 / 3;
    canvas_draw_line(canvas, x, y_corner, x + 4, y_corner - 4);
    canvas_draw_line(canvas, x, y_corner, x + 4, y_corner + 4);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_line(canvas, x + 4, y_corner - 3, x + 4, y_corner + 3);
    canvas_set_color(canvas, ColorBlack);
}

void elements_bubble_str(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    const char* text,
    Align horizontal,
    Align vertical) {
    furi_check(canvas);
    furi_check(text);

    size_t font_height = canvas_current_font_height(canvas);
    size_t str_width = canvas_string_width(canvas, text);

    // count \n's
    size_t lines = 1;
    const char* t = text;
    while(*t != '\0') {
        if(*t == '\n') {
            lines++;
            size_t temp_width = canvas_string_width(canvas, t + 1);
            str_width = temp_width > str_width ? temp_width : str_width;
        }
        t++;
    }

    int32_t frame_x = x;
    int32_t frame_y = y;
    size_t frame_width = str_width + 8;
    size_t frame_height = font_height * lines + 4;

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_box(canvas, frame_x + 1, frame_y + 1, frame_width - 2, frame_height - 2);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_rframe(canvas, frame_x, frame_y, frame_width, frame_height, 1);
    elements_multiline_text(canvas, x + 4, y - 1 + font_height, text);

    int32_t x1 = 0;
    int32_t x2 = 0;
    int32_t x3 = 0;
    int32_t y1 = 0;
    int32_t y2 = 0;
    int32_t y3 = 0;
    if((horizontal == AlignLeft) && (vertical == AlignTop)) {
        x1 = frame_x;
        y1 = frame_y;
        x2 = frame_x - 4;
        y2 = frame_y;
        x3 = frame_x;
        y3 = frame_y + 4;
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, x2 + 2, y2 + 1, 2, 2);
        canvas_set_color(canvas, ColorBlack);
    } else if((horizontal == AlignLeft) && (vertical == AlignCenter)) {
        x1 = frame_x;
        y1 = frame_y + (frame_height - 1) / 2 - 4;
        x2 = frame_x - 4;
        y2 = frame_y + (frame_height - 1) / 2;
        x3 = frame_x;
        y3 = frame_y + (frame_height - 1) / 2 + 4;
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, x2 + 2, y2 - 2, 2, 5);
        canvas_draw_dot(canvas, x2 + 1, y2);
        canvas_set_color(canvas, ColorBlack);
    } else if((horizontal == AlignLeft) && (vertical == AlignBottom)) {
        x1 = frame_x;
        y1 = frame_y + (frame_height - 1) - 4;
        x2 = frame_x - 4;
        y2 = frame_y + (frame_height - 1);
        x3 = frame_x;
        y3 = frame_y + (frame_height - 1);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, x2 + 2, y2 - 2, 2, 2);
        canvas_set_color(canvas, ColorBlack);
    } else if((horizontal == AlignRight) && (vertical == AlignTop)) {
        x1 = frame_x + (frame_width - 1);
        y1 = frame_y;
        x2 = frame_x + (frame_width - 1) + 4;
        y2 = frame_y;
        x3 = frame_x + (frame_width - 1);
        y3 = frame_y + 4;
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, x2 - 3, y2 + 1, 2, 2);
        canvas_set_color(canvas, ColorBlack);
    } else if((horizontal == AlignRight) && (vertical == AlignCenter)) {
        x1 = frame_x + (frame_width - 1);
        y1 = frame_y + (frame_height - 1) / 2 - 4;
        x2 = frame_x + (frame_width - 1) + 4;
        y2 = frame_y + (frame_height - 1) / 2;
        x3 = frame_x + (frame_width - 1);
        y3 = frame_y + (frame_height - 1) / 2 + 4;
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, x2 - 3, y2 - 2, 2, 5);
        canvas_draw_dot(canvas, x2 - 1, y2);
        canvas_set_color(canvas, ColorBlack);
    } else if((horizontal == AlignRight) && (vertical == AlignBottom)) {
        x1 = frame_x + (frame_width - 1);
        y1 = frame_y + (frame_height - 1) - 4;
        x2 = frame_x + (frame_width - 1) + 4;
        y2 = frame_y + (frame_height - 1);
        x3 = frame_x + (frame_width - 1);
        y3 = frame_y + (frame_height - 1);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, x2 - 3, y2 - 2, 2, 2);
        canvas_set_color(canvas, ColorBlack);
    } else if((horizontal == AlignCenter) && (vertical == AlignTop)) {
        x1 = frame_x + (frame_width - 1) / 2 - 4;
        y1 = frame_y;
        x2 = frame_x + (frame_width - 1) / 2;
        y2 = frame_y - 4;
        x3 = frame_x + (frame_width - 1) / 2 + 4;
        y3 = frame_y;
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, x2 - 2, y2 + 2, 5, 2);
        canvas_draw_dot(canvas, x2, y2 + 1);
        canvas_set_color(canvas, ColorBlack);
    } else if((horizontal == AlignCenter) && (vertical == AlignBottom)) {
        x1 = frame_x + (frame_width - 1) / 2 - 4;
        y1 = frame_y + (frame_height - 1);
        x2 = frame_x + (frame_width - 1) / 2;
        y2 = frame_y + (frame_height - 1) + 4;
        x3 = frame_x + (frame_width - 1) / 2 + 4;
        y3 = frame_y + (frame_height - 1);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_box(canvas, x2 - 2, y2 - 3, 5, 2);
        canvas_draw_dot(canvas, x2, y2 - 1);
        canvas_set_color(canvas, ColorBlack);
    }

    canvas_set_color(canvas, ColorWhite);
    canvas_draw_line(canvas, x3, y3, x1, y1);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_line(canvas, x1, y1, x2, y2);
    canvas_draw_line(canvas, x2, y2, x3, y3);
}

void elements_string_fit_width(Canvas* canvas, FuriString* string, size_t width) {
    furi_check(canvas);
    furi_check(string);

    size_t len_px = canvas_string_width(canvas, furi_string_get_cstr(string));
    if(len_px > width) {
        width -= canvas_string_width(canvas, "...");
        do {
            furi_string_left(string, furi_string_size(string) - 1);
            len_px = canvas_string_width(canvas, furi_string_get_cstr(string));
        } while(len_px > width);
        furi_string_cat(string, "...");
    }
}

void elements_scrollable_text_line(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    FuriString* string,
    size_t scroll,
    bool ellipsis) {
    furi_check(canvas);
    furi_check(string);

    FuriString* line = furi_string_alloc_set(string);

    size_t len_px = canvas_string_width(canvas, furi_string_get_cstr(line));
    if(len_px > width) {
        if(ellipsis) {
            width -= canvas_string_width(canvas, "...");
        }

        // Calculate scroll size
        size_t scroll_size = furi_string_size(line);
        size_t right_width = 0;
        for(size_t i = scroll_size; i > 0; i--) {
            right_width += canvas_glyph_width(canvas, furi_string_get_char(line, i));
            if(right_width > width) break;
            scroll_size--;
            if(!scroll_size) break;
        }
        // Ensure that we have something to scroll
        if(scroll_size) {
            scroll_size += 3;
            scroll = scroll % scroll_size;
            furi_string_right(line, scroll);
        }

        len_px = canvas_string_width(canvas, furi_string_get_cstr(line));
        while(len_px > width) {
            furi_string_left(line, furi_string_size(line) - 1);
            len_px = canvas_string_width(canvas, furi_string_get_cstr(line));
        }

        if(ellipsis) {
            furi_string_cat(line, "...");
        }
    }

    canvas_draw_str(canvas, x, y, furi_string_get_cstr(line));
    furi_string_free(line);
}

void elements_text_box(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    size_t width,
    size_t height,
    Align horizontal,
    Align vertical,
    const char* text,
    bool strip_to_dots) {
    furi_check(canvas);

    ElementTextBoxLine line[ELEMENTS_MAX_LINES_NUM];
    bool bold = false;
    bool mono = false;
    bool inverse = false;
    bool inverse_present = false;
    Font current_font = FontSecondary;
    Font prev_font = FontSecondary;
    const CanvasFontParameters* font_params = canvas_get_font_params(canvas, current_font);

    // Fill line parameters
    size_t line_leading_min = font_params->leading_min;
    size_t line_leading_default = font_params->leading_default;
    size_t line_height = font_params->height;
    size_t line_descender = font_params->descender;
    size_t line_num = 0;
    size_t line_width = 0;
    size_t line_len = 0;
    size_t total_height_min = 0;
    size_t total_height_default = 0;
    size_t i = 0;
    bool full_text_processed = false;
    size_t dots_width = canvas_string_width(canvas, "...");

    canvas_set_font(canvas, FontSecondary);

    // Fill all lines
    line[0].text = text;
    for(i = 0; !full_text_processed; i++) {
        line_len++;
        // Identify line height
        if(prev_font != current_font) {
            font_params = canvas_get_font_params(canvas, current_font);
            line_leading_min = MAX(line_leading_min, font_params->leading_min);
            line_leading_default = MAX(line_leading_default, font_params->leading_default);
            line_height = MAX(line_height, font_params->height);
            line_descender = MAX(line_descender, font_params->descender);
            prev_font = current_font;
        }
        // Set the font
        if(text[i] == '\e' && text[i + 1]) {
            i++;
            line_len++;
            if(text[i] == ELEMENTS_BOLD_MARKER) {
                if(bold) {
                    current_font = FontSecondary;
                } else {
                    current_font = FontPrimary;
                }
                canvas_set_font(canvas, current_font);
                bold = !bold;
            }
            if(text[i] == ELEMENTS_MONO_MARKER) {
                if(mono) {
                    current_font = FontSecondary;
                } else {
                    current_font = FontKeyboard;
                }
                canvas_set_font(canvas, FontKeyboard);
                mono = !mono;
            }
            if(text[i] == ELEMENTS_INVERSE_MARKER) {
                inverse_present = true;
            }
            continue;
        }
        if(text[i] != '\n') {
            line_width += canvas_glyph_width(canvas, text[i]);
        }
        // Process new line
        if(text[i] == '\n' || text[i] == '\0' || line_width > width) {
            if(line_width > width) {
                line_width -= canvas_glyph_width(canvas, text[i--]);
                line_len--;
            }
            if(text[i] == '\0') {
                full_text_processed = true;
            }
            if(inverse_present) {
                line_leading_min += 1;
                line_leading_default += 1;
                inverse_present = false;
            }
            line[line_num].leading_min = line_leading_min;
            line[line_num].leading_default = line_leading_default;
            line[line_num].height = line_height;
            line[line_num].descender = line_descender;
            if(total_height_min + line_leading_min > height) {
                break;
            }
            total_height_min += line_leading_min;
            total_height_default += line_leading_default;
            line[line_num].len = line_len;
            if(horizontal == AlignCenter) {
                line[line_num].x = x + (width - line_width) / 2;
            } else if(horizontal == AlignRight) {
                line[line_num].x = x + (width - line_width);
            } else {
                line[line_num].x = x;
            }
            line[line_num].y = total_height_min;
            line_num++;
            if(!full_text_processed) {
                line[line_num].text = &text[i + 1];
            }
            line_leading_min = font_params->leading_min;
            line_height = font_params->height;
            line_descender = font_params->descender;
            line_width = 0;
            line_len = 0;
        }
    }

    // Set vertical alignment for all lines
    if(total_height_default < height) {
        if(vertical == AlignTop) {
            line[0].y = y + line[0].height;
        } else if(vertical == AlignCenter) {
            line[0].y = y + line[0].height + (height - total_height_default) / 2;
        } else if(vertical == AlignBottom) {
            line[0].y = y + line[0].height + (height - total_height_default);
        }
        if(line_num > 1) {
            for(size_t i = 1; i < line_num; i++) {
                line[i].y = line[i - 1].y + line[i - 1].leading_default;
            }
        }
    } else if(line_num > 1) {
        size_t free_pixel_num = height - total_height_min;
        size_t fill_pixel = 0;
        size_t j = 1;
        line[0].y = y + line[0].height;
        while(fill_pixel < free_pixel_num) {
            line[j].y = line[j - 1].y + line[j - 1].leading_min + 1;
            fill_pixel++;
            j = j % (line_num - 1) + 1;
        }
    }

    // Draw line by line
    canvas_set_font(canvas, FontSecondary);
    bold = false;
    mono = false;
    inverse = false;
    for(size_t i = 0; i < line_num; i++) {
        for(size_t j = 0; j < line[i].len; j++) {
            // Process format symbols
            if(line[i].text[j] == '\e' && j < line[i].len - 1) {
                ++j;
                if(line[i].text[j] == ELEMENTS_BOLD_MARKER) {
                    if(bold) {
                        current_font = FontSecondary;
                    } else {
                        current_font = FontPrimary;
                    }
                    canvas_set_font(canvas, current_font);
                    bold = !bold;
                    continue;
                }
                if(line[i].text[j] == ELEMENTS_MONO_MARKER) {
                    if(mono) {
                        current_font = FontSecondary;
                    } else {
                        current_font = FontKeyboard;
                    }
                    canvas_set_font(canvas, current_font);
                    mono = !mono;
                    continue;
                }
                if(line[i].text[j] == ELEMENTS_INVERSE_MARKER) {
                    inverse = !inverse;
                    continue;
                }
            }
            if(inverse) {
                canvas_draw_box(
                    canvas,
                    line[i].x - 1,
                    line[i].y - line[i].height - 1,
                    canvas_glyph_width(canvas, line[i].text[j]) + 1,
                    line[i].height + line[i].descender + 2);
                canvas_invert_color(canvas);
                canvas_draw_glyph(canvas, line[i].x, line[i].y, line[i].text[j]);
                canvas_invert_color(canvas);
            } else {
                if((i == line_num - 1) && strip_to_dots) {
                    size_t next_symbol_width = canvas_glyph_width(canvas, line[i].text[j]);
                    if((line[i].x + (int32_t)next_symbol_width + (int32_t)dots_width) >
                       (x + (int32_t)width)) {
                        canvas_draw_str(canvas, line[i].x, line[i].y, "...");
                        break;
                    }
                }
                canvas_draw_glyph(canvas, line[i].x, line[i].y, line[i].text[j]);
            }
            line[i].x += canvas_glyph_width(canvas, line[i].text[j]);
        }
    }
    canvas_set_font(canvas, FontSecondary);
}
