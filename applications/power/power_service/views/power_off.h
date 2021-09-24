#pragma once

typedef struct PowerOff PowerOff;

#include <gui/view.h>

PowerOff* power_off_alloc();

void power_off_free(PowerOff* power_off);

View* power_off_get_view(PowerOff* power_off);

void power_off_set_time_left(PowerOff* power_off, uint8_t time_left);
