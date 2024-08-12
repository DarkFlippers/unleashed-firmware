#include <gui/view_stack.h>
#include <stdint.h>
#include <furi.h>
#include <furi_hal.h>
#include <dolphin/dolphin.h>
#include <power/power_service/power.h>
#include <storage/storage.h>
#include <assets_icons.h>

#include "views/bubble_animation_view.h"
#include "views/one_shot_animation_view.h"
#include "animation_storage.h"
#include "animation_manager.h"

#define TAG "AnimationManager"

#define HARDCODED_ANIMATION_NAME   "L1_Tv_128x47"
#define NO_SD_ANIMATION_NAME       "L1_NoSd_128x49"
#define BAD_BATTERY_ANIMATION_NAME "L1_BadBattery_128x47"

#define NO_DB_ANIMATION_NAME    "L0_NoDb_128x51"
#define BAD_SD_ANIMATION_NAME   "L0_SdBad_128x51"
#define SD_OK_ANIMATION_NAME    "L0_SdOk_128x51"
#define URL_ANIMATION_NAME      "L0_Url_128x51"
#define NEW_MAIL_ANIMATION_NAME "L0_NewMail_128x51"

typedef enum {
    AnimationManagerStateIdle,
    AnimationManagerStateBlocked,
    AnimationManagerStateFreezedIdle,
    AnimationManagerStateFreezedBlocked,
} AnimationManagerState;

struct AnimationManager {
    AnimationManagerState state;
    FuriPubSubSubscription* pubsub_subscription_storage;
    FuriPubSubSubscription* pubsub_subscription_dolphin;
    BubbleAnimationView* animation_view;
    OneShotView* one_shot_view;
    FuriTimer* idle_animation_timer;
    StorageAnimation* current_animation;
    AnimationManagerInteractCallback interact_callback;
    AnimationManagerSetNewIdleAnimationCallback new_idle_callback;
    AnimationManagerSetNewIdleAnimationCallback check_blocking_callback;
    void* context;
    FuriString* freezed_animation_name;
    int32_t freezed_animation_time_left;
    ViewStack* view_stack;

    bool dummy_mode            : 1;
    bool blocking_shown_url    : 1;
    bool blocking_shown_sd_bad : 1;
    bool blocking_shown_no_db  : 1;
    bool blocking_shown_sd_ok  : 1;
    bool levelup_pending       : 1;
    bool levelup_active        : 1;
};

static StorageAnimation*
    animation_manager_select_idle_animation(AnimationManager* animation_manager);
static void animation_manager_replace_current_animation(
    AnimationManager* animation_manager,
    StorageAnimation* storage_animation);
static void animation_manager_start_new_idle(AnimationManager* animation_manager);
static bool animation_manager_check_blocking(AnimationManager* animation_manager);
static bool animation_manager_is_valid_idle_animation(
    const StorageAnimationManifestInfo* info,
    const DolphinStats* stats);
static void animation_manager_switch_to_one_shot_view(AnimationManager* animation_manager);
static void animation_manager_switch_to_animation_view(AnimationManager* animation_manager);

void animation_manager_set_context(AnimationManager* animation_manager, void* context) {
    furi_assert(animation_manager);
    animation_manager->context = context;
}

void animation_manager_set_new_idle_callback(
    AnimationManager* animation_manager,
    AnimationManagerSetNewIdleAnimationCallback callback) {
    furi_assert(animation_manager);
    animation_manager->new_idle_callback = callback;
}

void animation_manager_set_check_callback(
    AnimationManager* animation_manager,
    AnimationManagerCheckBlockingCallback callback) {
    furi_assert(animation_manager);
    animation_manager->check_blocking_callback = callback;
}

void animation_manager_set_interact_callback(
    AnimationManager* animation_manager,
    AnimationManagerInteractCallback callback) {
    furi_assert(animation_manager);
    animation_manager->interact_callback = callback;
}

void animation_manager_set_dummy_mode_state(AnimationManager* animation_manager, bool enabled) {
    furi_assert(animation_manager);
    // Prevent change of animations if mode is the same
    if(animation_manager->dummy_mode != enabled) {
        animation_manager->dummy_mode = enabled;
        animation_manager_start_new_idle(animation_manager);
    }
}

static void animation_manager_storage_callback(const void* message, void* context) {
    const StorageEvent* storage_event = message;

    switch(storage_event->type) {
    case StorageEventTypeCardMount:
    case StorageEventTypeCardUnmount:
    case StorageEventTypeCardMountError:
        furi_assert(context);
        AnimationManager* animation_manager = context;
        if(animation_manager->check_blocking_callback) {
            animation_manager->check_blocking_callback(animation_manager->context);
        }
        break;

    default:
        break;
    }
}

static void animation_manager_dolphin_callback(const void* message, void* context) {
    const DolphinPubsubEvent* dolphin_event = message;

    switch(*dolphin_event) {
    case DolphinPubsubEventUpdate:
        furi_assert(context);
        AnimationManager* animation_manager = context;
        if(animation_manager->check_blocking_callback) {
            animation_manager->check_blocking_callback(animation_manager->context);
        }
        break;
    default:
        break;
    }
}

static void animation_manager_timer_callback(void* context) {
    furi_assert(context);
    AnimationManager* animation_manager = context;
    if(animation_manager->new_idle_callback) {
        animation_manager->new_idle_callback(animation_manager->context);
    }
}

static void animation_manager_interact_callback(void* context) {
    furi_assert(context);
    AnimationManager* animation_manager = context;
    if(animation_manager->interact_callback) {
        animation_manager->interact_callback(animation_manager->context);
    }
}

/* reaction to animation_manager->check_blocking_callback() */
void animation_manager_check_blocking_process(AnimationManager* animation_manager) {
    furi_assert(animation_manager);

    if(animation_manager->state == AnimationManagerStateIdle) {
        bool blocked = animation_manager_check_blocking(animation_manager);

        if(!blocked) {
            Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
            DolphinStats stats = dolphin_stats(dolphin);
            furi_record_close(RECORD_DOLPHIN);

            const StorageAnimationManifestInfo* manifest_info =
                animation_storage_get_meta(animation_manager->current_animation);
            bool valid = animation_manager_is_valid_idle_animation(manifest_info, &stats);

            if(!valid) {
                animation_manager_start_new_idle(animation_manager);
            }
        }
    }
}

/* reaction to animation_manager->new_idle_callback() */
void animation_manager_new_idle_process(AnimationManager* animation_manager) {
    furi_assert(animation_manager);

    if(animation_manager->state == AnimationManagerStateIdle) {
        animation_manager_start_new_idle(animation_manager);
    }
}

/* reaction to animation_manager->interact_callback() */
bool animation_manager_interact_process(AnimationManager* animation_manager) {
    furi_assert(animation_manager);
    bool consumed = true;

    if(animation_manager->levelup_pending) {
        animation_manager->levelup_pending = false;
        animation_manager->levelup_active = true;
        animation_manager_switch_to_one_shot_view(animation_manager);
        Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
        dolphin_upgrade_level(dolphin);
        furi_record_close(RECORD_DOLPHIN);
    } else if(animation_manager->levelup_active) {
        animation_manager->levelup_active = false;
        animation_manager_start_new_idle(animation_manager);
        animation_manager_switch_to_animation_view(animation_manager);
    } else if(animation_manager->state == AnimationManagerStateBlocked) {
        bool blocked = animation_manager_check_blocking(animation_manager);

        if(!blocked) {
            animation_manager_start_new_idle(animation_manager);
        }
    } else {
        consumed = false;
    }

    return consumed;
}

static void animation_manager_start_new_idle(AnimationManager* animation_manager) {
    furi_assert(animation_manager);

    StorageAnimation* new_animation = animation_manager_select_idle_animation(animation_manager);
    animation_manager_replace_current_animation(animation_manager, new_animation);
    const BubbleAnimation* bubble_animation =
        animation_storage_get_bubble_animation(animation_manager->current_animation);
    animation_manager->state = AnimationManagerStateIdle;
    furi_timer_start(animation_manager->idle_animation_timer, bubble_animation->duration * 1000);
}

static bool animation_manager_check_blocking(AnimationManager* animation_manager) {
    furi_assert(animation_manager);

    StorageAnimation* blocking_animation = NULL;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FS_Error sd_status = storage_sd_status(storage);

    if(sd_status == FSE_INTERNAL) {
        if(!animation_manager->blocking_shown_sd_bad) {
            blocking_animation = animation_storage_find_animation(BAD_SD_ANIMATION_NAME);
            furi_assert(blocking_animation);
            animation_manager->blocking_shown_sd_bad = true;
        }
    } else if(sd_status == FSE_NOT_READY) {
        animation_manager->blocking_shown_sd_bad = false;
        animation_manager->blocking_shown_sd_ok = false;
        animation_manager->blocking_shown_no_db = false;
    } else if(sd_status == FSE_OK) {
        if(!animation_manager->blocking_shown_sd_ok) {
            blocking_animation = animation_storage_find_animation(SD_OK_ANIMATION_NAME);
            furi_assert(blocking_animation);
            animation_manager->blocking_shown_sd_ok = true;
        } else if(!animation_manager->blocking_shown_no_db) {
            if(!storage_file_exists(storage, EXT_PATH("Manifest"))) {
                blocking_animation = animation_storage_find_animation(NO_DB_ANIMATION_NAME);
                furi_assert(blocking_animation);
                animation_manager->blocking_shown_no_db = true;
                animation_manager->blocking_shown_url = true;
            }
        } else if(animation_manager->blocking_shown_url) {
            blocking_animation = animation_storage_find_animation(URL_ANIMATION_NAME);
            furi_assert(blocking_animation);
            animation_manager->blocking_shown_url = false;
        }
    }

    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
    DolphinStats stats = dolphin_stats(dolphin);
    furi_record_close(RECORD_DOLPHIN);
    if(!blocking_animation && stats.level_up_is_pending) {
        blocking_animation = animation_storage_find_animation(NEW_MAIL_ANIMATION_NAME);
        furi_check(blocking_animation);
        animation_manager->levelup_pending = true;
    }

    if(blocking_animation) {
        furi_timer_stop(animation_manager->idle_animation_timer);
        animation_manager_replace_current_animation(animation_manager, blocking_animation);
        /* no timer starting because this is blocking animation */
        animation_manager->state = AnimationManagerStateBlocked;
    }

    furi_record_close(RECORD_STORAGE);

    return !!blocking_animation;
}

static void animation_manager_replace_current_animation(
    AnimationManager* animation_manager,
    StorageAnimation* storage_animation) {
    furi_assert(storage_animation);
    StorageAnimation* previous_animation = animation_manager->current_animation;

    const BubbleAnimation* animation = animation_storage_get_bubble_animation(storage_animation);
    bubble_animation_view_set_animation(animation_manager->animation_view, animation);
    const char* new_name = animation_storage_get_meta(storage_animation)->name;
    FURI_LOG_I(TAG, "Select \'%s\' animation", new_name);
    animation_manager->current_animation = storage_animation;

    if(previous_animation) {
        animation_storage_free_storage_animation(&previous_animation);
    }
}

AnimationManager* animation_manager_alloc(void) {
    AnimationManager* animation_manager = malloc(sizeof(AnimationManager));
    animation_manager->animation_view = bubble_animation_view_alloc();
    animation_manager->view_stack = view_stack_alloc();
    View* animation_view = bubble_animation_get_view(animation_manager->animation_view);
    view_stack_add_view(animation_manager->view_stack, animation_view);
    animation_manager->freezed_animation_name = furi_string_alloc();

    animation_manager->idle_animation_timer =
        furi_timer_alloc(animation_manager_timer_callback, FuriTimerTypeOnce, animation_manager);
    bubble_animation_view_set_interact_callback(
        animation_manager->animation_view, animation_manager_interact_callback, animation_manager);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    animation_manager->pubsub_subscription_storage = furi_pubsub_subscribe(
        storage_get_pubsub(storage), animation_manager_storage_callback, animation_manager);
    furi_record_close(RECORD_STORAGE);

    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
    animation_manager->pubsub_subscription_dolphin = furi_pubsub_subscribe(
        dolphin_get_pubsub(dolphin), animation_manager_dolphin_callback, animation_manager);
    furi_record_close(RECORD_DOLPHIN);

    animation_manager->blocking_shown_sd_ok = true;
    if(!animation_manager_check_blocking(animation_manager)) {
        animation_manager_start_new_idle(animation_manager);
    }

    return animation_manager;
}

void animation_manager_free(AnimationManager* animation_manager) {
    furi_assert(animation_manager);

    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
    furi_pubsub_unsubscribe(
        dolphin_get_pubsub(dolphin), animation_manager->pubsub_subscription_dolphin);
    furi_record_close(RECORD_DOLPHIN);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_pubsub_unsubscribe(
        storage_get_pubsub(storage), animation_manager->pubsub_subscription_storage);
    furi_record_close(RECORD_STORAGE);

    furi_string_free(animation_manager->freezed_animation_name);
    View* animation_view = bubble_animation_get_view(animation_manager->animation_view);
    view_stack_remove_view(animation_manager->view_stack, animation_view);
    bubble_animation_view_free(animation_manager->animation_view);
    furi_timer_free(animation_manager->idle_animation_timer);
}

View* animation_manager_get_animation_view(AnimationManager* animation_manager) {
    furi_assert(animation_manager);

    return view_stack_get_view(animation_manager->view_stack);
}

static bool animation_manager_is_valid_idle_animation(
    const StorageAnimationManifestInfo* info,
    const DolphinStats* stats) {
    furi_assert(info);
    furi_assert(info->name);

    bool result = true;

    if(!strcmp(info->name, BAD_BATTERY_ANIMATION_NAME)) {
        Power* power = furi_record_open(RECORD_POWER);
        bool battery_is_well = power_is_battery_healthy(power);
        furi_record_close(RECORD_POWER);

        result = !battery_is_well;
    }
    if(!strcmp(info->name, NO_SD_ANIMATION_NAME)) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        FS_Error sd_status = storage_sd_status(storage);
        furi_record_close(RECORD_STORAGE);

        result = (sd_status == FSE_NOT_READY);
    }
    if((stats->butthurt < info->min_butthurt) || (stats->butthurt > info->max_butthurt)) {
        result = false;
    }
    if((stats->level < info->min_level) || (stats->level > info->max_level)) {
        result = false;
    }

    return result;
}

static StorageAnimation*
    animation_manager_select_idle_animation(AnimationManager* animation_manager) {
    if(animation_manager->dummy_mode) {
        return animation_storage_find_animation(HARDCODED_ANIMATION_NAME);
    }
    StorageAnimationList_t animation_list;
    StorageAnimationList_init(animation_list);
    animation_storage_fill_animation_list(&animation_list);

    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
    DolphinStats stats = dolphin_stats(dolphin);
    furi_record_close(RECORD_DOLPHIN);
    uint32_t whole_weight = 0;

    StorageAnimationList_it_t it;
    for(StorageAnimationList_it(it, animation_list); !StorageAnimationList_end_p(it);) {
        StorageAnimation* storage_animation = *StorageAnimationList_ref(it);
        const StorageAnimationManifestInfo* manifest_info =
            animation_storage_get_meta(storage_animation);
        bool valid = animation_manager_is_valid_idle_animation(manifest_info, &stats);

        if(valid) {
            whole_weight += manifest_info->weight;
            StorageAnimationList_next(it);
        } else {
            animation_storage_free_storage_animation(&storage_animation);
            /* remove and increase iterator */
            StorageAnimationList_remove(animation_list, it);
        }
    }

    uint32_t lucky_number = furi_hal_random_get() % whole_weight;
    uint32_t weight = 0;

    StorageAnimation* selected = NULL;
    for
        M_EACH(item, animation_list, StorageAnimationList_t) {
            if(lucky_number < weight) {
                break;
            }
            weight += animation_storage_get_meta(*item)->weight;
            selected = *item;
        }

    for
        M_EACH(item, animation_list, StorageAnimationList_t) {
            if(*item != selected) {
                animation_storage_free_storage_animation(item);
            }
        }

    StorageAnimationList_clear(animation_list);

    /* cache animation, if failed - choose reliable animation */
    if(!animation_storage_get_bubble_animation(selected)) {
        const char* name = animation_storage_get_meta(selected)->name;
        FURI_LOG_E(TAG, "Can't upload animation described in manifest: \'%s\'", name);
        animation_storage_free_storage_animation(&selected);
        selected = animation_storage_find_animation(HARDCODED_ANIMATION_NAME);
    }

    furi_assert(selected);
    return selected;
}

bool animation_manager_is_animation_loaded(AnimationManager* animation_manager) {
    furi_assert(animation_manager);
    return animation_manager->current_animation;
}

void animation_manager_unload_and_stall_animation(AnimationManager* animation_manager) {
    furi_assert(animation_manager);
    furi_assert(animation_manager->current_animation);
    furi_assert(!furi_string_size(animation_manager->freezed_animation_name));
    furi_assert(
        (animation_manager->state == AnimationManagerStateIdle) ||
        (animation_manager->state == AnimationManagerStateBlocked));

    if(animation_manager->state == AnimationManagerStateBlocked) {
        animation_manager->state = AnimationManagerStateFreezedBlocked;
    } else if(animation_manager->state == AnimationManagerStateIdle) { //-V547
        animation_manager->state = AnimationManagerStateFreezedIdle;

        animation_manager->freezed_animation_time_left =
            furi_timer_get_expire_time(animation_manager->idle_animation_timer) - furi_get_tick();
        if(animation_manager->freezed_animation_time_left < 0) {
            animation_manager->freezed_animation_time_left = 0;
        }
        furi_timer_stop(animation_manager->idle_animation_timer);
    } else {
        furi_crash();
    }

    FURI_LOG_I(
        TAG,
        "Unload animation \'%s\'",
        animation_storage_get_meta(animation_manager->current_animation)->name);

    StorageAnimationManifestInfo* meta =
        animation_storage_get_meta(animation_manager->current_animation);
    /* copy str, not move, because it can be internal animation */
    furi_string_set(animation_manager->freezed_animation_name, meta->name);

    bubble_animation_freeze(animation_manager->animation_view);
    animation_storage_free_storage_animation(&animation_manager->current_animation);
}

void animation_manager_load_and_continue_animation(AnimationManager* animation_manager) {
    furi_assert(animation_manager);
    furi_assert(!animation_manager->current_animation);
    furi_assert(furi_string_size(animation_manager->freezed_animation_name));
    furi_assert(
        (animation_manager->state == AnimationManagerStateFreezedIdle) ||
        (animation_manager->state == AnimationManagerStateFreezedBlocked));

    if(animation_manager->state == AnimationManagerStateFreezedBlocked) {
        StorageAnimation* restore_animation = animation_storage_find_animation(
            furi_string_get_cstr(animation_manager->freezed_animation_name));
        /* all blocked animations must be in flipper -> we can
         * always find blocking animation */
        furi_assert(restore_animation);
        animation_manager_replace_current_animation(animation_manager, restore_animation);
        animation_manager->state = AnimationManagerStateBlocked;
    } else if(animation_manager->state == AnimationManagerStateFreezedIdle) { //-V547
        /* check if we missed some system notifications, and set current_animation */
        bool blocked = animation_manager_check_blocking(animation_manager);
        if(!blocked) {
            /* if no blocking - try restore last one idle */
            StorageAnimation* restore_animation = animation_storage_find_animation(
                furi_string_get_cstr(animation_manager->freezed_animation_name));
            if(restore_animation) {
                Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
                DolphinStats stats = dolphin_stats(dolphin);
                furi_record_close(RECORD_DOLPHIN);
                const StorageAnimationManifestInfo* manifest_info =
                    animation_storage_get_meta(restore_animation);
                bool valid = animation_manager_is_valid_idle_animation(manifest_info, &stats);
                if(valid) {
                    animation_manager_replace_current_animation(
                        animation_manager, restore_animation);
                    animation_manager->state = AnimationManagerStateIdle;

                    if(animation_manager->freezed_animation_time_left) {
                        furi_timer_start(
                            animation_manager->idle_animation_timer,
                            animation_manager->freezed_animation_time_left);
                    } else {
                        const BubbleAnimation* animation = animation_storage_get_bubble_animation(
                            animation_manager->current_animation);
                        furi_timer_start(
                            animation_manager->idle_animation_timer, animation->duration * 1000);
                    }
                }
            } else {
                FURI_LOG_E(
                    TAG,
                    "Failed to restore \'%s\'",
                    furi_string_get_cstr(animation_manager->freezed_animation_name));
            }
        }
    } else {
        /* Unknown state is an error. But not in release version.*/
        furi_crash();
    }

    /* if can't restore previous animation - select new */
    if(!animation_manager->current_animation) {
        animation_manager_start_new_idle(animation_manager);
    }
    FURI_LOG_I(
        TAG,
        "Load animation \'%s\'",
        animation_storage_get_meta(animation_manager->current_animation)->name);

    bubble_animation_unfreeze(animation_manager->animation_view);
    furi_string_reset(animation_manager->freezed_animation_name);
    furi_assert(animation_manager->current_animation);
}

static void animation_manager_switch_to_one_shot_view(AnimationManager* animation_manager) {
    furi_assert(animation_manager);
    furi_assert(!animation_manager->one_shot_view);
    Dolphin* dolphin = furi_record_open(RECORD_DOLPHIN);
    DolphinStats stats = dolphin_stats(dolphin);
    furi_record_close(RECORD_DOLPHIN);

    animation_manager->one_shot_view = one_shot_view_alloc();
    one_shot_view_set_interact_callback(
        animation_manager->one_shot_view, animation_manager_interact_callback, animation_manager);
    View* prev_view = bubble_animation_get_view(animation_manager->animation_view);
    View* next_view = one_shot_view_get_view(animation_manager->one_shot_view);
    view_stack_remove_view(animation_manager->view_stack, prev_view);
    view_stack_add_view(animation_manager->view_stack, next_view);
    if(stats.level == 1) {
        one_shot_view_start_animation(animation_manager->one_shot_view, &A_Levelup1_128x64);
    } else if(stats.level == 2) {
        one_shot_view_start_animation(animation_manager->one_shot_view, &A_Levelup2_128x64);
    } else {
        furi_crash();
    }
}

static void animation_manager_switch_to_animation_view(AnimationManager* animation_manager) {
    furi_assert(animation_manager);
    furi_assert(animation_manager->one_shot_view);

    View* prev_view = one_shot_view_get_view(animation_manager->one_shot_view);
    View* next_view = bubble_animation_get_view(animation_manager->animation_view);
    view_stack_remove_view(animation_manager->view_stack, prev_view);
    view_stack_add_view(animation_manager->view_stack, next_view);
    one_shot_view_free(animation_manager->one_shot_view);
    animation_manager->one_shot_view = NULL;
}
