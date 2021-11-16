#pragma once

#include <stdint.h>
#include <furi/pubsub.h>

typedef struct Power Power;

typedef enum {
    PowerBootModeNormal,
    PowerBootModeDfu,
} PowerBootMode;

typedef enum {
    PowerEventTypeStopCharging,
    PowerEventTypeStartCharging,
    PowerEventTypeFullyCharged,
    PowerEventTypeBatteryLevelChanged,
} PowerEventType;

typedef union {
    uint8_t battery_level;
} PowerEventData;

typedef struct {
    PowerEventType type;
    PowerEventData data;
} PowerEvent;

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
} PowerInfo;

/** Power off device
 */
void power_off(Power* power);

/** Reboot device
 * @param mode - PowerBootMode
 */
void power_reboot(PowerBootMode mode);

/** Get power info
 * @param power - Power instance
 * @param info - PowerInfo instance
 */
void power_get_info(Power* power, PowerInfo* info);

/** Get power event pubsub handler
 * @param power - Power instance
 */
FuriPubSub* power_get_pubsub(Power* power);
