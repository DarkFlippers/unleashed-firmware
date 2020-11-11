#pragma once

#include <stdbool.h>

/* Initialize Driver */
void bq25896_init();

/* Send device into shipping mode */
void bq25896_poweroff();

/* Is currently charging */
bool bq25896_is_charging();

/* Enable otg */
void bq25896_enable_otg();

/* Disable otg */
void bq25896_disable_otg();
