#pragma once

#include <stdint.h>

/* Initialize Driver */
void bq27220_init();

/* Get battery voltage in mV */
uint16_t bq27220_get_voltage();

/* Get current in mA */
int16_t bq27220_get_current();

/* Get compensated full charge capacity in in mAh */
uint16_t bq27220_get_full_charge_capacity();

/* Get remaining capacity in in mAh */
uint16_t bq27220_get_remaining_capacity();

/* Get predicted remaining battery capacity in percents */
uint16_t bq27220_get_state_of_charge();
