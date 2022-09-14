#pragma once

#include <gui/view.h>

typedef struct BatteryInfo BatteryInfo;

typedef struct {
    float vbus_voltage;
    float gauge_voltage;
    float gauge_current;
    float gauge_temperature;
    uint8_t charge;
    uint8_t health;
} BatteryInfoModel;

BatteryInfo* battery_info_alloc();

void battery_info_free(BatteryInfo* battery_info);

View* battery_info_get_view(BatteryInfo* battery_info);

void battery_info_set_data(BatteryInfo* battery_info, BatteryInfoModel* data);
