#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <gui/canvas.h>
#include <flipper_v2.h>
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

static uint32_t power_info_back_callback(void* context) {
    return VIEW_NONE;
}

void power_info_draw_callback(Canvas* canvas, void* context);
