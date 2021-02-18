#include <furi.h>
#include <api-hal.h>

#include "u8g2/u8g2.h"

extern TIM_HandleTypeDef SPEAKER_TIM;

int32_t coreglitch_demo_0(void* p) {
    printf("coreglitch demo!\r\n");

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
            hal_pwm_set(width, freq, &SPEAKER_TIM, SPEAKER_CH);

            // delay(1);

            cnt++;

            delay(100);
        }
    }

    return 0;
}
