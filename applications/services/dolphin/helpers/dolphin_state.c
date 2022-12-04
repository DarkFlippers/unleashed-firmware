#include "dolphin_state.h"
#include "dolphin/helpers/dolphin_deed.h"
#include "dolphin_state_filename.h"

#include <stdint.h>
#include <storage/storage.h>
#include <furi.h>
#include <furi_hal.h>
#include <math.h>
#include <toolbox/saved_struct.h>

#define TAG "DolphinState"

#define DOLPHIN_STATE_PATH INT_PATH(DOLPHIN_STATE_FILE_NAME)
#define DOLPHIN_STATE_HEADER_MAGIC 0xD0
#define DOLPHIN_STATE_HEADER_VERSION 0x01
#define LEVEL2_THRESHOLD 450
#define LEVEL3_THRESHOLD 700
#define LEVEL4_THRESHOLD 1100
#define LEVEL5_THRESHOLD 1800
#define LEVEL6_THRESHOLD 2300
#define LEVEL7_THRESHOLD 2900
#define LEVEL8_THRESHOLD 3900
#define LEVEL9_THRESHOLD 5000
#define LEVEL10_THRESHOLD 5900
#define LEVEL11_THRESHOLD 7200
#define LEVEL12_THRESHOLD 8400
#define LEVEL13_THRESHOLD 10000
#define LEVEL14_THRESHOLD 11500
#define LEVEL15_THRESHOLD 13000
#define LEVEL16_THRESHOLD 15000
#define LEVEL17_THRESHOLD 18000
#define LEVEL18_THRESHOLD 20000
#define LEVEL19_THRESHOLD 22000
#define LEVEL20_THRESHOLD 25000
#define LEVEL21_THRESHOLD 33000
#define LEVEL22_THRESHOLD 41000
#define LEVEL23_THRESHOLD 50000
#define LEVEL24_THRESHOLD 62000
#define LEVEL25_THRESHOLD 75000
#define LEVEL26_THRESHOLD 90000
#define LEVEL27_THRESHOLD 105000
#define LEVEL28_THRESHOLD 120000
#define LEVEL29_THRESHOLD 135000
#define LEVEL30_THRESHOLD 155000
#define BUTTHURT_MAX 14
#define BUTTHURT_MIN 0

DolphinState* dolphin_state_alloc() {
    return malloc(sizeof(DolphinState));
}

void dolphin_state_free(DolphinState* dolphin_state) {
    free(dolphin_state);
}

bool dolphin_state_save(DolphinState* dolphin_state) {
    if(!dolphin_state->dirty) {
        return true;
    }

    bool result = saved_struct_save(
        DOLPHIN_STATE_PATH,
        &dolphin_state->data,
        sizeof(DolphinStoreData),
        DOLPHIN_STATE_HEADER_MAGIC,
        DOLPHIN_STATE_HEADER_VERSION);

    if(result) {
        FURI_LOG_I(TAG, "State saved");
        dolphin_state->dirty = false;
    } else {
        FURI_LOG_E(TAG, "Failed to save state");
    }

    return result;
}

bool dolphin_state_load(DolphinState* dolphin_state) {
    bool success = saved_struct_load(
        DOLPHIN_STATE_PATH,
        &dolphin_state->data,
        sizeof(DolphinStoreData),
        DOLPHIN_STATE_HEADER_MAGIC,
        DOLPHIN_STATE_HEADER_VERSION);

    if(success) {
        if((dolphin_state->data.butthurt > BUTTHURT_MAX) ||
           (dolphin_state->data.butthurt < BUTTHURT_MIN)) {
            success = false;
        }
    }

    if(!success) {
        FURI_LOG_W(TAG, "Reset dolphin-state");
        memset(dolphin_state, 0, sizeof(*dolphin_state));
        dolphin_state->dirty = true;
    }

    return success;
}

uint64_t dolphin_state_timestamp() {
    FuriHalRtcDateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    return furi_hal_rtc_datetime_to_timestamp(&datetime);
}

bool dolphin_state_is_levelup(uint32_t icounter) {
    return (icounter == LEVEL2_THRESHOLD) || (icounter == LEVEL3_THRESHOLD) ||
           (icounter == LEVEL4_THRESHOLD) || (icounter == LEVEL5_THRESHOLD) ||
           (icounter == LEVEL6_THRESHOLD) || (icounter == LEVEL7_THRESHOLD) ||
           (icounter == LEVEL8_THRESHOLD) || (icounter == LEVEL9_THRESHOLD) ||
           (icounter == LEVEL10_THRESHOLD) || (icounter == LEVEL11_THRESHOLD) ||
           (icounter == LEVEL12_THRESHOLD) || (icounter == LEVEL13_THRESHOLD) ||
           (icounter == LEVEL14_THRESHOLD) || (icounter == LEVEL15_THRESHOLD) ||
           (icounter == LEVEL16_THRESHOLD) || (icounter == LEVEL17_THRESHOLD) ||
           (icounter == LEVEL18_THRESHOLD) || (icounter == LEVEL19_THRESHOLD) ||
           (icounter == LEVEL20_THRESHOLD) || (icounter == LEVEL21_THRESHOLD) ||
           (icounter == LEVEL22_THRESHOLD) || (icounter == LEVEL23_THRESHOLD) ||
           (icounter == LEVEL24_THRESHOLD) || (icounter == LEVEL25_THRESHOLD) ||
           (icounter == LEVEL26_THRESHOLD) || (icounter == LEVEL27_THRESHOLD) ||
           (icounter == LEVEL28_THRESHOLD) || (icounter == LEVEL29_THRESHOLD) ||
           (icounter == LEVEL30_THRESHOLD);
}

uint8_t dolphin_get_level(uint32_t icounter) {
    if(icounter <= LEVEL2_THRESHOLD) {
        return 1;
    } else if(icounter <= LEVEL3_THRESHOLD) {
        return 2;
    } else if(icounter <= LEVEL4_THRESHOLD) {
        return 3;
    } else if(icounter <= LEVEL5_THRESHOLD) {
        return 4;
    } else if(icounter <= LEVEL6_THRESHOLD) {
        return 5;
    } else if(icounter <= LEVEL7_THRESHOLD) {
        return 6;
    } else if(icounter <= LEVEL8_THRESHOLD) {
        return 7;
    } else if(icounter <= LEVEL9_THRESHOLD) {
        return 8;
    } else if(icounter <= LEVEL10_THRESHOLD) {
        return 9;
    } else if(icounter <= LEVEL11_THRESHOLD) {
        return 10;
    } else if(icounter <= LEVEL12_THRESHOLD) {
        return 11;
    } else if(icounter <= LEVEL13_THRESHOLD) {
        return 12;
    } else if(icounter <= LEVEL14_THRESHOLD) {
        return 13;
    } else if(icounter <= LEVEL15_THRESHOLD) {
        return 14;
    } else if(icounter <= LEVEL16_THRESHOLD) {
        return 15;
    } else if(icounter <= LEVEL17_THRESHOLD) {
        return 16;
    } else if(icounter <= LEVEL18_THRESHOLD) {
        return 16;
    } else if(icounter <= LEVEL19_THRESHOLD) {
        return 18;
    } else if(icounter <= LEVEL20_THRESHOLD) {
        return 19;
    } else if(icounter <= LEVEL21_THRESHOLD) {
        return 20;
    } else if(icounter <= LEVEL22_THRESHOLD) {
        return 21;
    } else if(icounter <= LEVEL23_THRESHOLD) {
        return 22;
    } else if(icounter <= LEVEL24_THRESHOLD) {
        return 23;
    } else if(icounter <= LEVEL25_THRESHOLD) {
        return 24;
    } else if(icounter <= LEVEL26_THRESHOLD) {
        return 25;
    } else if(icounter <= LEVEL27_THRESHOLD) {
        return 26;
    } else if(icounter <= LEVEL28_THRESHOLD) {
        return 27;
    } else if(icounter <= LEVEL29_THRESHOLD) {
        return 28;
    } else if(icounter <= LEVEL30_THRESHOLD) {
        return 29;
    } else {
        return 30;
    }
}

uint32_t dolphin_state_xp_above_last_levelup(uint32_t icounter) {
    uint32_t threshold = 0;
    if(icounter <= LEVEL2_THRESHOLD) {
        threshold = 0;
    } else if(icounter <= LEVEL3_THRESHOLD) {
        threshold = LEVEL2_THRESHOLD + 1;
    } else if(icounter <= LEVEL4_THRESHOLD) {
        threshold = LEVEL3_THRESHOLD + 1;
    } else if(icounter <= LEVEL5_THRESHOLD) {
        threshold = LEVEL4_THRESHOLD + 1;
    } else if(icounter <= LEVEL6_THRESHOLD) {
        threshold = LEVEL5_THRESHOLD + 1;
    } else if(icounter <= LEVEL7_THRESHOLD) {
        threshold = LEVEL6_THRESHOLD + 1;
    } else if(icounter <= LEVEL8_THRESHOLD) {
        threshold = LEVEL7_THRESHOLD + 1;
    } else if(icounter <= LEVEL9_THRESHOLD) {
        threshold = LEVEL8_THRESHOLD + 1;
    } else if(icounter <= LEVEL10_THRESHOLD) {
        threshold = LEVEL9_THRESHOLD + 1;
    } else if(icounter <= LEVEL11_THRESHOLD) {
        threshold = LEVEL10_THRESHOLD + 1;
    } else if(icounter <= LEVEL12_THRESHOLD) {
        threshold = LEVEL11_THRESHOLD + 1;
    } else if(icounter <= LEVEL13_THRESHOLD) {
        threshold = LEVEL12_THRESHOLD + 1;
    } else if(icounter <= LEVEL14_THRESHOLD) {
        threshold = LEVEL13_THRESHOLD + 1;
    } else if(icounter <= LEVEL15_THRESHOLD) {
        threshold = LEVEL14_THRESHOLD + 1;
    } else if(icounter <= LEVEL16_THRESHOLD) {
        threshold = LEVEL15_THRESHOLD + 1;
    } else if(icounter <= LEVEL17_THRESHOLD) {
        threshold = LEVEL16_THRESHOLD + 1;
    } else if(icounter <= LEVEL18_THRESHOLD) {
        threshold = LEVEL17_THRESHOLD + 1;
    } else if(icounter <= LEVEL19_THRESHOLD) {
        threshold = LEVEL18_THRESHOLD + 1;
    } else if(icounter <= LEVEL20_THRESHOLD) {
        threshold = LEVEL19_THRESHOLD + 1;
    } else if(icounter <= LEVEL21_THRESHOLD) {
        threshold = LEVEL20_THRESHOLD + 1;
    } else if(icounter <= LEVEL22_THRESHOLD) {
        threshold = LEVEL21_THRESHOLD + 1;
    } else if(icounter <= LEVEL23_THRESHOLD) {
        threshold = LEVEL22_THRESHOLD + 1;
    } else if(icounter <= LEVEL24_THRESHOLD) {
        threshold = LEVEL23_THRESHOLD + 1;
    } else if(icounter <= LEVEL25_THRESHOLD) {
        threshold = LEVEL24_THRESHOLD + 1;
    } else if(icounter <= LEVEL26_THRESHOLD) {
        threshold = LEVEL25_THRESHOLD + 1;
    } else if(icounter <= LEVEL27_THRESHOLD) {
        threshold = LEVEL26_THRESHOLD + 1;
    } else if(icounter <= LEVEL28_THRESHOLD) {
        threshold = LEVEL27_THRESHOLD + 1;
    } else if(icounter <= LEVEL29_THRESHOLD) {
        threshold = LEVEL28_THRESHOLD + 1;
    } else if(icounter <= LEVEL30_THRESHOLD) {
        threshold = LEVEL29_THRESHOLD + 1;
    } else {
        threshold = LEVEL30_THRESHOLD + 1;
    }
    return icounter - threshold;
}

uint32_t dolphin_state_xp_to_levelup(uint32_t icounter) {
    uint32_t threshold = 0;
    if(icounter <= LEVEL2_THRESHOLD) {
        threshold = LEVEL2_THRESHOLD;
    } else if(icounter <= LEVEL3_THRESHOLD) {
        threshold = LEVEL3_THRESHOLD;
    } else if(icounter <= LEVEL4_THRESHOLD) {
        threshold = LEVEL4_THRESHOLD;
    } else if(icounter <= LEVEL5_THRESHOLD) {
        threshold = LEVEL5_THRESHOLD;
    } else if(icounter <= LEVEL6_THRESHOLD) {
        threshold = LEVEL6_THRESHOLD;
    } else if(icounter <= LEVEL7_THRESHOLD) {
        threshold = LEVEL7_THRESHOLD;
    } else if(icounter <= LEVEL8_THRESHOLD) {
        threshold = LEVEL8_THRESHOLD;
    } else if(icounter <= LEVEL9_THRESHOLD) {
        threshold = LEVEL9_THRESHOLD;
    } else if(icounter <= LEVEL10_THRESHOLD) {
        threshold = LEVEL10_THRESHOLD;
    } else if(icounter <= LEVEL11_THRESHOLD) {
        threshold = LEVEL11_THRESHOLD;
    } else if(icounter <= LEVEL12_THRESHOLD) {
        threshold = LEVEL12_THRESHOLD;
    } else if(icounter <= LEVEL13_THRESHOLD) {
        threshold = LEVEL13_THRESHOLD;
    } else if(icounter <= LEVEL14_THRESHOLD) {
        threshold = LEVEL14_THRESHOLD;
    } else if(icounter <= LEVEL15_THRESHOLD) {
        threshold = LEVEL15_THRESHOLD;
    } else if(icounter <= LEVEL16_THRESHOLD) {
        threshold = LEVEL16_THRESHOLD;
    } else if(icounter <= LEVEL17_THRESHOLD) {
        threshold = LEVEL17_THRESHOLD;
    } else if(icounter <= LEVEL18_THRESHOLD) {
        threshold = LEVEL18_THRESHOLD;
    } else if(icounter <= LEVEL19_THRESHOLD) {
        threshold = LEVEL19_THRESHOLD;
    } else if(icounter <= LEVEL20_THRESHOLD) {
        threshold = LEVEL20_THRESHOLD;
    } else if(icounter <= LEVEL21_THRESHOLD) {
        threshold = LEVEL21_THRESHOLD;
    } else if(icounter <= LEVEL22_THRESHOLD) {
        threshold = LEVEL22_THRESHOLD;
    } else if(icounter <= LEVEL23_THRESHOLD) {
        threshold = LEVEL23_THRESHOLD;
    } else if(icounter <= LEVEL24_THRESHOLD) {
        threshold = LEVEL24_THRESHOLD;
    } else if(icounter <= LEVEL25_THRESHOLD) {
        threshold = LEVEL25_THRESHOLD;
    } else if(icounter <= LEVEL26_THRESHOLD) {
        threshold = LEVEL26_THRESHOLD;
    } else if(icounter <= LEVEL27_THRESHOLD) {
        threshold = LEVEL27_THRESHOLD;
    } else if(icounter <= LEVEL28_THRESHOLD) {
        threshold = LEVEL28_THRESHOLD;
    } else if(icounter <= LEVEL29_THRESHOLD) {
        threshold = LEVEL29_THRESHOLD;
    } else if(icounter <= LEVEL30_THRESHOLD) {
        threshold = LEVEL30_THRESHOLD;
    } else {
        threshold = (uint32_t)-1;
    }
    return threshold - icounter;
}

void dolphin_state_on_deed(DolphinState* dolphin_state, DolphinDeed deed) {
    // Special case for testing
    if(deed > DolphinDeedMAX) {
        if(deed == DolphinDeedTestLeft) {
            dolphin_state->data.butthurt =
                CLAMP(dolphin_state->data.butthurt + 1, BUTTHURT_MAX, BUTTHURT_MIN);
            if(dolphin_state->data.icounter > 0) dolphin_state->data.icounter--;
            dolphin_state->data.timestamp = dolphin_state_timestamp();
            dolphin_state->dirty = true;
        } else if(deed == DolphinDeedTestRight) {
            dolphin_state->data.butthurt = BUTTHURT_MIN;
            if(dolphin_state->data.icounter < UINT32_MAX) dolphin_state->data.icounter++;
            dolphin_state->data.timestamp = dolphin_state_timestamp();
            dolphin_state->dirty = true;
        }
        return;
    }

    DolphinApp app = dolphin_deed_get_app(deed);
    int8_t weight_limit =
        dolphin_deed_get_app_limit(app) - dolphin_state->data.icounter_daily_limit[app];
    uint8_t deed_weight = CLAMP(dolphin_deed_get_weight(deed), weight_limit, 0);

    uint32_t xp_to_levelup = dolphin_state_xp_to_levelup(dolphin_state->data.icounter);
    if(xp_to_levelup) {
        deed_weight = MIN(xp_to_levelup, deed_weight);
        dolphin_state->data.icounter += deed_weight;
        dolphin_state->data.icounter_daily_limit[app] += deed_weight;
    }

    /* decrease butthurt:
     * 0 deeds accumulating --> 0 butthurt
     * +1....+15 deeds accumulating --> -1 butthurt
     * +16...+30 deeds accumulating --> -1 butthurt
     * +31...+45 deeds accumulating --> -1 butthurt
     * +46...... deeds accumulating --> -1 butthurt
     * -4 butthurt per day is maximum
     * */
    uint8_t butthurt_icounter_level_old = dolphin_state->data.butthurt_daily_limit / 15 +
                                          !!(dolphin_state->data.butthurt_daily_limit % 15);
    dolphin_state->data.butthurt_daily_limit =
        CLAMP(dolphin_state->data.butthurt_daily_limit + deed_weight, 46, 0);
    uint8_t butthurt_icounter_level_new = dolphin_state->data.butthurt_daily_limit / 15 +
                                          !!(dolphin_state->data.butthurt_daily_limit % 15);
    int32_t new_butthurt = ((int32_t)dolphin_state->data.butthurt) -
                           (butthurt_icounter_level_old != butthurt_icounter_level_new);
    new_butthurt = CLAMP(new_butthurt, BUTTHURT_MAX, BUTTHURT_MIN);
    if(new_butthurt >= 7) new_butthurt = BUTTHURT_MIN; // FLIPPER STAYS HAPPY
    dolphin_state->data.butthurt = new_butthurt;
    dolphin_state->data.timestamp = dolphin_state_timestamp();
    dolphin_state->dirty = true;

    FURI_LOG_D(
        TAG,
        "icounter %ld, butthurt %ld",
        dolphin_state->data.icounter,
        dolphin_state->data.butthurt);
}

void dolphin_state_butthurted(DolphinState* dolphin_state) {
    if(dolphin_state->data.butthurt < BUTTHURT_MAX) {
        dolphin_state->data.butthurt++;
        dolphin_state->data.timestamp = dolphin_state_timestamp();
        dolphin_state->dirty = true;
    }
}

void dolphin_state_increase_level(DolphinState* dolphin_state) {
    furi_assert(dolphin_state_is_levelup(dolphin_state->data.icounter));
    ++dolphin_state->data.icounter;
    dolphin_state->dirty = true;
}

void dolphin_state_clear_limits(DolphinState* dolphin_state) {
    furi_assert(dolphin_state);

    for(int i = 0; i < DolphinAppMAX; ++i) {
        dolphin_state->data.icounter_daily_limit[i] = 0;
    }
    dolphin_state->data.butthurt_daily_limit = 0;
    dolphin_state->dirty = true;
}
