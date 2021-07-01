#pragma once

typedef struct Power Power;

typedef enum {
    PowerBootModeNormal,
    PowerBootModeDfu,
} PowerBootMode;

/** Power off device
 * @param power - Power instance
 */
void power_off(Power* power);

/** Reboot device
 * @param power - Power instance
 * @param mode - PowerBootMode
 */
void power_reboot(Power* power, PowerBootMode mode);
