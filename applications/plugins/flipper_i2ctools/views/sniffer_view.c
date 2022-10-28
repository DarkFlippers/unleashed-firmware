#include "sniffer_view.h"

void draw_sniffer_view(Canvas* canvas, i2cSniffer* i2c_sniffer) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_rframe(canvas, 0, 0, 128, 64, 3);
    canvas_draw_icon(canvas, 2, 13, &I_passport_happy2_46x49);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 3, 3, AlignLeft, AlignTop, SNIFF_MENU_TEXT);
    canvas_set_font(canvas, FontSecondary);

    // Button
    canvas_draw_rbox(canvas, 70, 48, 45, 13, 3);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_icon(canvas, 75, 50, &I_Ok_btn_9x9);
    if(!i2c_sniffer->started) {
        canvas_draw_str_aligned(canvas, 85, 51, AlignLeft, AlignTop, "Start");
    } else {
        canvas_draw_str_aligned(canvas, 85, 51, AlignLeft, AlignTop, "Stop");
    }
    canvas_set_color(canvas, ColorBlack);
    // Address text
    char addr_text[8];
    snprintf(
        addr_text,
        sizeof(addr_text),
        "0x%02x",
        (int)(i2c_sniffer->frames[i2c_sniffer->menu_index].data[0] >> 1));
    canvas_draw_str_aligned(canvas, 50, 3, AlignLeft, AlignTop, "Addr: ");
    canvas_draw_str_aligned(canvas, 75, 3, AlignLeft, AlignTop, addr_text);
    // R/W
    if((int)(i2c_sniffer->frames[i2c_sniffer->menu_index].data[0]) % 2 == 0) {
        canvas_draw_str_aligned(canvas, 105, 3, AlignLeft, AlignTop, "W");
    } else {
        canvas_draw_str_aligned(canvas, 105, 3, AlignLeft, AlignTop, "R");
    }
    // nbFrame text
    canvas_draw_str_aligned(canvas, 50, 13, AlignLeft, AlignTop, "Frames: ");
    snprintf(addr_text, sizeof(addr_text), "%d", (int)i2c_sniffer->menu_index + 1);
    canvas_draw_str_aligned(canvas, 90, 13, AlignLeft, AlignTop, addr_text);
    canvas_draw_str_aligned(canvas, 100, 13, AlignLeft, AlignTop, "/");
    snprintf(addr_text, sizeof(addr_text), "%d", (int)i2c_sniffer->frame_index + 1);
    canvas_draw_str_aligned(canvas, 110, 13, AlignLeft, AlignTop, addr_text);
    // Frames content
    uint8_t x_pos = 0;
    uint8_t y_pos = 23;
    for(uint8_t i = 1; i < i2c_sniffer->frames[i2c_sniffer->menu_index].data_index; i++) {
        snprintf(
            addr_text,
            sizeof(addr_text),
            "0x%02x",
            (int)i2c_sniffer->frames[i2c_sniffer->menu_index].data[i]);
        x_pos = 50 + (i - 1) * 35;
        canvas_draw_str_aligned(canvas, x_pos, y_pos, AlignLeft, AlignTop, addr_text);
        if(i2c_sniffer->frames[i2c_sniffer->menu_index].ack[i]) {
            canvas_draw_str_aligned(canvas, x_pos + 24, y_pos, AlignLeft, AlignTop, "A");
        } else {
            canvas_draw_str_aligned(canvas, x_pos + 24, y_pos, AlignLeft, AlignTop, "N");
        }
    }
}