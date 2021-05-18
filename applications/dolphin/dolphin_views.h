#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <gui/canvas.h>
#include <input/input.h>
#include <furi.h>

// Idle screen
typedef enum {
    DolphinViewIdleMain,
    DolphinViewFirstStart,
    DolphinViewStats,
    DolphinViewHwMismatch,
    DolphinViewLockMenu,
} DolphinViewIdle;

// Debug info
typedef enum {
    DolphinViewStatsFw,
    DolphinViewStatsBoot,
    DolphinViewStatsMeta,
    DolphinViewStatsTotalCount,
} DolphinViewStatsScreens;

typedef struct {
    uint32_t page;
} DolphinViewFirstStartModel;

void dolphin_view_first_start_draw(Canvas* canvas, void* model);
bool dolphin_view_first_start_input(InputEvent* event, void* context);

typedef struct {
    uint32_t icounter;
    uint32_t butthurt;
    DolphinViewStatsScreens screen;
} DolphinViewStatsModel;

typedef struct {
    uint8_t idx;
    int8_t door_left_x;
    int8_t door_right_x;
    uint8_t exit_timeout;
    bool locked;
} DolphinViewLockMenuModel;

typedef struct {
    Icon* animation;
    uint8_t scene_num;
    uint8_t hint_timeout;
    bool locked;
} DolphinViewMainModel;

void dolphin_view_idle_main_draw(Canvas* canvas, void* model);
bool dolphin_view_idle_main_input(InputEvent* event, void* context);

void dolphin_view_idle_up_draw(Canvas* canvas, void* model);

void dolphin_view_lockmenu_draw(Canvas* canvas, void* model);

void dolphin_view_idle_down_draw(Canvas* canvas, void* model);

void dolphin_view_hw_mismatch_draw(Canvas* canvas, void* model);

uint32_t dolphin_view_idle_back(void* context);
