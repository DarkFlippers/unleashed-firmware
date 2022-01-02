#pragma once
#include "animation_storage.h"
#include "assets_icons.h"
#include "animation_manager.h"
#include "gui/canvas.h"

struct StorageAnimation {
    const BubbleAnimation* animation;
    bool external;
    StorageAnimationMeta meta;
};

// Hard-coded, always available idle animation
FrameBubble tv_bubble1 = {
    .bubble =
        {.x = 1,
         .y = 23,
         .str = "Take the red pill",
         .horizontal = AlignRight,
         .vertical = AlignBottom},
    .starts_at_frame = 7,
    .ends_at_frame = 9,
    .next_bubble = NULL,
};
FrameBubble tv_bubble2 = {
    .bubble =
        {.x = 1,
         .y = 23,
         .str = "I can joke better",
         .horizontal = AlignRight,
         .vertical = AlignBottom},
    .starts_at_frame = 7,
    .ends_at_frame = 9,
    .next_bubble = NULL,
};
FrameBubble* tv_bubbles[] = {&tv_bubble1, &tv_bubble2};
const Icon* tv_icons[] = {
    &I_tv1,
    &I_tv2,
    &I_tv3,
    &I_tv4,
    &I_tv5,
    &I_tv6,
    &I_tv7,
    &I_tv8,
};
const BubbleAnimation tv_bubble_animation = {
    .icons = tv_icons,
    .frame_bubbles = tv_bubbles,
    .frame_bubbles_count = COUNT_OF(tv_bubbles),
    .passive_frames = 6,
    .active_frames = 2,
    .active_cycles = 2,
    .frame_rate = 2,
    .duration = 3600,
    .active_cooldown = 5,
};

// System animation - no SD card
const Icon* no_sd_icons[] = {
    &I_no_sd1,
    &I_no_sd2,
    &I_no_sd1,
    &I_no_sd2,
    &I_no_sd1,
    &I_no_sd3,
    &I_no_sd4,
    &I_no_sd5,
    &I_no_sd4,
    &I_no_sd6,
};
FrameBubble no_sd_bubble = {
    .bubble =
        {.x = 40,
         .y = 18,
         .str = "Need an\nSD card",
         .horizontal = AlignRight,
         .vertical = AlignBottom},
    .starts_at_frame = 0,
    .ends_at_frame = 9,
    .next_bubble = NULL,
};
FrameBubble* no_sd_bubbles[] = {&no_sd_bubble};
const BubbleAnimation no_sd_bubble_animation = {
    .icons = no_sd_icons,
    .frame_bubbles = no_sd_bubbles,
    .frame_bubbles_count = COUNT_OF(no_sd_bubbles),
    .passive_frames = 10,
    .active_frames = 0,
    .frame_rate = 2,
    .duration = 3600,
    .active_cooldown = 0,
    .active_cycles = 0,
};

// BLOCKING ANIMATION - no_db, bad_sd, sd_ok, url
const Icon* no_db_icons[] = {
    &I_no_databases1,
    &I_no_databases2,
    &I_no_databases3,
    &I_no_databases4,
};
const BubbleAnimation no_db_bubble_animation = {
    .icons = no_db_icons,
    .passive_frames = COUNT_OF(no_db_icons),
    .frame_rate = 2,
};

const Icon* bad_sd_icons[] = {
    &I_card_bad1,
    &I_card_bad2,
};
const BubbleAnimation bad_sd_bubble_animation = {
    .icons = bad_sd_icons,
    .passive_frames = COUNT_OF(bad_sd_icons),
    .frame_rate = 2,
};

const Icon* url_icons[] = {
    &I_url1,
    &I_url2,
    &I_url3,
    &I_url4,
};
const BubbleAnimation url_bubble_animation = {
    .icons = url_icons,
    .passive_frames = COUNT_OF(url_icons),
    .frame_rate = 2,
};

const Icon* sd_ok_icons[] = {
    &I_card_ok1,
    &I_card_ok2,
    &I_card_ok3,
    &I_card_ok4,
};
const BubbleAnimation sd_ok_bubble_animation = {
    .icons = sd_ok_icons,
    .passive_frames = COUNT_OF(sd_ok_icons),
    .frame_rate = 2,
};

static StorageAnimation StorageAnimationInternal[] = {
    {.animation = &tv_bubble_animation,
     .external = false,
     .meta =
         {
             .min_butthurt = 0,
             .max_butthurt = 11,
             .min_level = 1,
             .max_level = 3,
             .weight = 3,
         }},
    {.animation = &no_sd_bubble_animation,
     .external = false,
     .meta =
         {
             .min_butthurt = 0,
             .max_butthurt = 14,
             .min_level = 1,
             .max_level = 3,
             .weight = 6,
         }},
    {
        .animation = &no_db_bubble_animation,
        .external = false,
    },
    {
        .animation = &bad_sd_bubble_animation,
        .external = false,
    },
    {
        .animation = &sd_ok_bubble_animation,
        .external = false,
    },
    {
        .animation = &url_bubble_animation,
        .external = false,
    },
};

void animation_storage_initialize_internal_animations(void) {
    /* not in constructor - no memory pool yet */
    /* called in 1 thread - no need in double check */
    static bool initialized = false;
    if(!initialized) {
        initialized = true;
        string_init_set_str(StorageAnimationInternal[0].meta.name, HARDCODED_ANIMATION_NAME);
        string_init_set_str(StorageAnimationInternal[1].meta.name, NO_SD_ANIMATION_NAME);
        string_init_set_str(StorageAnimationInternal[2].meta.name, NO_DB_ANIMATION_NAME);
        string_init_set_str(StorageAnimationInternal[3].meta.name, BAD_SD_ANIMATION_NAME);
        string_init_set_str(StorageAnimationInternal[4].meta.name, SD_OK_ANIMATION_NAME);
        string_init_set_str(StorageAnimationInternal[5].meta.name, URL_ANIMATION_NAME);
    }
}
