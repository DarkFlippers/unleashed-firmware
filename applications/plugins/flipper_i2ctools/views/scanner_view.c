#include "scanner_view.h"

void draw_scanner_view(Canvas* canvas, i2cScanner* i2c_scanner) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_rframe(canvas, 0, 0, 128, 64, 3);

    char count_text[46];
    char count_text_fmt[] = "Peripherals Found: %d";
    canvas_set_font(canvas, FontSecondary);
    snprintf(count_text, sizeof(count_text), count_text_fmt, (int)i2c_scanner->nb_found);
    canvas_draw_str_aligned(canvas, 3, 3, AlignLeft, AlignTop, count_text);
    uint8_t x_pos = 0;
    uint8_t y_pos = 0;
    uint8_t idx_to_print = 0;
    for(uint8_t i = 0; i < (int)i2c_scanner->nb_found; i++) {
        idx_to_print = i + i2c_scanner->menu_index * 3;
        if(idx_to_print >= MAX_I2C_ADDR) {
            break;
        }
        snprintf(
            count_text, sizeof(count_text), "0x%02x ", (int)i2c_scanner->addresses[idx_to_print]);
        const uint8_t x_start = 3;
        if(i < 4) {
            x_pos = x_start + (i * 26);
            y_pos = 15;
        } else if(i < 8) {
            x_pos = x_start + ((i - 4) * 26);
            y_pos = 25;
        } else if(i < 12) {
            x_pos = x_start + ((i - 8) * 26);
            y_pos = 35;
        } else {
            break;
        }
        canvas_draw_str_aligned(canvas, x_pos, y_pos, AlignLeft, AlignTop, count_text);
    }
    // Right cursor
    y_pos = 14 + i2c_scanner->menu_index;
    canvas_draw_rbox(canvas, 125, y_pos, 3, 10, 1);

    // Button
    canvas_draw_rbox(canvas, 45, 48, 45, 13, 3);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_icon(canvas, 50, 50, &I_Ok_btn_9x9);
    canvas_draw_str_aligned(canvas, 62, 51, AlignLeft, AlignTop, "Scan");
}