#include "elements.h"

void elements_scrollbar(Canvas* canvas, uint8_t pos, uint8_t total) {
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
        uint8_t block_h = ((float)height) / total;
        canvas_draw_box(canvas, width - 3, block_h * pos, 3, block_h);
    }
}

void elements_frame(Canvas* canvas, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    canvas_draw_line(canvas, x + 2, y, x + width - 2, y);
    canvas_draw_line(canvas, x + 1, y + height - 1, x + width, y + height - 1);
    canvas_draw_line(canvas, x + 2, y + height, x + width - 1, y + height);

    canvas_draw_line(canvas, x, y + 2, x, y + height - 2);
    canvas_draw_line(canvas, x + width - 1, y + 1, x + width - 1, y + height - 2);
    canvas_draw_line(canvas, x + width, y + 2, x + width, y + height - 2);

    canvas_draw_dot(canvas, x + 1, y + 1);
}