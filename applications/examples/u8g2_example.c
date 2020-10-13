#include "u8g2/u8g2.h"
#include "flipper.h"

void u8g2_example(void* p) {
    FuriRecordSubscriber* log = get_default_log();

    // open record
    FuriRecordSubscriber* fb_record = furi_open_deprecated("u8g2_fb", false, false, NULL, NULL, NULL);

    if(fb_record == NULL) {
        fuprintf(log, "[widget] cannot create fb record\n");
        furiac_exit(NULL);
    }

    while(1) {
        u8g2_t* fb = furi_take(fb_record);
        if(fb != NULL) {
            u8g2_SetFont(fb, u8g2_font_6x10_mf);
            u8g2_SetDrawColor(fb, 1);
            u8g2_SetFontMode(fb, 1);
            u8g2_DrawStr(fb, 2, 12, "hello world!");
        }
        furi_commit(fb_record);

        if(fb != NULL) {
            furiac_exit(NULL);
        }

        delay(1);
    }
}