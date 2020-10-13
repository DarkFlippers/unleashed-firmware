#include "flipper.h"
#include "u8g2.h"

void cc1101_workaround(void* p) {
    FuriRecordSubscriber* fb_record = furi_open_deprecated("u8g2_fb", false, false, NULL, NULL, NULL);

    if(fb_record == NULL) {
        printf("[cc1101] cannot create fb record\n");
        furiac_exit(NULL);
    }

    while(1) {
        u8g2_t* fb = furi_take(fb_record);
        if(fb != NULL) {
            u8g2_SetFont(fb, u8g2_font_6x10_mf);
            u8g2_SetDrawColor(fb, 1);
            u8g2_SetFontMode(fb, 1);
            u8g2_DrawStr(fb, 2, 12, "cc1101 workaround");
        }
        furi_commit(fb_record);

        delay(1000);
    }
}