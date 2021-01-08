#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <gui/canvas.h>
#include <flipper_v2.h>

// Idle scree
typedef enum {
    DolphinViewFirstStart,
    DolphinViewIdleMain,
    DolphinViewIdleStats,
    DolphinViewIdleDebug,
} DolphinViewIdle;

typedef struct {
    uint32_t page;
} DolphinViewFirstStartModel;

void dolphin_view_first_start_draw(Canvas* canvas, void* model);
bool dolphin_view_first_start_input(InputEvent* event, void* context);

typedef struct {
    uint32_t icounter;
    uint32_t butthurt;
} DolphinViewIdleStatsModel;

void dolphin_view_idle_main_draw(Canvas* canvas, void* model);
bool dolphin_view_idle_main_input(InputEvent* event, void* context);
void dolphin_view_idle_stats_draw(Canvas* canvas, void* model);
bool dolphin_view_idle_stats_input(InputEvent* event, void* context);
void dolphin_view_idle_debug_draw(Canvas* canvas, void* model);
uint32_t dolphin_view_idle_back(void* context);
