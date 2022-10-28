#include "sender_view.h"

void draw_sender_view(Canvas* canvas, i2cSender* i2c_sender) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_rframe(canvas, 0, 0, 128, 64, 3);
    canvas_draw_icon(canvas, 2, 13, &I_passport_happy2_46x49);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 3, 3, AlignLeft, AlignTop, SEND_MENU_TEXT);

    if(!i2c_sender->scanner->scanned) {
        scan_i2c_bus(i2c_sender->scanner);
    }

    canvas_set_font(canvas, FontSecondary);
    if(i2c_sender->scanner->nb_found <= 0) {
        canvas_draw_str_aligned(canvas, 60, 5, AlignLeft, AlignTop, "No peripherals");
        canvas_draw_str_aligned(canvas, 60, 15, AlignLeft, AlignTop, "Found");
        return;
    }
    canvas_draw_rbox(canvas, 70, 48, 45, 13, 3);
    canvas_set_color(canvas, ColorWhite);
    canvas_draw_icon(canvas, 75, 50, &I_Ok_btn_9x9);
    canvas_draw_str_aligned(canvas, 85, 51, AlignLeft, AlignTop, "Send");
    canvas_set_color(canvas, ColorBlack);
    canvas_draw_str_aligned(canvas, 50, 5, AlignLeft, AlignTop, "Addr: ");
    canvas_draw_icon(canvas, 80, 5, &I_ButtonLeft_4x7);
    canvas_draw_icon(canvas, 115, 5, &I_ButtonRight_4x7);
    char addr_text[8];
    snprintf(
        addr_text,
        sizeof(addr_text),
        "0x%02x",
        (int)i2c_sender->scanner->addresses[i2c_sender->address_idx]);
    canvas_draw_str_aligned(canvas, 90, 5, AlignLeft, AlignTop, addr_text);
    canvas_draw_str_aligned(canvas, 50, 15, AlignLeft, AlignTop, "Value: ");

    canvas_draw_icon(canvas, 80, 17, &I_ButtonUp_7x4);
    canvas_draw_icon(canvas, 115, 17, &I_ButtonDown_7x4);
    snprintf(addr_text, sizeof(addr_text), "0x%02x", (int)i2c_sender->value);
    canvas_draw_str_aligned(canvas, 90, 15, AlignLeft, AlignTop, addr_text);
    if(i2c_sender->must_send) {
        i2c_send(i2c_sender);
    }
    canvas_draw_str_aligned(canvas, 50, 25, AlignLeft, AlignTop, "Result: ");
    if(i2c_sender->sended) {
        for(uint8_t i = 0; i < sizeof(i2c_sender->recv); i++) {
            snprintf(addr_text, sizeof(addr_text), "0x%02x", (int)i2c_sender->recv[i]);
            canvas_draw_str_aligned(canvas, 90, 25 + (i * 10), AlignLeft, AlignTop, addr_text);
        }
    }
}