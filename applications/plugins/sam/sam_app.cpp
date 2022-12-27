#include <furi.h>
#include <furi_hal.h>
#include "stm32_sam.h"
// WOULD BE COOL IF SOMEONE MADE A TEXT ENTRY SCREEN TO HAVE IT READ WHAT IS ENTERED TO TEXT
STM32SAM voice;

extern "C" int32_t sam_app(void* p) {
    UNUSED(p);
    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
        voice.begin();
        voice.say(
            "All your base are belong to us. You have no chance to survive make your time. ha. ha. ha. GOOD BYE. ");
        furi_hal_speaker_release();
    }
    return 0;
}

extern "C" int32_t sam_app_yes(void* p) {
    UNUSED(p);
    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
        voice.begin();
        voice.say("Yes");
        furi_hal_speaker_release();
    }
    return 0;
}

extern "C" int32_t sam_app_no(void* p) {
    UNUSED(p);
    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
        voice.begin();
        voice.say("No");
        furi_hal_speaker_release();
    }
    return 0;
}

extern "C" int32_t sam_app_wtf(void* p) {
    UNUSED(p);
    if(furi_hal_speaker_is_mine() || furi_hal_speaker_acquire(30)) {
        voice.begin();
        voice.say("What The Fuck");
        furi_hal_speaker_release();
    }
    return 0;
}