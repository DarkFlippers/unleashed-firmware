#include "u8g2/u8g2.h"
#include <furi.h>

void u8g2_example(void* p) {
    // open record
    u8g2_t* fb = furi_record_open("u8g2_fb");
    u8g2_SetFont(fb, u8g2_font_6x10_mf);
    u8g2_SetDrawColor(fb, 1);
    u8g2_SetFontMode(fb, 1);
    u8g2_DrawStr(fb, 2, 12, "hello world!");
    furi_record_close("u8g2_fb");

    furiac_exit(NULL);
}