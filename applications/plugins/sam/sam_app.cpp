#include <furi.h>
#include "stm32_sam.h"
// WOULD BE COOL IF SOMEONE MADE A TEXT ENTRY SCREEN TO HAVE IT READ WHAT IS ENTERED TO TEXT
STM32SAM voice;

extern "C" int32_t sam_app(void* p) {
    UNUSED(p);

    voice.begin();
    voice.say(
        "All your base are belong to us. You have no chance to survive make your time. ha. ha. ha. GOOD BYE. ");

    return 0;
}

extern "C" int32_t sam_app_yes(void* p) {
    UNUSED(p);

    voice.begin();
    voice.say("Yes");

    return 0;
}

extern "C" int32_t sam_app_no(void* p) {
    UNUSED(p);

    voice.begin();
    voice.say("No");

    return 0;
}