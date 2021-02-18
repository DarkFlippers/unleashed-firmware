#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <gui/canvas.h>
#include <input/input.h>
#include <furi.h>

// Idle scree
typedef enum {
    DolphinViewFirstStart,
    DolphinViewIdleMain,
    DolphinViewIdleUp,
    DolphinViewIdleDown,
    DolphinViewHwMismatch,
} DolphinViewIdle;

typedef struct {
    uint32_t page;
} DolphinViewFirstStartModel;

void dolphin_view_first_start_draw(Canvas* canvas, void* model);
bool dolphin_view_first_start_input(InputEvent* event, void* context);

typedef struct {
    uint32_t icounter;
    uint32_t butthurt;
} DolphinViewIdleUpModel;

void dolphin_view_idle_main_draw(Canvas* canvas, void* model);
bool dolphin_view_idle_main_input(InputEvent* event, void* context);

void dolphin_view_idle_up_draw(Canvas* canvas, void* model);
void dolphin_view_idle_down_draw(Canvas* canvas, void* model);

void dolphin_view_hw_mismatch_draw(Canvas* canvas, void* model);

uint32_t dolphin_view_idle_back(void* context);
