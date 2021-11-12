#include "desktop_animation.h"

#define TAG "DesktopAnimation"

static const Icon* idle_scenes[] = {&A_Wink_128x64, &A_WatchingTV_128x64};

const Icon* desktop_get_icon() {
    uint8_t new = 0;

#if 0
    // checking dolphin state here to choose appropriate animation

    Dolphin* dolphin = furi_record_open("dolphin");
    DolphinStats stats = dolphin_stats(dolphin);
    float timediff = fabs(difftime(stats.timestamp, dolphin_state_timestamp()));

    FURI_LOG_I(TAG, "background change");
    FURI_LOG_I(TAG, "icounter: %d", stats.icounter);
    FURI_LOG_I(TAG, "butthurt: %d", stats.butthurt);
    FURI_LOG_I(TAG, "time since deeed: %.0f", timediff);
#endif

    if((random() % 100) > 50) { // temp rnd selection
        new = random() % COUNT_OF(idle_scenes);
    }

    return idle_scenes[new];
}
