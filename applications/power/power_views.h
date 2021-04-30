#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <furi.h>
#include <gui/canvas.h>
#include <gui/view.h>

typedef enum { PowerViewInfo, PowerViewDialog, PowerViewOff, PowerViewDisconnect } PowerView;

typedef struct {
    float current_charger;
    float current_gauge;

    float voltage_charger;
    float voltage_gauge;
    float voltage_vbus;

    uint32_t capacity_remaining;
    uint32_t capacity_full;

    float temperature_charger;
    float temperature_gauge;

    uint8_t charge;
    uint8_t health;
} PowerInfoModel;

typedef struct {
    uint32_t poweroff_tick;
    bool battery_low;
} PowerOffModel;

void power_info_draw_callback(Canvas* canvas, void* context);

void power_off_draw_callback(Canvas* canvas, void* context);

void power_disconnect_draw_callback(Canvas* canvas, void* context);