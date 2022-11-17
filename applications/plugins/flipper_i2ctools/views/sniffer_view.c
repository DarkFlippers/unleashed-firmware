#include "sniffer_view.h"

void draw_sniffer_view(Canvas* canvas, i2cSniffer* i2c_sniffer) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_rframe(canvas, 0, 0, 128, 64, 3);
    canvas_set_font(canvas, FontSecondary);

    // Button
    canvas_draw_rbox(canvas, 40, 48, 45, 13, 3);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_icon(canvas, 45, 50, &I_Ok_btn_9x9);
    if(!i2c_sniffer->started) {
        canvas_draw_str_aligned(canvas, 57, 51, AlignLeft, AlignTop, "Start");
    } else {
        canvas_draw_str_aligned(canvas, 57, 51, AlignLeft, AlignTop, "Stop");
    }
    canvas_set_color(canvas, ColorBlack);
    if(i2c_sniffer->first) {
        canvas_draw_str_aligned(canvas, 30, 3, AlignLeft, AlignTop, "Nothing Recorded");
        return;
    }
    char text_buffer[10];
    // nbFrame text
    canvas_draw_str_aligned(canvas, 3, 3, AlignLeft, AlignTop, "Frame: ");
    snprintf(
        text_buffer,
        sizeof(text_buffer),
        "%d/%d",
        (int)i2c_sniffer->menu_index + 1,
        (int)i2c_sniffer->frame_index + 1);
    canvas_draw_str_aligned(canvas, 38, 3, AlignLeft, AlignTop, text_buffer);
    // Address text
    snprintf(
        text_buffer,
        sizeof(text_buffer),
        "0x%02x",
        (int)(i2c_sniffer->frames[i2c_sniffer->menu_index].data[0] >> 1));
    canvas_draw_str_aligned(canvas, 3, 13, AlignLeft, AlignTop, "Addr: ");
    canvas_draw_str_aligned(canvas, 30, 13, AlignLeft, AlignTop, text_buffer);
    // R/W
    if((int)(i2c_sniffer->frames[i2c_sniffer->menu_index].data[0]) % 2 == 0) {
        canvas_draw_str_aligned(canvas, 58, 13, AlignLeft, AlignTop, "Write");
    } else {
        canvas_draw_str_aligned(canvas, 58, 13, AlignLeft, AlignTop, "Read");
    }
    // ACK
    if(i2c_sniffer->frames[i2c_sniffer->menu_index].ack[0]) {
        canvas_draw_str_aligned(canvas, 90, 13, AlignLeft, AlignTop, "ACK");
    } else {
        canvas_draw_str_aligned(canvas, 90, 13, AlignLeft, AlignTop, "NACK");
    }
    // Frames content
    const uint8_t x_min = 3;
    const uint8_t y_min = 23;
    uint8_t x_pos = 0;
    uint8_t y_pos = 0;
    uint8_t row = 1;
    uint8_t column = 1;
    uint8_t frame_size = i2c_sniffer->frames[i2c_sniffer->menu_index].data_index;
    uint8_t offset = i2c_sniffer->row_index;
    if(i2c_sniffer->row_index > 0) {
        offset += 1;
    }
    canvas_draw_str_aligned(canvas, x_min, y_min, AlignLeft, AlignTop, "Data:");
    for(uint8_t i = 1 + offset; i < frame_size; i++) {
        snprintf(
            text_buffer,
            sizeof(text_buffer),
            "0x%02x",
            (int)i2c_sniffer->frames[i2c_sniffer->menu_index].data[i]);
        x_pos = x_min + (column - 1) * 35;
        if(row == 1) {
            x_pos += 30;
        }
        y_pos = y_min + (row - 1) * 10;
        canvas_draw_str_aligned(canvas, x_pos, y_pos, AlignLeft, AlignTop, text_buffer);
        if(i2c_sniffer->frames[i2c_sniffer->menu_index].ack[i]) {
            canvas_draw_str_aligned(canvas, x_pos + 24, y_pos, AlignLeft, AlignTop, "A");
        } else {
            canvas_draw_str_aligned(canvas, x_pos + 24, y_pos, AlignLeft, AlignTop, "N");
        }
        column++;
        if((row > 1 && column > 3) || (row == 1 && column > 2)) {
            column = 1;
            row++;
        }
        if(row > 2) {
            break;
        }
    }
}