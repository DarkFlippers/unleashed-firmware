#include "flipper.h"
#include "u8g2/u8g2.h"

extern TIM_HandleTypeDef htim5;

void coreglitch_demo_0(void* p) {
    FuriRecordSubscriber* log = get_default_log();

    fuprintf(log, "coreglitch demo!\n");

    // open record
    FuriRecordSubscriber* fb_record =
        furi_open_deprecated("u8g2_fb", false, false, NULL, NULL, NULL);

    if(fb_record == NULL) {
        fuprintf(log, "[widget] cannot create fb record\n");
        furiac_exit(NULL);
    }

    float notes[] = {
        0.0,
        330.0,
        220.0,
        0.0,
        110.0 + 55.0,
        440.0,
        330.0,
        55.0,
    };

    float scales[] = {
        1.0,
        1.5,
        0.75,
        0.8,
    };

    uint8_t cnt = 0;

    while(1) {
        for(size_t note_idx = 0; note_idx < 400; note_idx++) {
            float scale = scales[((cnt + note_idx) / 16) % 4];

            float freq = notes[(note_idx + cnt / 2) % 8] * scale;
            float width = 0.001 + 0.05 * (note_idx % (cnt / 7 + 5));

            if(note_idx % 8 == 0) {
                freq = 0;
            }

            // TODO get sound from FURI
            pwm_set(width, freq, &htim5, TIM_CHANNEL_4);
            // delay(1);

            cnt++;

            u8g2_t* fb = furi_take(fb_record);
            if(fb != NULL) {
                u8g2_SetDrawColor(fb, 0);
                u8g2_DrawBox(fb, 0, 0, 120, 30);

                u8g2_SetFont(fb, u8g2_font_6x10_mf);
                u8g2_SetDrawColor(fb, 1);
                u8g2_SetFontMode(fb, 1);
                char buf[64];
                sprintf(buf, "freq: %d Hz", (uint32_t)freq);
                u8g2_DrawStr(fb, 2 + width * 20, 12 + freq / 100, buf);
            }
            furi_commit(fb_record);

            delay(100);
        }
    }
}
