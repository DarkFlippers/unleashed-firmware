#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <furi.h>
#include <gui/canvas.h>
#include <gui/view.h>

typedef enum { PowerViewInfo } PowerView;

typedef struct {
    float current_charger;
    float current_gauge;

    float voltage_charger;
    float voltage_gauge;

    uint32_t capacity_remaining;
    uint32_t capacity_full;

    float temperature_charger;
    float temperature_gauge;

    uint8_t charge;
} PowerInfoModel;

void power_info_draw_callback(Canvas* canvas, void* context);
