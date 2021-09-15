#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Bt Bt;

void bt_update_statusbar(Bt* bt);

void bt_update_battery_level(Bt* bt, uint8_t battery_level);

bool bt_pin_code_show(Bt* bt, uint32_t pin_code);

#ifdef __cplusplus
}
#endif
