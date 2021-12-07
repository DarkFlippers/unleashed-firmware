#include "desktop/helpers/desktop_animation.h"
#include "assets_icons.h"
#include "desktop_animation_i.h"
#include "cmsis_os2.h"
#include "furi/common_defines.h"
#include "furi/record.h"
#include "storage/filesystem-api-defines.h"
#include <power/power_service/power.h>
#include <m-list.h>
#include <storage/storage.h>
#include <desktop/desktop.h>
#include <dolphin/dolphin.h>

#define KEEP_ONLY_CALM_BASIC_ANIMATIONS 1

LIST_DEF(AnimationList, const PairedAnimation*, M_PTR_OPLIST)
#define M_OPL_AnimationList_t() LIST_OPLIST(AnimationList)

#define PUSH_BACK_ANIMATIONS(listname, animations, butthurt)                          \
    for(int i = 0; i < COUNT_OF(animations); ++i) {                                   \
        if(!(animations)[i].basic->butthurt_level_mask ||                             \
           ((animations)[i].basic->butthurt_level_mask & BUTTHURT_LEVEL(butthurt))) { \
            AnimationList_push_back(animation_list, &(animations)[i]);                \
        }                                                                             \
    }

#define IS_BLOCKING_ANIMATION(x) \
    (((x) != DesktopAnimationStateBasic) && ((x) != DesktopAnimationStateActive))
#define IS_ONESHOT_ANIMATION(x) ((x) == DesktopAnimationStateLevelUpIsPending)

static void desktop_animation_timer_callback(void* context);

struct DesktopAnimation {
    bool sd_shown_error_db;
    bool sd_shown_error_card_bad;
    osTimerId_t timer;
    const PairedAnimation* current;
    const Icon* current_blocking_icon;
    const Icon** current_one_shot_icons;
    uint8_t one_shot_animation_counter;
    uint8_t one_shot_animation_size;
    DesktopAnimationState state;
    TickType_t basic_started_at;
    TickType_t active_finished_at;
    AnimationChangedCallback animation_changed_callback;
    void* animation_changed_callback_context;
};

DesktopAnimation* desktop_animation_alloc(void) {
    DesktopAnimation* animation = furi_alloc(sizeof(DesktopAnimation));

    animation->timer = osTimerNew(
        desktop_animation_timer_callback, osTimerPeriodic /* osTimerOnce */, animation, NULL);
    animation->active_finished_at = (TickType_t)(-30);
    animation->basic_started_at = 0;
    animation->animation_changed_callback = NULL;
    animation->animation_changed_callback_context = NULL;
    desktop_start_new_idle_animation(animation);

    return animation;
}

void desktop_animation_free(DesktopAnimation* animation) {
    furi_assert(animation);

    osTimerDelete(animation->timer);
    free(animation);
}

void desktop_animation_set_animation_changed_callback(
    DesktopAnimation* animation,
    AnimationChangedCallback callback,
    void* context) {
    furi_assert(animation);

    animation->animation_changed_callback = callback;
    animation->animation_changed_callback_context = context;
}

void desktop_start_new_idle_animation(DesktopAnimation* animation) {
    Dolphin* dolphin = furi_record_open("dolphin");
    DolphinStats stats = dolphin_stats(dolphin);
    furi_record_close("dolphin");

    furi_assert((stats.level >= 1) && (stats.level <= 3));

    AnimationList_t animation_list;
    AnimationList_init(animation_list);

#if KEEP_ONLY_CALM_BASIC_ANIMATIONS
    PUSH_BACK_ANIMATIONS(animation_list, calm_animation, 0);
#else
    PUSH_BACK_ANIMATIONS(animation_list, calm_animation, stats.butthurt);
    PUSH_BACK_ANIMATIONS(animation_list, mad_animation, stats.butthurt);
    switch(stats.level) {
    case 1:
        PUSH_BACK_ANIMATIONS(animation_list, level_1_animation, stats.butthurt);
        break;
    case 2:
        PUSH_BACK_ANIMATIONS(animation_list, level_2_animation, stats.butthurt);
        break;
    case 3:
        PUSH_BACK_ANIMATIONS(animation_list, level_3_animation, stats.butthurt);
        break;
    default:
        furi_crash("Dolphin level is out of bounds");
    }

    Power* power = furi_record_open("power");
    PowerInfo info;
    power_get_info(power, &info);

    if(!power_is_battery_well(&info)) {
        PUSH_BACK_ANIMATIONS(animation_list, check_battery_animation, stats.butthurt);
    }

    Storage* storage = furi_record_open("storage");
    FS_Error sd_status = storage_sd_status(storage);
    animation->current = NULL;

    if(sd_status == FSE_NOT_READY) {
        PUSH_BACK_ANIMATIONS(animation_list, no_sd_animation, stats.butthurt);
        animation->sd_shown_error_card_bad = false;
        animation->sd_shown_error_db = false;
    }
#endif

    uint32_t whole_weight = 0;
    for
        M_EACH(item, animation_list, AnimationList_t) {
            whole_weight += (*item)->basic->weight;
        }

    uint32_t lucky_number = random() % whole_weight;
    uint32_t weight = 0;

    const PairedAnimation* selected = NULL;
    for
        M_EACH(item, animation_list, AnimationList_t) {
            if(lucky_number < weight) {
                break;
            }
            weight += (*item)->basic->weight;
            selected = *item;
        }
    animation->basic_started_at = osKernelGetTickCount();
    animation->current = selected;
    osTimerStart(animation->timer, animation->current->basic->duration * 1000);
    animation->state = DesktopAnimationStateBasic;
    furi_assert(selected);
    AnimationList_clear(animation_list);
}

static void desktop_animation_timer_callback(void* context) {
    furi_assert(context);
    DesktopAnimation* animation = context;
    TickType_t now_ms = osKernelGetTickCount();
    AnimationList_t animation_list;
    AnimationList_init(animation_list);
    bool new_basic_animation = false;

    if(animation->state == DesktopAnimationStateActive) {
        animation->state = DesktopAnimationStateBasic;
        TickType_t basic_lasts_ms = now_ms - animation->basic_started_at;
        animation->active_finished_at = now_ms;
        TickType_t basic_duration_ms = animation->current->basic->duration * 1000;
        if(basic_lasts_ms > basic_duration_ms) {
            // if active animation finished, and basic duration came to an end
            // select new idle animation
            new_basic_animation = true;
        } else {
            // if active animation finished, but basic duration is not finished
            // play current animation for the rest of time
            furi_assert(basic_duration_ms != basic_lasts_ms);
            osTimerStart(animation->timer, basic_duration_ms - basic_lasts_ms);
        }
    } else if(animation->state == DesktopAnimationStateBasic) {
        // if basic animation finished
        // select new idle animation
        new_basic_animation = true;
    }

    if(new_basic_animation) {
        animation->basic_started_at = now_ms;
        desktop_start_new_idle_animation(animation);
    }

    // for oneshot generate events every time
    if(animation->animation_changed_callback) {
        animation->animation_changed_callback(animation->animation_changed_callback_context);
    }
}

void desktop_animation_activate(DesktopAnimation* animation) {
    furi_assert(animation);

    if(animation->state != DesktopAnimationStateBasic) {
        return;
    }

    if(animation->state == DesktopAnimationStateActive) {
        return;
    }

    if(!animation->current->active) {
        return;
    }

    TickType_t now = osKernelGetTickCount();
    TickType_t time_since_last_active = now - animation->active_finished_at;

    if(time_since_last_active > (animation->current->basic->active_cooldown * 1000)) {
        animation->state = DesktopAnimationStateActive;
        furi_assert(animation->current->active->duration > 0);
        osTimerStart(animation->timer, animation->current->active->duration * 1000);
        if(animation->animation_changed_callback) {
            animation->animation_changed_callback(animation->animation_changed_callback_context);
        }
    }
}

static const Icon* desktop_animation_get_current_idle_animation(
    DesktopAnimation* animation,
    bool* status_bar_background_black) {
    const ActiveAnimation* active = animation->current->active;
    const BasicAnimation* basic = animation->current->basic;
    if(animation->state == DesktopAnimationStateActive && active->icon) {
        *status_bar_background_black = active->black_status_bar;
        return active->icon;
    } else {
        *status_bar_background_black = basic->black_status_bar;
        return basic->icon;
    }
}

// Every time somebody starts 'desktop_animation_get_animation()'
// 1) check if there is a new level
// 2) check if there is SD card corruption
// 3) check if the SD card is empty
// 4) if all false - get idle animation

const Icon* desktop_animation_get_animation(
    DesktopAnimation* animation,
    bool* status_bar_background_black) {
    Dolphin* dolphin = furi_record_open("dolphin");
    Storage* storage = furi_record_open("storage");
    const Icon* icon = NULL;
    furi_assert(animation);
    FS_Error sd_status = storage_sd_status(storage);

    if(IS_BLOCKING_ANIMATION(animation->state)) {
        // don't give new animation till blocked animation
        // is reseted
        icon = animation->current_blocking_icon;
    }

    if(!icon) {
        if(sd_status == FSE_INTERNAL) {
            osTimerStop(animation->timer);
            icon = &A_CardBad_128x51;
            animation->current_blocking_icon = icon;
            animation->state = DesktopAnimationStateSDCorrupted;
            animation->sd_shown_error_card_bad = true;
            animation->sd_shown_error_db = false;
        } else if(sd_status == FSE_NOT_READY) {
            animation->sd_shown_error_card_bad = false;
            animation->sd_shown_error_db = false;
        } else if(sd_status == FSE_OK) {
            bool db_exists = storage_common_stat(storage, "/ext/manifest.txt", NULL) == FSE_OK;
            if(db_exists && !animation->sd_shown_error_db) {
                osTimerStop(animation->timer);
                icon = &A_CardNoDB_128x51;
                animation->current_blocking_icon = icon;
                animation->state = DesktopAnimationStateSDEmpty;
                animation->sd_shown_error_db = true;
            }
        }
    }

    DolphinStats stats = dolphin_stats(dolphin);
    if(!icon && stats.level_up_is_pending) {
        osTimerStop(animation->timer);
        icon = &A_LevelUpPending_128x51;
        animation->current_blocking_icon = icon;
        animation->state = DesktopAnimationStateLevelUpIsPending;
    }

    if(!icon) {
        icon =
            desktop_animation_get_current_idle_animation(animation, status_bar_background_black);
    } else {
        status_bar_background_black = false;
    }

    furi_record_close("storage");
    furi_record_close("dolphin");

    return icon;
}

DesktopAnimationState desktop_animation_handle_right(DesktopAnimation* animation) {
    furi_assert(animation);

    bool reset_animation = false;
    bool update_animation = false;

    switch(animation->state) {
    case DesktopAnimationStateActive:
    case DesktopAnimationStateBasic:
        /* nothing */
        break;
    case DesktopAnimationStateLevelUpIsPending:
        /* do nothing, main scene should change itself */
        break;
    case DesktopAnimationStateSDCorrupted:
        reset_animation = true;
        break;
    case DesktopAnimationStateSDEmpty:
        animation->state = DesktopAnimationStateSDEmptyURL;
        animation->current_blocking_icon = &A_CardNoDBUrl_128x51;
        update_animation = true;
        break;
    case DesktopAnimationStateSDEmptyURL:
        reset_animation = true;
        break;
    default:
        furi_crash("Unhandled desktop animation state");
    }

    if(reset_animation) {
        desktop_start_new_idle_animation(animation);
        update_animation = true;
    }

    if(update_animation) {
        if(animation->animation_changed_callback) {
            animation->animation_changed_callback(animation->animation_changed_callback_context);
        }
    }

    return animation->state;
}

#define LEVELUP_FRAME_RATE (0.2)

void desktop_animation_start_oneshot_levelup(DesktopAnimation* animation) {
    animation->one_shot_animation_counter = 0;
    animation->state = DesktopAnimationStateLevelUpIsPending;

    Dolphin* dolphin = furi_record_open("dolphin");
    DolphinStats stats = dolphin_stats(dolphin);
    furi_record_close("dolphin");
    furi_assert(stats.level_up_is_pending);
    if(stats.level == 1) {
        animation->current_one_shot_icons = animation_level2up;
        animation->one_shot_animation_size = COUNT_OF(animation_level2up);
    } else if(stats.level == 2) {
        animation->current_one_shot_icons = animation_level3up;
        animation->one_shot_animation_size = COUNT_OF(animation_level3up);
    } else {
        furi_crash("Dolphin level is out of bounds");
    }
    osTimerStart(animation->timer, LEVELUP_FRAME_RATE * 1000);
}

const Icon* desktop_animation_get_oneshot_frame(DesktopAnimation* animation) {
    furi_assert(IS_ONESHOT_ANIMATION(animation->state));
    furi_assert(animation->one_shot_animation_size > 0);
    const Icon* icon = NULL;

    if(animation->one_shot_animation_counter < animation->one_shot_animation_size) {
        icon = animation->current_one_shot_icons[animation->one_shot_animation_counter];
        ++animation->one_shot_animation_counter;
    } else {
        animation->state = DesktopAnimationStateBasic;
        animation->one_shot_animation_size = 0;
        osTimerStop(animation->timer);
        icon = NULL;
    }

    return icon;
}
