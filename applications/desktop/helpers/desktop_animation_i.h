#include <assets_icons.h>
#include <stddef.h>
#include <stdint.h>
#include <gui/icon.h>
#include "desktop_animation.h"

// Calm/Mad Basic Idle Animations

#define COMMON_BASIC_DURATION (60 * 60)
#define COMMON_ACTIVE_CYCLES 7
#define COMMON_ACTIVE_COOLDOWN 30
#define COMMON_WEIGHT 3

#define BUTTHURT_LEVEL(x) (1UL << (x))
#define BUTTHURT_LEVEL_0 0

// frames * cycles / frame_rate
#define COMMON_ACTIVE_DURATION(x) ((x)*COMMON_ACTIVE_CYCLES / 2)

static const BasicAnimation animation_TV = {
    .icon = &A_TV_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7)};

static const ActiveAnimation animation_TV_active = {
    .icon = &A_TVActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_sleep = {
    .icon = &A_Sleep_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7) | BUTTHURT_LEVEL(8) |
                           BUTTHURT_LEVEL(9) | BUTTHURT_LEVEL(10)};

static const ActiveAnimation animation_sleep_active = {
    .icon = &A_SleepActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_leaving = {
    .icon = &A_Leaving_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(13) | BUTTHURT_LEVEL(14),
};

static const ActiveAnimation animation_leaving_active = {
    .icon = &A_LeavingActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_laptop = {
    .icon = &A_Laptop_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5)};

static const ActiveAnimation animation_laptop_active = {
    .icon = &A_LaptopActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_knife = {
    .icon = &A_Knife_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(5) | BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7) |
                           BUTTHURT_LEVEL(8) | BUTTHURT_LEVEL(9) | BUTTHURT_LEVEL(10) |
                           BUTTHURT_LEVEL(11) | BUTTHURT_LEVEL(12) | BUTTHURT_LEVEL(13)};

static const ActiveAnimation animation_knife_active = {
    .icon = &A_KnifeActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_cry = {
    .icon = &A_Cry_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7) | BUTTHURT_LEVEL(8) |
                           BUTTHURT_LEVEL(9) | BUTTHURT_LEVEL(10) | BUTTHURT_LEVEL(11) |
                           BUTTHURT_LEVEL(12) | BUTTHURT_LEVEL(13)};

static const ActiveAnimation animation_cry_active = {
    .icon = &A_CryActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(3),
};

static const BasicAnimation animation_box = {
    .icon = &A_Box_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(7) | BUTTHURT_LEVEL(8) | BUTTHURT_LEVEL(9) |
                           BUTTHURT_LEVEL(10) | BUTTHURT_LEVEL(11) | BUTTHURT_LEVEL(12) |
                           BUTTHURT_LEVEL(13)};

static const ActiveAnimation animation_box_active = {
    .icon = &A_BoxActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_waves = {
    .icon = &A_Waves_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2)};

static const ActiveAnimation animation_waves_active = {
    .icon = &A_WavesActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

// Level Idle Animations

static const BasicAnimation animation_level1furippa = {
    .icon = &A_Level1Furippa_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7)};

static const ActiveAnimation animation_level1furippa_active = {
    .icon = &A_Level1FurippaActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(6),
};

static const BasicAnimation animation_level1read = {
    .icon = &A_Level1Read_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2)};

static const ActiveAnimation animation_level1read_active = {
    .icon = &A_Level1ReadActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_level1toys = {
    .icon = &A_Level1Toys_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7) | BUTTHURT_LEVEL(8)};

static const ActiveAnimation animation_level1toys_active = {
    .icon = &A_Level1ToysActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_level2furippa = {
    .icon = &A_Level2Furippa_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7)};

static const ActiveAnimation animation_level2furippa_active = {
    .icon = &A_Level2FurippaActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(6),
};

static const BasicAnimation animation_level2soldering = {
    .icon = &A_Level2Soldering_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7) | BUTTHURT_LEVEL(8) |
                           BUTTHURT_LEVEL(9)};

static const ActiveAnimation animation_level2soldering_active = {
    .icon = &A_Level2SolderingActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_level2hack = {
    .icon = &A_Level2Hack_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7) | BUTTHURT_LEVEL(8)};

static const ActiveAnimation animation_level2hack_active = {
    .icon = &A_Level2HackActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_level3furippa = {
    .icon = &A_Level3Furippa_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7)};

static const ActiveAnimation animation_level3furippa_active = {
    .icon = &A_Level3FurippaActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(6),
};

static const BasicAnimation animation_level3hijack = {
    .icon = &A_Level3Hijack_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7) | BUTTHURT_LEVEL(8) |
                           BUTTHURT_LEVEL(9)};

static const ActiveAnimation animation_level3hijack_active = {
    .icon = &A_Level3HijackActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

static const BasicAnimation animation_level3lab = {
    .icon = &A_Level3Lab_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = COMMON_WEIGHT,
    .active_cooldown = COMMON_ACTIVE_COOLDOWN,
    .butthurt_level_mask = BUTTHURT_LEVEL(0) | BUTTHURT_LEVEL(1) | BUTTHURT_LEVEL(2) |
                           BUTTHURT_LEVEL(3) | BUTTHURT_LEVEL(4) | BUTTHURT_LEVEL(5) |
                           BUTTHURT_LEVEL(6) | BUTTHURT_LEVEL(7) | BUTTHURT_LEVEL(8)};

static const ActiveAnimation animation_level3lab_active = {
    .icon = &A_Level3LabActive_128x51,
    .duration = COMMON_ACTIVE_DURATION(2),
};

// System Idle Animations

static const BasicAnimation animation_bad_battery = {
    .icon = &A_BadBattery_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = 7,
};

static const BasicAnimation animation_no_sd_card = {
    .icon = &A_NoSdCard_128x51,
    .duration = COMMON_BASIC_DURATION,
    .weight = 7,
};

const Icon* animation_level2up[] = {
    &I_LevelUp2_01,
    &I_LevelUp2_02,
    &I_LevelUp2_03,
    &I_LevelUp2_04,
    &I_LevelUp2_05,
    &I_LevelUp2_06,
    &I_LevelUp2_07};

const Icon* animation_level3up[] = {
    &I_LevelUp3_01,
    &I_LevelUp3_02,
    &I_LevelUp3_03,
    &I_LevelUp3_04,
    &I_LevelUp3_05,
    &I_LevelUp3_06,
    &I_LevelUp3_07};

// Blocking Idle Animations & One shot Animations represented as naked Icon

static const PairedAnimation calm_animation[] = {
    {.basic = &animation_TV, .active = &animation_TV_active},
    {.basic = &animation_waves, .active = &animation_waves_active},
    {.basic = &animation_sleep, .active = &animation_sleep_active},
    {.basic = &animation_laptop, .active = &animation_laptop_active},
};

static const PairedAnimation mad_animation[] = {
    {.basic = &animation_cry, .active = &animation_cry_active},
    {.basic = &animation_knife, .active = &animation_knife_active},
    {.basic = &animation_box, .active = &animation_box_active},
    {.basic = &animation_leaving, .active = &animation_leaving_active},
};

static const PairedAnimation level_1_animation[] = {
    {.basic = &animation_level1furippa, .active = &animation_level1furippa_active},
    {.basic = &animation_level1read, .active = &animation_level1read_active},
    {.basic = &animation_level1toys, .active = &animation_level1toys_active},
};

static const PairedAnimation level_2_animation[] = {
    {.basic = &animation_level2furippa, .active = &animation_level2furippa_active},
    {.basic = &animation_level2soldering, .active = &animation_level2soldering_active},
    {.basic = &animation_level2hack, .active = &animation_level2hack_active},
};

static const PairedAnimation level_3_animation[] = {
    {.basic = &animation_level3furippa, .active = &animation_level3furippa_active},
    {.basic = &animation_level3hijack, .active = &animation_level3hijack_active},
    {.basic = &animation_level3lab, .active = &animation_level3lab_active},
};

static const PairedAnimation no_sd_animation[] = {
    {.basic = &animation_no_sd_card, .active = NULL},
};

static const PairedAnimation check_battery_animation[] = {
    {.basic = &animation_bad_battery, .active = NULL},
};
